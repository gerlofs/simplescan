#include "portscan.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORTS_MAX 20

struct addrinfo host, *phost;

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
    printf("%s\n", ip_addr);

    free(ports); // Important to free up allocated memory.

    return 0;
}
