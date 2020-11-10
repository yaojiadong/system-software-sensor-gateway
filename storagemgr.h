#ifndef _STORAGEMGR_H_
#define _STORAGEMGR_H_

#include "common.h"


/*------------------------------------------------------------------------------
		definitions (defines, typedefs, ...)
------------------------------------------------------------------------------*/

/*defined in main*/
extern list_ptr_t list_flag;
extern int dequeue_flag; 
extern pthread_mutex_t flag_mutex, fifo_mutex;

/*storage manager*/
void storagemgr(queue_t* queue);


#endif /* _STORAGEMGR_H_ */

