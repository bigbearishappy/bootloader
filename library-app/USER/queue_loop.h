#ifndef QUEUE_LOOP_H
#define QUEUE_LOOP_H
#include <string.h>

typedef enum{FALSE = 0,TRUE = 1} bool;

#ifndef MAX_SIZE
#define MAX_SIZE	5
#endif

#define QUEUEISFULL -1
#define QUEUEISEMPTY -2

typedef struct{
	volatile unsigned int head;
	volatile unsigned int tail;
	unsigned int size;
	char *buffer;
}Queue_l;

void Queueinit(Queue_l *queue,char *buffer,unsigned int size);
bool Queueisempty(Queue_l *queue);
bool Queueisfull(Queue_l *queue);
char Queueputc(Queue_l *queue,char data);
char Queuegetc(Queue_l *queue);
void Queueflush(Queue_l *queue);

#endif
