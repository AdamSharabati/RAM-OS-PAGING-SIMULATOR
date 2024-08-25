#!/bin/bash

echo "aaaaaaaabbbbbbbbccccccccddddddddeeeeeeeeffffffff" > exec_file
gcc -Wall runEX.c mem_sim.c -o run
./run
