#include "portscan.h"
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

/*
 *  -a (hostname/address): Can be provided as a hostname (e.g. www.google.com) or as an IP address 
 *      (e.g. 172.217.20.132).
 *  -p (port(s)): Provided as a list of numbers, comma seperated. 
 */

int getArgs(int* port_array, int argc, char *argv[]) {
    extern char *optarg;
    extern int optind;
    int opt, arglen, arg_index, addr_index;
    unsigned int i = 0;
    bool aflag, pflag = false;
    char *port_arg;
    int d_port_arg = 0;
    
    printf("Parsing arguments... \n");

    while ((opt = getopt(argc, argv, "a:p")) != -1)
        switch(opt) { 
            case 'a': // Address string is present.
                addr_index = optind-1; // Save the 'argv' index (easier than returning a str).
                aflag = true;
                break;
            case 'p': // A list of ports is present.
                arg_index = optind; 
                while ( arg_index < argc ) { // Iterate through arguments starting from '-p'.
                   port_arg = argv[arg_index]; // Get argument.
                   if (port_arg[0] != '-') { 
                       arglen = strlen(port_arg); // If not the 'flag', set length of arg.
                       for ( int c = 0; c < arglen; c++ ) { // Iterate through chars.
                           // For each char, convert to int from ASCII and multiply to put it in its correct decimal place.
                           d_port_arg += (((int)port_arg[c])-48)*(pow(10, ((arglen-1)-c)));
                       }
                       port_array[i] = d_port_arg; // Set port in port array.
                       printf("Port set: %d\n", port_array[i]); 
                       d_port_arg = 0; // Reset port numeric.
                       arg_index++; i++; 
                   } else break;
                }

                pflag = true;
                optind = arg_index - 1; // Important to reset optind so that parsing may continue.
                break;
        }

    // Flag checking:
    if ( aflag ) {
        if (!pflag) {
            int default_ports[20] = {21, 22, 23, 25, 53, 80, 110, 111, 135, 139, 143, 443, 445, 993, 995, 1723, 3306, 3389, 5900, 8080};
            for ( i = 0; i < (sizeof(default_ports) / sizeof(default_ports[0])); i++ ) {
                port_array[i] = default_ports[i];
            }
            printf("Default ports set.\n");
        }

        return addr_index;
    } else return -1;
}
