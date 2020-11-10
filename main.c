/*
 author: Yao Jiadong
 date: 08.10.2020
*/

#include "common.h"
#include <pthread.h>
#include <sys/stat.h>
#include "gateway.h"
#include "storagemgr.h"
#include "datamgr.h"


// #define WR_FILE "sensor_data"  //the file from which the writer read and then write to queue
// #define RD_FILE "sensor_data_output" //the file read from the queue

/*Command line arguments: NUM_RD and NUM_WR*/

/****************** global variables**************/

FILE *fp_fifo;
char *send_buf; 

int NUM_WR=1, NUM_RD=2;
int list_errno;

int dequeue_flag = 0;
list_ptr_t list_flag;
queue_t* queue;
pthread_mutex_t data_mutex, flag_mutex, fifo_mutex;

// mutex
// pthread_mutex_t data_mutex  = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t flag_mutex = PTHREAD_MUTEX_INITIALIZER;

/********** definitions ******************/

/******* function declaration ***********/
//callback for queue
void element_print(element_ptr_t element);
void element_copy(element_ptr_t *dest_element, element_ptr_t src_element);
void element_free(element_ptr_t *element);
//callback for list
void list_element_copy(element_ptr_t *dest_element, element_ptr_t src_element);
void list_element_free(element_ptr_t *element);
int list_element_compare(element_ptr_t x, element_ptr_t y);
void list_element_print(element_ptr_t element);
//helper




int main( int argc, char* argv[] )
{
	// create FIFO
	int result;

	/* Create the FIFO if it does not exist */ 
	result = mkfifo(FIFO_NAME, 0666);
	CHECK_MKFIFO(result); 	

	fp_fifo = fopen(FIFO_NAME, "w"); 
	printf("syncing with reader ok\n");
	FILE_OPEN_ERROR(fp_fifo);
  
  
	
	//list is created for store the dequeue flag. The sequence corresponds to the reader sequence.
	list_flag = list_create(list_element_copy, list_element_free, list_element_compare, list_element_print);
  
	// initialize read_flag. The number is the same as the readers. 
	for (int i =0; i<NUM_RD; i++){
	  list_ele_t* p = (list_ele_t*) malloc(sizeof(list_ele_t));
	  p->flag = 0;
	  p->enabled = true;
	  list_insert_at_index(list_flag, p, NUM_RD); // always insert at the end
	  free(p); // list insert made a deep copy. So a free has to be done after insert.
	}
	
	//create queue which stores the data
	queue = queue_create();

	//initialize pthread
	int presult,i;
	pthread_t thread_WR[NUM_WR], thread_RD[NUM_RD];
  
	//mutex initialization
	pthread_mutex_init(&data_mutex, NULL);
	pthread_mutex_init(&flag_mutex, NULL);
	pthread_mutex_init(&fifo_mutex, NULL);
	
	//initialize gateway, create pthread
	// Socket server = gateway_init();
	presult = pthread_create( &thread_WR[0], NULL, (void *)gateway_run, (void *)queue );
	pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );	
	
	
	//initialize datamgr 
	// void sensor_mapping();
	presult = pthread_create( &thread_RD[0], NULL, (void *)datamgr, (void *)queue );
	pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
	
	//initialize storagemgr
	presult = pthread_create( &thread_RD[1], NULL, (void *)storagemgr, (void *)queue );
	pthread_err_handler( presult, "pthread_create", __FILE__, __LINE__ );
	
  
	//thread join
	for(i=0; i<NUM_WR; i++){
		// important: don't forget to join, otherwise main thread exists and destroys the mutex
		presult= pthread_join(thread_WR[i], NULL);
		pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );
	}

	for(i=0; i<NUM_RD; i++){
		presult= pthread_join(thread_RD[i], NULL);
		pthread_err_handler( presult, "pthread_join", __FILE__, __LINE__ );
	}

	//sleep(1);
	// printf("main: start to free queue\n");
	queue_free(&queue);
	// printf("main: start to free list_flag\n");
	
	list_free(&list_flag);
	
	fclose(fp_fifo);
  
	//queue free will call dequeue that contains date_mutex. So first free queue and then destroy data_mutex.
	presult = pthread_mutex_destroy( &data_mutex );
	pthread_err_handler( presult, "pthread_mutex_destroy", __FILE__, __LINE__ );
	presult = pthread_mutex_destroy( &flag_mutex );
	pthread_err_handler( presult, "pthread_mutex_destroy", __FILE__, __LINE__ );
	presult = pthread_mutex_destroy( &fifo_mutex );
	pthread_err_handler( presult, "pthread_mutex_destroy", __FILE__, __LINE__ );

	pthread_exit(NULL);
}



/*
 * Implement here private functions to copy, to print and to destroy an element. Do you understand why you need these functions? 

Because in the queue.c, the copy, print and destroy functions are needed. But these functions need not be known by queue.c. 
The real type is known by main, not by queue. The main has responsibility to typecast the void to the realy type. Only in this way can we know how to print the specific type of data(e.g. struct)

 * Later you will learn how you could avoid this by using funtion pointers. 
*
Dont understand how to avoid. Even use function pointers, still need copy print and destroy functions in main ! ????????????????????????
 * 
 */

/*
 * Print 1 element to stdout. 
 * If the defition of element_ptr_t changes, then this code needs to change as well.
 */
void element_print(element_ptr_t element)
{
  	sensor_data_t* temp = (sensor_data_t*)element;
	if(temp != NULL)
		printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", temp->sensor_id, temp->temperature, (long int)(temp->time_stamp));  
	else
		printf("element is not valid\n");
}

/*
 * Copy the content (e.g. all fields of a struct) of src_element to dst_element.
 * dest_element should point to allocated memory - no memory allocation will be done in this function
 * If the defition of element_ptr_t changes, then this code needs to change as well.
 */
void element_copy(element_ptr_t *dest_element, element_ptr_t src_element)
{
	if((sensor_data_t**)dest_element != NULL){
		*(sensor_data_t **)dest_element = (sensor_data_t *)src_element;
	}
	else{
		printf("Copy failed. Dest_element is NULL pointer\n");
	}
}

/*
 * Free the memory allocated to an element (if needed)
 * If the defition of element_ptr_t changes, then this code needs to change as well.
 */
void element_free(element_ptr_t *element)
{
	free(*element);
	*element =NULL;
}



/*
 * Copy the 'content' of src_element to dst_element.
 */
void list_element_copy(element_ptr_t *dest_element, element_ptr_t src_element)
{
  	list_ele_t *p = (list_ele_t*) malloc(sizeof(list_ele_t));
	assert( p != NULL );
	*p = *(list_ele_t *)src_element; // copy the content
	*(list_ele_t**)dest_element = p;  
}

/*
 * Clean up element, including freeing memory if needed
 */
void list_element_free(element_ptr_t *element)
{
  free(*(list_ele_t **)element);
  *(list_ele_t **)element =NULL;
}

/*
 * Print 1 element to stdout. 
 */
void list_element_print(element_ptr_t element)
{	
	list_ele_t* temp = (list_ele_t*)element;
	if(temp != NULL)
		printf("\nList_print: flag is %d, which is %d\n\n", temp->flag, temp->enabled);  
	else
		printf("element is not valid\n");
}

/*
 * Compare two element elements; returns -1, 0 or 1 
 */
int list_element_compare(element_ptr_t x, element_ptr_t y)
{
	//element compare is called in func: list_get_index_of_element
	printf("list element compare\n");
	return 0;	
}

/*
 *only set flag if all enabled readers have read it.
 */
int check_flag(){
	
	int size = list_size(list_flag);
	int i = 0;
	int result = 1;
	for(; i< size; i++){
		list_ele_t* ele = (list_ele_t*)list_get_element_at_index( list_flag,  i );
		if(ele->enabled == true){
			result = result && (ele->flag);
			if(result == 0)
				break;
		}
	}
	return result;
}

/* 
 * It can be used when there is only one writer. Each reader does not want to read duplicate data. 
 * If more than one writer, do not use this function.
 */
/* bool if_duplicate(	uint16_t sensor_id,	time_t ts, int reader_id){
	static 	uint16_t last_sensor_id = 0;
	static time_t last_ts = 0;
	static int last_reader_id = -1;
	
	if((sensor_id == last_sensor_id) && (ts ==last_ts) && (reader_id==last_reader_id))
		return true;
	else{
		last_sensor_id = sensor_id;
		last_ts = ts;
		last_reader_id = reader_id;
		return false;
	}
} */