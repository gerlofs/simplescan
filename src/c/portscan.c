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
#define PORTS_MAX 20

// TODO: Clean code, reformat where appropriate.
// TODO: Multi-thread.
// TODO: Test.

struct addrinfo host, *phost;

void scanPort(char *hostname, int port_number) { 
    long fc_resp;
    fd_set settr;
    struct timeval timeout;
    struct sockaddr_in client;
    socklen_t client_addr_len;
    int socket_response, socket_error, sockrlen;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    bzero((char *) &client, sizeof(client));

    if ( sock != -1 ) {
        
        client.sin_addr.s_addr = inet_addr(hostname);
        client.sin_family = AF_INET;
        client.sin_port = htons(port_number);
        client_addr_len = sizeof(client);
        
        printf("%d (%d), %s\n", client.sin_port,port_number, hostname); 

        printf("SOCK: %d\n", fcntl(sock, F_GETFL, 0));

        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
        printf("Set to non-block\n");
        printf("SOCK: %d\n", fcntl(sock, F_GETFL, 0));

        FD_ZERO(&settr);
        FD_SET(sock, &settr);
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        socket_response = connect(sock, (struct sockaddr *)&client, sizeof(client));
        if ( errno != EINPROGRESS ) { 
            printf("%d\n", errno);
            exit(0);
        }
        
        socket_response = select(sock+1, NULL, &settr, NULL, &timeout);
        switch(socket_response) {
            case 0:
                printf("Cannot connect (timeout)\n");
                return 0;
            case -1:
                printf("Error during wait period %d\n", errno);
            default:
                sockrlen = sizeof(socket_error);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, &socket_error, &sockrlen);
                if ( socket_error == 0 ) {
                    printf("Connected\n");
                    return 1;
                } else {
                    printf("Failed to connect %d\n", socket_error);
                    return 0;
                }
        }

        if ( socket_response == 1 ) {
            sockrlen = sizeof(socket_response);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &socket_response, &sockrlen);
            if ( socket_response == 0 ) {
                printf("Connected!\n");
                //return 1;
            } else {
                printf("Cannot connect!\n");
                return 0;
            }
        }

        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
        printf("Set to block\n");
   
        printf("SOCK: %d\n", fcntl(sock, F_GETFL, 0));

    } else {
        printf("Error initalising socket!\n");
        exit(1);
   }
 
}

void getAddr(char *ip_addr, char *hostname, int *ports[]){
    host.ai_family = AF_INET;
    int response = getaddrinfo(hostname, NULL, &host, &phost);
    
    if ( response ) {
        printf("Error finding address for hostname: %s\nResponse code (%d)\n", hostname, response);
        exit(1);
    }

    struct addrinfo *pres;
    char hostaddr[256];
    unsigned int addrlen;
    int comp;

    for ( pres = phost; pres != NULL; pres = pres->ai_next ) {
        getnameinfo(pres->ai_addr, pres->ai_addrlen, hostaddr, sizeof(hostaddr), NULL, 0, NI_NUMERICHOST);
        addrlen = strlen(hostaddr);
        comp = strncmp(ip_addr, hostaddr, addrlen);
        if ( comp != 0 ) {
            strncpy(ip_addr, hostaddr, addrlen);
        } else break;
    }
    
    ip_addr[addrlen] = '\0';
    freeaddrinfo(phost);
    return 0;
}

int main(int argc, char *argv[]){
    int* ports = ( int * ) malloc(sizeof(int) * PORTS_MAX); // Initalise array of port numbers.
    int address_index = getArgs( ports, argc, argv); // Get argv index of address string.
    char* hostname;
    if ( address_index > 0 ) {
        hostname = argv[address_index]; // Get address string.
    } else {
        printf("No address provided, use -a <address> on the command-line.\n");
        exit(0);
    }
    
    char ip_addr[15];
    getAddr(ip_addr, hostname, ports);
    printf("Args: %s %d\n", ip_addr, ports[0]);
    scanPort(ip_addr, ports[0]); 
    free(ports); // Important to free up allocated memory.
    
    return 0;
}
