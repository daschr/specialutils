all:	pipedelay

pipedelay: pipedelay.c
	cc -o pipedelay -lpthread pipedelay.c

clean:
	rm pipedelay
