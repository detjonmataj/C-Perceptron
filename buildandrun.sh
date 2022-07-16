#!/bin/sh

set -xe

# gcc -Wall -Wextra -ggdb -lm -g3 -Wpedantic -pedantic -Wformat=2 main.c -o main
gcc -O3 main.c -o main

./main
