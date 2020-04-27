#!/bin/bash
gcc -c portscan.c -o portscan.o -lm
gcc -c parseargs.c -o parseargs.o -lm
gcc portscan.o parseargs.o -o portscan -lm
echo "Compiled!"
