#include "queue_loop.h"

void Queueinit(Queue_l *queue,char *buffer,unsigned int size)
{
	queue->head = 0;
	queue->tail = 0;
	queue->size = size;
	queue->buffer = buffer;
	memset(queue->buffer,0x00,size);
}

bool Queueisempty(Queue_l *queue)
{
    return queue->head == queue->tail ? TRUE:FALSE;
}

bool Queueisfull(Queue_l *queue)
{
	return queue->tail-queue->head==MAX_SIZE||queue->head-queue->tail==1\
	?TRUE:FALSE;
}

char Queueputc(Queue_l *queue,char data)
{
	if(!Queueisfull(queue)){
		if(queue->tail==MAX_SIZE){
			queue->tail = 0;
    		queue->buffer[queue->tail++] = data;
    		return TRUE;
    	}
    	else{
    		queue->buffer[queue->tail++] = data;
    		return TRUE;
    	}
	}
	else
		return QUEUEISFULL;		
}

char Queuegetc(Queue_l *queue)
{
    if(!Queueisempty(queue)){
    	if(queue->head==MAX_SIZE){
    		queue->head = 0;
    		return queue->buffer[queue->head++];
    	}
    	else
    		return queue->buffer[queue->head++];
    	//printf("");
    }
    else
    	return QUEUEISEMPTY;
}

void Queueflush(Queue_l *queue)
{
	queue->head = 0;
	queue->tail = 0;
	memset(queue->buffer,0x00,queue->size);
}
