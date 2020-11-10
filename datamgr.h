#ifndef _DATAMGR_H_
#define _DATAMGR_H_

#include "common.h"

/*defined in main*/
extern list_ptr_t list_flag;
extern int dequeue_flag; 
extern pthread_mutex_t flag_mutex, fifo_mutex;

typedef struct element element_t;
// typedef struct sensor_data sensor_data_t;

void element_copy(element_ptr_t *dest_element, element_ptr_t src_element);

void element_free(element_ptr_t *element);

void element_print(element_ptr_t element);

int element_compare(element_ptr_t x, element_ptr_t y);

void sensor_mapping();

void datamgr(queue_t*);



#endif /* _DATAMGR_H_ */


