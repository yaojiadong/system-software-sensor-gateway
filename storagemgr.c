#include "storagemgr.h"

/*defined in main*/
extern FILE *fp_fifo;
extern char *send_buf; 

static bool if_duplicate(uint16_t sensor_id,	time_t ts, int reader_id);

void storagemgr(queue_t* queue){
	
	int presult;
	uint16_t sensor_id;
	double temperature;
	time_t ts;	
	sensor_data_t *packet = NULL;
	
	char* file_name = "storage manager output";
	FILE *fp = fopen(file_name,"w");
	if(fp == NULL){
	  perror("Error while opening the file.\n");
	  exit(EXIT_FAILURE);
	}
	
	presult = pthread_mutex_lock( &fifo_mutex );
	pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
	
	//assume that SQL connection to sever is established.
	asprintf( &send_buf, "Connection to SQL server established\n");
	if ( fputs( send_buf, fp_fifo ) == EOF ){
	  fprintf( stderr, "Error writing data to fifo\n");
	  exit( EXIT_FAILURE );
	} 
	FFLUSH_ERROR(fflush(fp_fifo));
	free( send_buf );
	
	presult = pthread_mutex_unlock( &fifo_mutex );
	pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
	
	//read queue
	while(1){
		
		// mutex lock
		presult = pthread_mutex_lock( &flag_mutex );
		pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
		
		sensor_data_t **top_ele_ptr = (sensor_data_t **)queue_top(queue);

		if(top_ele_ptr != NULL ){
			// printf("\nStoragemgr:queue top ptr is NOT null\n");
			packet = *top_ele_ptr;
			if(packet!= NULL){
				sensor_id = packet->sensor_id;
				temperature = packet->temperature;
				ts = packet->time_stamp ;
				
				bool duplicate = if_duplicate(sensor_id, ts,STORAGEMGR);	
				if(!duplicate){										
					fprintf(fp,"sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",sensor_id, temperature, (long int)ts );
					fflush(fp);
							

					// flag that the reader has read the packet.
					list_ele_t* ele = list_get_element_at_index(list_flag,STORAGEMGR); 
					ele->flag = 1;
				}	
			}else{
				// fprintf(fp,"count %d: packet is empty\n",count);
				printf("Stormgr: queue top is null\n");
			}
			
			//Reason to add a mutex: the dequeue_flag can be true for more than one reader at the same time.
			//consider that the Program comes to the point after if(dequeue_flag) and before queue_dequeue, after dequeue done once by one reader, another reader may dequeue again.
			dequeue_flag = check_flag();//temp_flag;
			if(dequeue_flag){
				queue_dequeue(queue);  
				//reset flag to 0				
				for(int i =0; i< list_size(list_flag);i++){
					list_ele_t* ele = list_get_element_at_index(list_flag,i);
					ele->flag = 0; 
				}
				dequeue_flag = 0;
			}	
		}else{
			printf("\nStoragemgr:queue top ptr is null\n");
		}	
				
		// usleep(20000);   //force a content switch to be able to let other readers to read from the queue and write to the output file. Must come up with another solution that is more robust.

		// mutex unlock
		presult = pthread_mutex_unlock( &flag_mutex );
		pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );	
			 
		usleep(100); //force a content switch
	}
	// out of while loop. Reader stops reading.
	// the flag in this reader is not considered anymore. Do not free at index reader_id so that the sequences are maintained.
	list_ele_t* ele = list_get_element_at_index(list_flag,STORAGEMGR);
	ele->enabled = false; 
	
    fclose(fp);
	
	pthread_exit( NULL );
	
}

/* It can be used when there is only one writer. Each reader does not want to read duplicate data. 
 * If more than one writer, do not use this function.
 */
static bool if_duplicate(uint16_t sensor_id,	time_t ts, int reader_id){
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
}
