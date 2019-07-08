#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char* argv[]){
	char* input_string = argv[1];
	int input_string_length = 4;
	int file_ptr = open("/dev/numpipe", O_WRONLY);
	if(file_ptr == -1){
		printf("error opening numpipe!\n");
		exit(0);
	}
	int _iter = 0;
	while(1){
		printf("writing string %s\n", input_string);
		int number_of_bytes_written = write(file_ptr, input_string, input_string_length);
		if(number_of_bytes_written <= 0){
			printf("error writing ret=0 errno=0 perror:Success\n");
			//exit(0);
		}
		else{
			printf("Writing : %d\n", ++_iter);
			
			//printf("Write successful!\n");
			printf("Bytes written = %d\n", number_of_bytes_written);
		}
		sleep(1);
	}
	close(file_ptr);
	return 0;
}
