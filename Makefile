FLAGS=-pedantic -Wall

all:	pipedelay inp pipeduprem

pipedelay: pipedelay.c
	cc $(FLAGS) -o pipedelay -lpthread pipedelay.c
pipeduprem: pipeduprem.c
	cc $(FLAGS) -o pipeduprem -lpthread pipeduprem.c
inp: inp.c
	cc $(FLAGS) -o inp inp.c

clean:
	rm pipedelay inp pipeduprem
