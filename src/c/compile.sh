#!/bin/bash
gcc -pthread -c portscan.c -o portscan.o -lm
gcc -c parseargs.c -o parseargs.o -lm
gcc -pthread portscan.o parseargs.o -o portscan -lm
echo "Compiled!"
