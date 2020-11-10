#ifndef MYQUEUE_
#define MYQUEUE_

#include <pthread.h>

typedef struct queue queue_t;  // later on, we can also change to void pointer type to queue_t;
				// the real queue is defined in queue.c . Then we need typecast to the real type when parameters in the main.c are passing to the queue.c //YAO

/*
 ** The default queue size is 5
 */
#ifndef QUEUE_SIZE
    #define QUEUE_SIZE 5
#endif



/*
 **  Make some changes here to define the type of element that will be stored in the queue
 */

typedef void* element_ptr_t; //set as void pointer to allow storing all types of element/ YAO

void pthread_err_handler( int err_code, char *msg, char *file_name, int line_nr );

/*
 **  Creates (memory allocation!) and initializes the queue and prepares it for usage
 **  Return a pointer to the newly created queue
 **  Returns NULL if queue creation failed
 */
queue_t* queue_create();

/*  
 **  Add an element to the queue
 **  Does nothing if queue is full
 */
void queue_enqueue(queue_t* queue, element_ptr_t element);

/*
 **  Delete the queue from memory; set queue to NULL
 **  The queue can no longer be used unless queue_create is called again
 */
void queue_free(queue_t** queue);

/*
 **  Return the number of elements in the queue
 */
int queue_size(queue_t* queue);

/*
 **  Return a pointer to the top element in the queue
 **  Returns NULL if queue is empty
 */
element_ptr_t* queue_top(queue_t* queue);

/*
 **  Remove the top element from the queue
 **  Does nothing if queue is empty
 */
void queue_dequeue(queue_t* queue);

/*
 **  Print all elements in the queue, starting from the front element
 */
void queue_print(queue_t *queue);

#endif //MYQUEUE_


