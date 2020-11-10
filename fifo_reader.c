#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <time.h>
#include <string.h>
#include "errmacros.h"

#define FIFO_NAME 	"MYFIFO" 
#define MAX 		80
#define LOG_FILE	"logFifo"

int main(void) { // FIFO reader
  FILE *fp, *fp_log; 
  int result;
  char *str_result;
  char recv_buf[MAX]; 
	  
  /* Create the FIFO if it does not exist */ 
  result = mkfifo(FIFO_NAME, 0666);
  CHECK_MKFIFO(result); 
  
  fp = fopen(FIFO_NAME, "r"); 
  printf("syncing with writer ok\n");
  FILE_OPEN_ERROR(fp);
  
  fp_log = fopen(LOG_FILE, "w");
  FILE_OPEN_ERROR(fp_log);
  static int seq_num = 0;
  while(1)
  {
    str_result = fgets(recv_buf, MAX, fp);

    if ( str_result != NULL )
    { 
		printf("Message received: %s", recv_buf); 
		recv_buf[strcspn(recv_buf,"\n")] = 0;		
		fprintf(fp_log, "<%d><%d><%s>\n", seq_num++, (int)time(NULL), recv_buf);
		// fprintf(fp_log, "<%d><%d><%s>\n", seq_num++, (int)time(NULL), recv_buf);
		fflush(fp_log);
    }
  } 

	fclose(fp_log);	
		
  result = fclose( fp );
  FILE_CLOSE_ERROR(result);
  
  exit(EXIT_SUCCESS);
}



