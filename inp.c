#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


struct ss{ 
	char *s;
	unsigned long size;
} s;

int readline(void){
	int c,retc=1;
	unsigned long lc=0;
	while((c=getc(stdin)) != '\n'){
		if(lc == s.size -2){
			s.size=s.size*2;
			s.s=realloc(s.s,s.size);
		}
		if(c==-1){
			retc=0;
			break;
		}	
		s.s[lc++]=c;
	}
	s.s[lc]='\0';
	return retc;
}


int main(int argc, char *args[]){
	if(argc != 2){
		fprintf(stderr, "Need exactly one argument!\n");
		return(EXIT_FAILURE);
	}
	unsigned long wait_time;
	if(sscanf(args[1],"%lu",&wait_time) != 1){
		fprintf(stderr, "Invalid time!\n");
		return(EXIT_FAILURE);
	}
	setvbuf(stdout, NULL, _IONBF, 0);
#ifdef DEBUG
	printf("wait_time: %lu\n",wait_time);
#endif

	s.s=malloc(256);
	s.size=256;

	unsigned long ctime=time(NULL);
	unsigned long ntime;
	if(!readline()){
		free(s.s);
		return 0;
	}
	printf("%s\n",s.s);
	
	while(readline()){
		ntime=time(NULL);
#ifdef DEBUG
		printf("%lu\n",ntime-ctime);
#endif
		if(ntime -ctime >= wait_time){
#ifdef DEBUG
			printf("size: %lu\n",s.size);
#endif
			printf("%s\n",s.s);
			ctime=ntime;
		}
	}
	free(s.s);
	return 0;
}
