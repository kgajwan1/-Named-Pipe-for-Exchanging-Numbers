#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


/*error writing ret=-1 errno=32 perror: Broken pipe*/
/*Number read: 23640 Bytes read: 4*/

void main(int argc){
	 
         char* buff;
	 int bytes_to_read = 4;
        // int iter_limit = atoi(argv[1]); 
 	 buff = (char*)malloc(bytes_to_read*sizeof(char));
	 int _iteration = 0;
	 int fp = open("/dev/numpipe",O_RDWR);
	 if(fp == -1){
		 printf("error opening numpipe\n");
		 exit(0);
	 }

	 while(1){
		 printf("\nNumber read: %d", _iteration++);

		 
       		 int bytes_actually_read = 0;
		 bytes_actually_read = read(fp,&buff,bytes_to_read);
		 if(bytes_actually_read >0){
			 printf("bytes read = %d: ", bytes_actually_read);
		         printf("%s\n", &buff);
		}
		else{
			printf("error writing ret=-1 errno=32 perror: Broken pipe");
                        sleep(1);
		}
		
	}
	close(fp);
	return ;
}

