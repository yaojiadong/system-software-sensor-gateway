#include "datamgr.h"


#ifndef SET_MIN_TEMP
//	#define SET_MIN_TEMP	17	// generate log event if running avg becomes smaller
	#error	"Please define SET_MIN_TEMP"
#endif

#ifndef SET_MAX_TEMP
//	#define SET_MAX_TEMP	24	// generate log event if running avg becomes bigger
	#error "Please define SET_MAX_TEMP"
#endif

#ifndef TIMEOUT
	#define TIMEOUT 60   // timeout 60 sec
#endif

#define	AVG_BUFFER_SIZE 5
#define NUM_AVG		5
// #define LOG_FILE "log"
#define mapping_file "room_sensor.map"
// #define data_file "sensor_data"

/* element stored in the list*/
struct element{
	uint16_t sensor_id;
	uint16_t room_id;
	double avg;
	time_t time_stamp;
};

/* gloable variable */

/* list is used to store the sensor data and perform the moving avg. */
list_ptr_t list = NULL;
int datamgr_list_errno;

/*defined in main*/
extern FILE *fp_fifo;
extern char *send_buf; 

/* helper*/
static double cal_avg(double temperature);
static void delete_inacitve_node(list_ptr_t list, time_t start_time);
static element_t * search_sensor_id(list_ptr_t list , int16_t sensor_id);
static bool if_duplicate(uint16_t sensor_id,	time_t ts, int reader_id);

/*
 * Copy the 'content' of src_element to dst_element.
 */
void datamgr_element_copy(element_ptr_t *dest_element, element_ptr_t src_element)
{
  element_t *p;
  p = (element_t *) malloc( sizeof(element_t) );
  assert ( p != NULL );
  *p = *(element_t *)src_element;
  *dest_element = p;
}


/*
 * Clean up element, including freeing memory if needed
 */
void datamgr_element_free(element_ptr_t *element)
{
  free(*(element_t **)element);
  *(element_t **)element =NULL;
}

/*
 * Print 1 element to stdout. 
 */
void datamgr_element_print(element_ptr_t element)
{
  printf("%lf\n",((element_t *)element)->avg);  //print the data in the struct.
}

/*
 * Compare two element elements; returns -1, 0 or 1 
 */
int datamgr_element_compare(element_ptr_t x, element_ptr_t y)
{
 	if( (x == NULL) || (y==NULL))
		return -1;
	
	if( ((element_t*)x)->sensor_id == ((element_t *)y)->sensor_id ) 
		return 1;
	else 
		return 0;
}

void sensor_mapping()
{
	printf("sensor mapping is called\n");
	list = list_create(datamgr_element_copy, datamgr_element_free, datamgr_element_compare, datamgr_element_print);
	FILE *fp;
	uint16_t temp1,temp2;
	
	fp = fopen(mapping_file, "r");
	if(fp == NULL)
	{
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }
	
	//every time read two values, i.e. room id and sensor id
	while(fscanf(fp,"%" SCNu16 " %" SCNu16 "\n", &temp1, &temp2) == 2 )	
	{
		element_t *ele_ptr = (element_t *) malloc(sizeof(element_t));
		// initialization of element_t
		ele_ptr->room_id = temp1;
		ele_ptr->sensor_id = temp2;  //first give value, then do inserting.
		ele_ptr->avg = 0;
		ele_ptr->time_stamp = 0;
		list = list_insert_at_index(list,ele_ptr,0);  // insert in the beginning, 
		
		free(ele_ptr);
	}
	fclose(fp);
}



void datamgr(queue_t* queue)
{
	sensor_mapping();
	printf("collecting data is called\n");
	uint16_t sensor_id;
	double temperature;
	time_t ts;
	sensor_data_t *packet = NULL;
	int presult;

	char* file_name = "data manager output";
	FILE *fp = fopen(file_name,"w");
	if(fp == NULL){
	  perror("Error while opening the file.\n");
	  exit(EXIT_FAILURE);
	}

	// fp_log = fopen(LOG_FILE, "w");
	//FILE_ERROR(fp_log,"Couldn't create log\n");

	while(1)
	{
		// printf("Datamgr: entering loop\n");
		// mutex lock
		presult = pthread_mutex_lock( &flag_mutex );
		pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
		
		sensor_data_t **top_ele_ptr = (sensor_data_t **)queue_top(queue);
			
		if(top_ele_ptr != NULL ){
			printf("Datamgr: queue top ptr is not NULL\n");
			packet = *top_ele_ptr;
			if(packet!= NULL){
				sensor_id = packet->sensor_id;
				temperature = packet->temperature;
				ts = packet->time_stamp ;
				

					bool duplicate = if_duplicate(sensor_id, ts,DATAMGR);	
					if(!duplicate){	
						// delete not active node
						delete_inacitve_node(list, ts);
				
						//find the element of the received sensor id 
						element_t * ele = search_sensor_id(list,sensor_id);
						if(ele != NULL){
							ele-> time_stamp = ts;
							ele-> avg = cal_avg(temperature);

							//minic the reading process and output data to a file as feedback, verification purpose
							fprintf(fp,"sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n",sensor_id, temperature, (long int)ts );
							fflush(fp);
							//set flag as having read
							list_ele_t* ele_flag = list_get_element_at_index(list_flag,DATAMGR);  //reader_id = 0;
							ele_flag->flag = 1;

							// mutex lock
							presult = pthread_mutex_lock( &fifo_mutex );
							pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
							if (ele->avg < SET_MIN_TEMP){
								asprintf( &send_buf, "Room %hd  is too cold\n", ele->room_id);
								if ( fputs( send_buf, fp_fifo ) == EOF ){
								  fprintf( stderr, "Error writing data to fifo\n");
								  exit( EXIT_FAILURE );
								} 
								FFLUSH_ERROR(fflush(fp_fifo));
								free( send_buf );
							}
							if (ele->avg > SET_MAX_TEMP){				
								asprintf( &send_buf, "Room %hd is too hot\n", ele->room_id);
								if ( fputs( send_buf, fp_fifo ) == EOF ){
								  fprintf( stderr, "Error writing data to fifo\n");
								  exit( EXIT_FAILURE );
								} 
								FFLUSH_ERROR(fflush(fp_fifo));
								free( send_buf );
							}
							//mutex unlock
							presult = pthread_mutex_unlock( &fifo_mutex );
							pthread_err_handler( presult, "pthread_mutex_lock", __FILE__, __LINE__ );
							
						}else{ // corresponding sensor not found
							// fprintf(fp_log,"Sensor ID not found\n");
							fprintf(stderr, "Datamgr: Sensor ID not found\n");
							// perror("Sensor ID not found\n\n");
						}		
						
						
					}
						
			}else{
				// fprintf(fp,"count %d: packet is empty\n",count);
				printf("Datamgr: queue top is null\n");
			}
				
			// dequeue
			dequeue_flag = check_flag();
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
			// fprintf(fp,"count %d: top is null\n",count);
			printf("\nDatamgr::queue top ptr is null\n");
		}	

		// mutex unlock
		presult = pthread_mutex_unlock( &flag_mutex );
		pthread_err_handler( presult, "pthread_mutex_unlock", __FILE__, __LINE__ );	
		
		usleep(100);
	}
	
	// out of while loop. Reader stops reading.
	// the flag in this reader is not considered anymore. Do not free at index reader_id so that the sequences are maintained.
	list_ele_t* ele = list_get_element_at_index(list_flag,DATAMGR);
	ele->enabled = false; 	

	fclose(fp);
	pthread_exit( NULL );
}

static double cal_avg(double temperature){

	static int index = 0;
	static int flag = 0;  // flag to show if buf is full
	static double buf[AVG_BUFFER_SIZE];
	double sum = 0;
	
	buf[index] = temperature;
	index++;
	
	// check if the buf is full
	if(flag == 0){
		if(index == AVG_BUFFER_SIZE)
			flag = 1;
	}

	// take the modulo to store the temperature from the beginning(overwrite) if more than AVG_BUFFER_SIZE;
	index = index%AVG_BUFFER_SIZE;
	
	//calculate the average value
	if(flag == 1){ //buf is full
		for(int i=0; i< AVG_BUFFER_SIZE; i++){
			sum += buf[i];
		}
		return sum/AVG_BUFFER_SIZE;
	}else{ //flag is 0, buf is not full
		for(int i=0; i< index; i++){
			sum += buf[i];
		}
		return sum/index;
	}

}

static void delete_inacitve_node(list_ptr_t list, time_t ts){

	int size = list_size(list);
	
	int i =0;
	for(; i< size; i++){
		element_t* ele_ptr = list_get_element_at_index(list, i);
		time_t last_modified = ele_ptr-> time_stamp;
		if( (last_modified != 0) && ((ts - last_modified) > TIMEOUT)){
			list = list_free_at_index(list, i);
			size--;
			i--;
		}
	}
	
}


static element_t * search_sensor_id(list_ptr_t list , int16_t sensor_id)
{
	int size = list_size( list );
	int index = 0;
	if(list == NULL)
	{ 
		printf("\n%s\n","Datamgr: search sensor ID, list is null");
		return NULL;	
	}
	else{
		for(;index< size; index++)
		{
			element_t *ele =(element_t *)list_get_element_at_index(list,index);
			if(ele-> sensor_id == sensor_id)
				return ele;
		}	
		return NULL;
	}
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






