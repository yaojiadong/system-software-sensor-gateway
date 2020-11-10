#ifndef _COMMON_H_
#define _COMMON_H_

// GNU source is needed for asprintf
#define _GNU_SOURCE 
#include <stdio.h> 
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include "list.h"
#include "myqueue.h"
#include "errmacros.h"

#define FIFO_NAME 	"MYFIFO" 

/*------------------------------------------------------------------------------
		global variables
------------------------------------------------------------------------------*/

/* enumeration of Readers */
enum  {DATAMGR=0, STORAGEMGR};

/*------------------------------------------------------------------------------
		definitions (defines, typedefs, ...)
------------------------------------------------------------------------------*/

/*
 * The element structure is used for the readers. Each reader has one element that is stored in the list_flag
 * flag is marked, if the reader has read the queue top element.
 * enabled is to indicate the flag is considered.
 */
typedef struct list_ele{
	int flag;
	bool enabled;
}list_ele_t;

/*
 * The sensor data structure.
 */
typedef struct sensor_data{
  uint16_t sensor_id;
  double temperature;
  time_t time_stamp;	
}sensor_data_t;


/* Chech if the enabled flags are set. dequeue is only allowed when all flags are set.*/
int check_flag();
		

#endif /* _COMMON_H_ */

