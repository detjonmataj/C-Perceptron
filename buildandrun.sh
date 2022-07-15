#!/bin/sh

set -xe

gcc -Wall -Wextra -ggdb -lm -g3 -Wpedantic -pedantic -Wformat=2 -o main main.c && ./main 
