/*
	pipedelay, like the name says, reads from stdin and writes to stdout line by line delayed by the amount of seconds given as an argument 

	f.e.:
		/usr/bin/inotifywait -m -e close_write --format=%w%f | pipedelay 5 | while read line; do 
			[ -e "$line" ] && echo "Yay, file still exists after 5 seconds!" 
		done 
	
	valgrind tested
*/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define STANDARD_SLEEP 500000
#define STANDARD_LINE 256

#define ts_head(X) X->first == NULL ? 0 : X->first->timestamp

typedef struct _entry{
	struct _entry *next;
	unsigned long timestamp;
	char *value;
} entry;

typedef struct {
	entry *first;
	entry *last;
} queue;

typedef struct {
	unsigned long wt;
	queue *q;
	int no_lines;
} thread_args;


void enqueue(queue *q, char *s){
	unsigned long ts=time(NULL);
	
	entry *new_entry=malloc(sizeof(entry));
	new_entry->timestamp=ts;
	new_entry->value=s;
	new_entry->next=NULL;
	
	if(q->first == NULL){
		q->first=new_entry;
		q->last=new_entry;
	}else{
		(q->last)->next=new_entry;
		q->last=new_entry;
	}
}

char *pop(queue *q){
	entry *h=q->first;
	if(h == q->last)
		q->last=NULL;
	char *v= h->value;
	q->first=h->next;
	free(h);
	return v;
}

char *head(queue *q){
	if(q->first == NULL)
		return NULL;		
	return (q->first)->value;
}


char *readline(void){
	char *line=malloc(STANDARD_LINE);
	size_t ls=STANDARD_LINE, ps=0;
	int c;
	for(c= getc(stdin);c!= -1 && c!='\n'; c=fgetc(stdin)){
		if(ps == ls){
			ls+=STANDARD_LINE;	
			line=realloc(line,ls);
		}
		line[ps++]=c;
	}
	if(ps==0 && c == -1){
		free(line);
		return NULL;
	}
	if(ps == ls)
		line=realloc(line,ls+1);
	line[ps]='\0';
	return line;
}

void *print_queue(void *args_p){
	thread_args *args= (thread_args *) args_p;
 	for(unsigned long ts=ts_head(args->q); ts != 0 || ! args->no_lines;ts=ts_head(args->q)){
			if(ts != 0 && time(NULL) - ts >= args->wt){
				char *lp=pop(args->q);
				printf("%s\n",lp);
				free(lp);
			}
			
			usleep(STANDARD_SLEEP);
	}
	return NULL;
}


int main(int argc, char *args[]){
	if(argc != 2){
		fprintf(stderr,"Usage: %s [waittime]\n",args[0]);
		return EXIT_FAILURE;
	}
	setvbuf(stdout, NULL, _IONBF, 0);	
	unsigned long wt;
	if(sscanf(args[1],"%lu",&wt) != 1){
		fprintf(stderr,"Argument must be a number!\n");
		exit(EXIT_FAILURE);
	}

	queue *q=malloc(sizeof(queue));
	q->first=NULL;
	q->last=NULL;
	
	thread_args *targs=malloc(sizeof(thread_args));
	targs->no_lines=0;
	targs->wt=wt;
	targs->q=q;
	
	pthread_t print_queue_thread;
	
	if(pthread_create(&print_queue_thread,NULL,print_queue,targs)){
		fprintf(stderr, "Could not create pthread!\n");
		free(q);
		free(targs);
		exit(EXIT_FAILURE);
	}


	char *l;
	while((l=readline()) != NULL)
		enqueue(q,l);
	targs->no_lines=1;


	if(pthread_join(print_queue_thread,NULL)){
		fprintf(stderr, "Could not join pthread!\n");
		free(q);
		free(targs);
		exit(EXIT_FAILURE);
	}
		
	free(q);
	free(targs);
	return EXIT_SUCCESS;
}
