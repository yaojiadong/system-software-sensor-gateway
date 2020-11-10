/*******************************************************************************
*  FILENAME: gateway.h							       
*
* Version V1.0		
* Author: Yao Jiadong
*
*  
*									    
*******************************************************************************/

#ifndef __GATEWAY_H__
#define __GATEWAY_H__


#include "common.h"
#include <poll.h>
#include "tcpsocket.h"

/*defined in main*/
extern list_ptr_t list_flag;
extern int dequeue_flag; 
extern pthread_mutex_t fifo_mutex;

typedef struct pollfd pollfd_t;
typedef pollfd_t * pollfd_ptr_t;


void gateway_run(queue_t* queue);



#endif  //__GATEWAY_H__
