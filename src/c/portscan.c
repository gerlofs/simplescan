#include <fcntl.h>
#include <errno.h>
#include "portscan.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#define PORTS_MAX 20

// TODO: Add timeout value as cl argument.

typedef struct { 
    char *ipaddr;
    int port;
    unsigned int timeout_s;
} socket_info;

struct addrinfo host, *phost;
pthread_mutex_t access_mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned int scanPort(void* args) { 
    /*
     *  Attempt to connect to a port @ hostname address on a timeout. 
     *  Returns bool dependent on whether the connection was successful or not. 
     */
    
    // Unpack input struct.
    socket_info *input_args = args;
    unsigned int timeout_seconds = input_args->timeout_s;
    char *hostname = input_args->ipaddr;
    int port_number = input_args->port;

    fd_set settr;
    struct timeval timeout;
    struct sockaddr_in client;
    socklen_t client_addr_len;
    int socket_response, socket_error, sockrlen;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero((char *) &client, sizeof(client)); // Zero buffer.

    if ( sock != -1 ) { // Socket is initalised without errors.
        // Setup sockaddr struct.
        client.sin_addr.s_addr = inet_addr(hostname);
        client.sin_family = AF_INET;
        client.sin_port = htons(port_number); // Convert to network byte order.
        client_addr_len = sizeof(client); 
        // Set socket to 'non-blocking' using system call.
        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

        FD_ZERO(&settr); 
        FD_SET(sock, &settr); // Clear then add socket descriptor. 
        timeout.tv_sec = timeout_seconds; // Set timeout.
        timeout.tv_usec = 0; // Note: Not sure about needing to set this.

        // Attempt to connect, store response.
        socket_response = connect(sock, (struct sockaddr *)&client, sizeof(client));
        if ( errno != EINPROGRESS ) { // Errno should be set to 'EINPROGRESS' during connection.
            perror("Error whilst opening connection with socket: ");
            return 0;
        }
        
        // Use select to block waiting on socket activity. 
        socket_response = select(sock+1, NULL, &settr, NULL, &timeout);
        
        switch(socket_response) { // Response value denotes connect/failure/timeout.
            case 0: 
                printf("Connection failed to port %d: Timed out waiting for port.\n", port_number);
                return 0;
            case -1:
                perror("Error whilst waiting on socket response: ");
                return 0;
            default:
                // If we've got a positive response, check for any socket errors.
                sockrlen = sizeof(socket_error); 
                // Get value of SO_ERROR from SOL_SOCKET level (also clears the error).
                getsockopt(sock, SOL_SOCKET, SO_ERROR, &socket_error, &sockrlen);
                if ( socket_error == 0 ) {
                    printf("Connected to port %d\n", port_number);
                } else {
                    printf("Failed to connect to port %d: Error code (%d)\n", port_number, socket_error);
                    return 0; // May have to close the socket here?
                }
        }

        // Set back to 'blocking'. 
        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
        close(sock);
        free(args);
        return 1;   

    } else {
        perror("Error whilst initalising socket: \n");
        return 0;
   }
 
}

void getAddr(char *ip_addr, char *hostname){
    /*
     *  Convert input hostname into an ip address using "getnameinfo" rather than calling
     *  directly to the now deprecated "gethostbyname".
     */
    
    host.ai_family = AF_INET; // Set address family to IP.
    int response = getaddrinfo(hostname, NULL, &host, &phost); //  
    
    if ( response ) { // If response is not NULL.
        printf("Error finding address for hostname: %s\nResponse code (%d)\n", hostname, response);
        exit(0);
    }

    struct addrinfo *pres; // Pointer to response from getaddrinfo.
    char hostaddr[16]; // Address length can be no more than 15 bytes.
    unsigned int addrlen;
    int comp;

    // Store host pointer in response pointer, iterate until at the end of the response struct.
    for ( pres = phost; pres != NULL; pres = pres->ai_next ) {
        // Store IP address in hostaddr.
        getnameinfo(pres->ai_addr, pres->ai_addrlen, hostaddr, sizeof(hostaddr), NULL, 0, NI_NUMERICHOST);
        addrlen = strlen(hostaddr);
        comp = strncmp(ip_addr, hostaddr, addrlen); 
        // If input ip address and hostaddr do not match, copy hostaddr->ip_addr.
        if ( comp != 0 ) {
            strncpy(ip_addr, hostaddr, addrlen);
        } else break; 
    }
    // Add null byte, free up memory and return (with ip addr updated).
    ip_addr[addrlen] = '\0';
    freeaddrinfo(phost);
    return 0;
}

int main(int argc, char *argv[]) {
    int* ports = ( int * ) malloc(sizeof(int) * PORTS_MAX); // Initalise array of port numbers.
    int address_index = getArgs( ports, argc, argv); // Get argv index of address string.
    char* hostname;
    unsigned int p = 0;

    if ( address_index > 0 ) {
        hostname = argv[address_index]; // Get address string.
    } else {
        printf("No address provided, use -a <address> on the command-line.\n");
        exit(0);
    }
 
    char ip_addr[15];
    getAddr(ip_addr, hostname);
    pthread_t threads[PORTS_MAX];
    unsigned int responses[PORTS_MAX];
    unsigned int n_threads = 0;
    pthread_mutex_init(&access_mutex, NULL);

    for ( ; p < PORTS_MAX; p++ ) {
        if ( ports[p] != 0 ) {
            printf("Starting scan on port %d (%d)\n", ports[p], p);
            socket_info *info_struct = malloc(sizeof *info_struct);       
            info_struct->port = ports[p];
            info_struct->ipaddr = ip_addr;
            info_struct->timeout_s = 2;
            responses[p] = pthread_create(&threads[p], NULL, scanPort, info_struct);
            n_threads++;
        }
    }

    for ( int t = 0; t <= n_threads; t++ ) {
        if ( responses[t] == 0 ) {
            pthread_join(threads[t], NULL);
        }
    }

    free(ports); // Important to free up allocated memory.
    
    return 0;
}
