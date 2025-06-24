#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

int open_file(const char* file_name){
    int fd;
    if((fd = open(file_name, O_RDONLY)) == -1){
        err(1, "File %s could not open!", file_name);
    }
    return fd;
}

void read_file(int fd, int* counter, bool numerate){
    char c;
    ssize_t res;
    bool new_line = 1;

    while((res = read(fd, &c, sizeof(c))) == sizeof(c)){
        if(numerate && new_line){
            char buff[32];
            int len = snprintf(buff, sizeof(buff), "%d ", (*counter)++);
            if (write(1, buff, len) != len) {
                err(1, "Write of line number failed");
            }
            new_line = 0;
        }
        if(write(1, &c, sizeof(c)) != sizeof(c)){
            err(1, "Could not write to stdout!");
        }
        if(c == '\n'){
            new_line = 1;
        }
    }
    if(res != 0){
        err(2, "Read has failed!");
    }
    close(fd);
}

int main(int argc, char** argv) {
    int counter = 1;
    bool numerate = 0;
    int start = 1;
    if(argc > 1 && strcmp(argv[1], "-n") == 0){
        numerate = 1;
        start = 2;
    }
    if(argc == start){
        read_file(0, &counter, numerate);
        exit(0);
    }

    for(int i = start; i < argc; i++){
        if(strcmp(argv[i], "-") == 0){
            read_file(0, &counter, numerate);
        }else{
            int fd = open_file(argv[i]);
            read_file(fd, &counter, numerate);
        }
    }

    exit(0);
}
