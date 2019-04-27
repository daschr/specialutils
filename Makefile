FLAGS=-pedantic -Wall

all:	pipedelay inp

pipedelay: pipedelay.c
	cc $(FLAGS) -o pipedelay -lpthread pipedelay.c
inp: inp.c
	cc $(FLAGS) -o inp inp.c

clean:
	rm pipedelay inp
