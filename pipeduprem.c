/*
	pipeduprem, like the name says, reads from stdin and checks if the input is a duplicate 
			in sense of all lines written in the last seconds given as an argument

	f.e.:
		/usr/bin/inotifywait -m -e close_write --format=%w%f | pipeduprem 10 | while read line; do 
			echo "$line is not a duplicate!" 
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
#define head(X) X->first == NULL ? NULL : X->first->value
#define ts_head(X) X->first == NULL ? 0 : X->first->timestamp
 
typedef struct _entry{
	unsigned long timestamp;
	char *value;
	struct _entry *next;
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

void pop(queue *q){
	entry *h=q->first;
	if(h == q->last)
		q->last=NULL;
	q->first=h->next;
	free(h->value);
	free(h);
}

int8_t is_enqueued(queue *q, char *s){
	for(entry *re=q->first; re!=NULL; re=re->next)
		if(strcmp(s,re->value)==0)
			return 1;
	return 0;
}

char *readline(void){
	char *line=malloc(STANDARD_LINE);
	unsigned long ls=STANDARD_LINE, ps=0;
	int c;
	for(c= getc(stdin);c!= -1 && c!='\n'; c=getc(stdin)){
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

void *queue_cleaner(void *args_p){
	thread_args *args= (thread_args *) args_p;

	for(unsigned long ts=ts_head(args->q); ! args->no_lines; ts=ts_head(args->q)){
			if(ts != 0 && time(NULL) - ts >= args->wt)
				pop(args->q);
			usleep(STANDARD_SLEEP);
	}
	
	while(args->q->first != NULL)
		pop(args->q);
	
	return NULL;
}


int main(int argc, char *args[]){
	if(argc != 2){
		fprintf(stderr,"Usage: %s [deadline]\n",args[0]);
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
	
	pthread_t queue_cleaner_thread;
	
	if(pthread_create(&queue_cleaner_thread,NULL,queue_cleaner,targs)){
		fprintf(stderr, "Could not create pthread!\n");
		free(q);
		free(targs);
		exit(EXIT_FAILURE);
	}


	char *l;
	while((l=readline()) != NULL){
		if(!is_enqueued(q,l)){
			enqueue(q,l);
			printf("%s\n",l);
		}else
			free(l);
	}

	targs->no_lines=1;

	if(pthread_join(queue_cleaner_thread,NULL)){
		fprintf(stderr, "Could not join pthread!\n");
		free(q);
		free(targs);
		exit(EXIT_FAILURE);
	}
		
	free(q);
	free(targs);
	return EXIT_SUCCESS;
}
