#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int open_file(const char* file_name){
    int fd;
    if((fd = open(file_name, O_RDONLY)) == -1){
        err(1, "File could not open!");
    }
    return fd;
}

void read_file(int fd){
    char c;
    ssize_t res;
    while((res = read(fd, &c, sizeof(c))) == sizeof(c)){
        if(write(1, &c, sizeof(c)) != sizeof(c)){
            err(1, "Could not write to stdout!");
        }
    }
    if(res != 0){
        err(2, "Read has failed!");
    }
    if(fd != 0){
        close(fd);
     }
}

int main(int argc, char** argv) {
    if(argc == 1){
        read_file(0);
        exit(0);
    }

	for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-") == 0){
            read_file(0);
        }else{
            int fd = open_file(argv[i]);
            read_file(fd);
        }
	}

	exit(0);
}
