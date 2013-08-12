# Rock-Paper-Scissors!

all : rps.c
	cc rps.c -o RPS -lpanel -lncurses -O3 -march=native
