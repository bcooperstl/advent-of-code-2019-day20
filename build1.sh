#!/bin/bash
gcc -g -c node.c
gcc -g -c part1.c
#gcc -g -c part2.c
gcc -o part1 part1.o node.o
#gcc -o part2 part2.o node.o

