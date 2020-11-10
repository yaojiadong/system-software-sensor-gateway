#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "myqueue.h"

/* define here a debug macro */
#ifdef DEBUG
#define DEBUG_PRINTF(...) \
	do {					  						\
		printf("In %s - function %s at line %d: ", __FILE__, __func__, __LINE__);		\
		printf(__VA_ARGS__);								\
	  } while(0)
#else
	#define DEBUG_PRINTF(...)
#endif


// mutex
extern pthread_mutex_t data_mutex;


/* The caller of the queue is responsible for implementing the functions below */
extern void element_print(element_ptr_t element);
extern void element_copy(element_ptr_t *dest_element, element_ptr_t src_element);
extern void element_free(element_ptr_t *element);

/*
 * The real definition of 'struct queue'
 */
struct queue {
  element_ptr_t *arr; // dynamic array containing data elements
  int current_size; // Counts number of elements in the queue
  int front, rear;
  // Remark for later: extra fields need to be added here to make it a thread-safe queue as needed for the assigment  /define function pointers here/ YAO
};


void pthread_err_handler( int err_code, char *msg, char *file_name, int line_nr )
{
	if( 0 != err_code ){
		fprintf( stderr, "\n%s failed with error code %d in file %s at line %d\n", msg, err_code, file_name, line_nr );
		// errno = err_code;
		perror("Error message: ");
	}
}


queue_t* queue_create()
{
	DEBUG_PRINTF("\n queue_create called\n\n");
	queue_t* q= (queue_t *) malloc ( sizeof(queue_t) );
	if (q == NULL ){
		return NULL;
	}else{	/*malloc without initialize the element*/
		q->arr=(element_ptr_t*)malloc(QUEUE_SIZE*sizeof(element_ptr_t));	
		// Initialization
		int i=0;
		for(;i<QUEUE_SIZE;i++){	
			q->arr[i]=NULL;
		}
		q->current_size = 0;
		q->front=0;
		q->rear=0;
		return q;
	}
 
}

void queue_free(queue_t** queue)
{
	DEBUG_PRINTF("\n queue_free called\n\n");
  	while((*queue )-> current_size != 0){
		queue_dequeue(*queue);  //free element
  	}
    free((*queue) ->arr);   //free the array in the queue
	(*queue)->arr = NULL;
	free(*queue);
	DEBUG_PRINTF("\nfree queue\n");
	*queue =NULL;
	DEBUG_PRINTF("\nset to NULL\n");
}

void queue_enqueue(queue_t* queue, element_ptr_t element)
{
	int presult;
	if(queue != NULL){
		
		printf("Enqueue: queue not null\n");
		// mutex lock
		presult = pthread_mutex_lock( &data_mutex );
		pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );	
		
		DEBUG_PRINTF("\n queue_enqueue called\n\n");
		if(queue->current_size >= QUEUE_SIZE){
			DEBUG_PRINTF("\n queue is full\n\n");
			printf("Enqueue: Enqueue failed. Queue is full.\n");
			free(element); //if queue is full, free the memory of the element.
		}else{	  
			DEBUG_PRINTF("\n queue_enqueue called successfully\n\n");
			// printf("Enqueue: processing enqueue\n\n");
			// printf("Enqueue: queue rear is %d\n",queue->rear);
			// printf("Enqueue: adress of element to be filled is %p\n", queue->arr[queue->rear]);
		
			element_copy( &queue->arr[queue->rear], element );  
			
			// printf("Enqueue: adress of element that is filled is %p\n", queue->arr[queue->rear]);
			
			queue->rear = (queue->rear == QUEUE_SIZE -1)? 0:(queue->rear)+1; 	  //move the rear to the next empty one.
			(queue->current_size)++;
			
			printf("Enqueue: Enqueue done. queue size is %d\n",queue->current_size);
			queue_print(queue);
		} 
	
		// mutex unlock
		presult = pthread_mutex_unlock( &data_mutex );
		pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );
	}
}

int queue_size(queue_t* queue)
{
	DEBUG_PRINTF("\n queue_size called\n\n");
	if(queue != NULL)
		return queue->current_size;
	else{ //queue not exists
		printf("\n queue_size: queue not exist \n\n");
		return -1;
	}
}

element_ptr_t* queue_top(queue_t* queue)
{
	DEBUG_PRINTF("\n queue_top called\n\n");
	if(queue != NULL){
		//queue is empty
		if(queue->current_size == 0){
			return NULL;
		}else{
			return &(queue->arr[queue->front]);
		}
	}else{ //queue not exists
		printf("\n queue_top: queue not exist \n\n");
		return NULL;
	}
}

void queue_dequeue(queue_t* queue)
{
	int presult;
	DEBUG_PRINTF("\n queue_dequeue called\n\n");
	if(queue != NULL){
		
		// mutex lock
		presult = pthread_mutex_lock( &data_mutex );
		pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
		
		//queue is empty
		if(queue->current_size == 0){
			printf("Dequeue: Dequeue failed. Queue is already empty.\n");
		}else{		
			// free is used pointer-type element. not used when primitive type eg. int is stored.
			element_free(&queue->arr[queue->front]);  		 
			queue->front =(queue->front == QUEUE_SIZE -1)? 0:(queue->front)+1; 		//move the front to the next one.	
			queue->current_size--;
			
			printf("Dequeue: Dequeue done. queue size is %d\n", queue->current_size);
			queue_print(queue);
		}
		
		//mutex unlock
		presult = pthread_mutex_unlock( &data_mutex );
		pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );
			
	}else{ //queue not exists
		printf("\n Dequeue: queue not exist \n\n");
	}
}


void queue_print(queue_t *queue)
{
	DEBUG_PRINTF("\n queue_print called\n\n");
	if(queue != NULL){
		int f = queue->front;
		for(;f<queue->front+queue->current_size;f++){
			element_print(queue->arr[f%QUEUE_SIZE] ); 
		}
		putchar('\n');
	}else{ //queue not exists
		printf("\n queue_print: queue not exist. \n\n");
	}
}
