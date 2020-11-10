#include "gateway.h"


#ifndef TIMEOUT
	#error "Please define TIMEOUT for socket."
#endif

#define PORT 1234

/* global variable */

pollfd_ptr_t fds = NULL;
int gateway_list_errno;

/* defined in main */
extern FILE *fp_fifo;
extern char *send_buf; 


typedef struct element{
	Socket socket;
	time_t ts;
	bool new_con;
}element_t;

/* helper*/
static int check_active_client(int socket_sz);

/*
 * Copy the 'content' of src_element to dst_element.
 */
void gateway_element_copy(element_ptr_t *dest_element, element_ptr_t src_element)
{  
  element_t *p;
  p = (element_t *) malloc( sizeof(element_t) );
  assert ( p != NULL );
  *p = *(element_t *)src_element;
  *dest_element =p;
}


/*
 * Clean up element, including freeing memory if needed
 */
void gateway_element_free(element_ptr_t *element)
{
	free(*element);
	*element = NULL; //important
}

/*
 * Print 1 element to stdout. 
 */
void gateway_element_print(element_ptr_t element)
{
	printf("\nThis is element_ptr\n");
	/*
	putchar('\n');
  	printf("sensor id is %hd\t ",((element_t *)element)->sensor_id);  //print the data in the struct.
	printf("room id is %hd\t ",((element_t *)element)->room_id);
	printf("avg is %lf\n ",((element_t *)element)->avg);*/
}

/*
 * Compare two element elements; returns -1, 0 or 1 
 */
int gateway_element_compare(element_ptr_t x, element_ptr_t y)
{
	printf("element compare is called \n\n");
	
	if(x == NULL || y == NULL)
		return -1;
	
	if(((element_t *)x)->socket ==  ((element_t *)y)->socket)
		return 1;
	else 
		return 0;

}



/* return a server socket */
/*Socket gateway_init(){
 	list =list_create(element_copy, element_free, element_compare, element_print);
	
	Socket server = tcp_passive_open(PORT);
	// insert element of server
	element_t* ele = (element_t*)malloc(sizeof(element_t));
	ele->socket = server;
	ele->ts = time(NULL); // set the current time
	list_insert_at_index(list, ele, 0); 
	
	// add fd for server
	fds = (pollfd_ptr_t) malloc(sizeof(pollfd_t));
	fds[0].fd = get_socket_descriptor(server);
	fds[0].events = POLLIN;
	
	return server; 
	return NULL;
}*/

void gateway_run(queue_t* queue){
    
	int presult;
	uint16_t sensor_id;
	double temperature;
	time_t timestamp;
	int bytes, poll_result, socket_sz;
		
	//local variable
	list_ptr_t list =list_create(gateway_element_copy, gateway_element_free, gateway_element_compare, gateway_element_print);
	//gateway initialization
	Socket server = tcp_passive_open(PORT);
	// insert element of server
	element_t* ele = (element_t*)malloc(sizeof(element_t));
	ele->socket = server;
	ele->ts = time(NULL); // set the current time
	list_insert_at_index(list, ele, 0); 
	free(ele);
	
	// add fd for server
	fds = (pollfd_ptr_t) malloc(sizeof(pollfd_t));
	fds[0].fd = get_socket_descriptor(server);
	fds[0].events = POLLIN;
	
		
	while(1){
		socket_sz = list_size(list); 
		poll_result = poll(fds,socket_sz,500); // client + server
		// printf("poll_result is %d when client size is %d\n", poll_result, client_sz);
		if(poll_result > 0){
			//server action
			printf("\nserver fd[0] is polled? %d \n",fds[0].revents == POLLIN);
			if(fds[0].revents == POLLIN){ //server action, a new tcp connection request received.
			
				Socket client = tcp_wait_for_connection(server);				
				socket_sz += 1;  // update client size
				// add client to list, the initial time is set to the current time.
				element_t* ele = (element_t*)malloc(sizeof(element_t));
				ele->socket = client;
				ele->ts = time(NULL); // set the current time
				
				ele->new_con = true;
				list_insert_at_index(list, ele, socket_sz); // insert at the end
				free(ele);
				printf("incoming client connection\n");
				// printf("the number of connection is %d\n",list_size(list));
				// add fd for client
				fds = realloc(fds,sizeof(pollfd_t)*(socket_sz));
				fds[socket_sz-1].fd = get_socket_descriptor(client);
				fds[socket_sz-1].events = POLLIN;
			}
			
			// clients action
			int i;
			for(i = 1; i < socket_sz; i++){
				printf("\nclient fd[%d] is polled? %d \n", i, fds[i].revents == POLLIN);
				if(fds[i].revents == POLLIN){
					printf("\nread client info with fd[%d]\n", i);			
					element_t* ele =  (element_t*)list_get_element_at_index(list,i);	
					Socket client = ele->socket;
					time_t last_active = ele->ts;
					
					printf("last active is %ld\n", last_active);
					
					bytes = tcp_receive( client, (void *)&sensor_id, sizeof(sensor_id));
					// bytes == 0 indicates tcp connection teardown
					assert( (bytes == sizeof(sensor_id)) || (bytes == 0) );	
					bytes = tcp_receive( client, (void *)&temperature, sizeof(temperature));
					assert( (bytes == sizeof(temperature)) || (bytes == 0) );
					bytes = tcp_receive( client, (void *)&timestamp, sizeof(timestamp));
					assert( (bytes == sizeof(timestamp)) || (bytes == 0) );  
					//update time stamp
					ele->ts = timestamp;
					
					if(ele->new_con){
					//log process
						presult = pthread_mutex_lock( &fifo_mutex );
						pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
						asprintf( &send_buf, "Sensor id %d opened a new connection\n", sensor_id);
						if ( fputs( send_buf, fp_fifo ) == EOF ){
						  fprintf( stderr, "Error writing data to fifo\n");
						  exit( EXIT_FAILURE );
						} 
						FFLUSH_ERROR(fflush(fp_fifo));
						free( send_buf );
						ele->new_con = false;
						presult = pthread_mutex_unlock( &fifo_mutex );
						pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
					}
					if (bytes) {
						
						if((timestamp - last_active) > TIMEOUT){
							printf("timeout\n");
							// ele->flag = 1;
							fds[i].fd = -1;  // dont poll 
							fds[i].events = 0;
							tcp_close( &client );
							// list_free_at_index(list,i); // free the client at index i
							// socket_sz = list_size(list); 
						}else{
							//write packet to queue	
							sensor_data_t *packet =  (sensor_data_t *) malloc(sizeof(sensor_data_t));
							packet->sensor_id = sensor_id;
							packet->temperature = temperature;
							packet->time_stamp = timestamp;
							printf("Writer: reads from file: sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", sensor_id, temperature, (long int)timestamp );
							queue_enqueue(queue, packet); //if queue is full, data loss.
						}
					}else{
							//TCP client terminated connection, thus the corresponding socket at server side should also be closed.
							// do not poll on this fd
							fds[i].fd = -1;
							fds[i].events = 0;
						  tcp_close(&client);
					}
					
			
				}
			}
			
			
			// by setting fd. events to zero, the return value revents will not be POLLIN. Thus it can be assumed that TCP client is terminated.
			// if all clients are teared down, jump out of loop and close server. This feqture is optional, because sensor node connection is comming in future while keeping the server running forever.
			if(check_active_client(socket_sz) == 0){
				break;
			}
			
			usleep(100);
		}	// poll_result > 0
	} // while(1)


	tcp_close(&server);
	free(fds);	
	list_free(&list);	
	pthread_exit( NULL );
}

static int check_active_client(int socket_sz){
	if(socket_sz !=0){
		int b=0;
		//stating from 1. At index 0 server is stored.
		for(int i =1; i<socket_sz; i++){
			b = b || fds[i].events;
		}
		if(b == 0){ // all clients are set to not poll
			return 0;
		}else{// at least one client wait for polling
			return 1; 
		}
	}
	else
		return -1;
}





