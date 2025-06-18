#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

typedef struct{
    uint64_t next;
    uint8_t user_data[504];
} node;

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(1, "fstat error");
    }
    return st.st_size;
}

ssize_t read_wrapper(int fd, void* buff, ssize_t count){
    ssize_t res = read(fd, buff, count);
    if(res == -1){
        err(2, "read error");
    }
    return res;
}

ssize_t write_wrapper(int fd, void* buff, ssize_t count){
    ssize_t res = write(fd, buff, count);
    if(res == -1){
        err(3, "write error");
    }
    return res;
}

off_t lseek_wrapper(int fd, off_t offset, int whence){
    off_t o = lseek(fd, offset, whence);
    if(o == -1){
        err(4, "lseek error");
    }
    return o;
}

int main(int argc, char** argv) {
	if(argc != 2){
        err(5, "Invalid args count");
	}

	int input = open(argv[1], O_RDWR);
    if(input == -1){
        err(6, "open %s error", argv[1]);
    }

    node nd;
    if(file_size(input) % sizeof(nd) != 0){
        errx(7, "invalid input file");
    }

    char template[] = "tempXXXXXX";
    int tempFd = mkstemp(template);

    uint64_t current = 0;
    do{
        lseek_wrapper(input, current * sizeof(nd), SEEK_SET);

        if(read_wrapper(input, &nd, sizeof(nd)) != sizeof(nd)){
            errx(8, "eof");
        }
        lseek_wrapper(tempFd, current * sizeof(nd), SEEK_SET);
        write_wrapper(tempFd, &nd, sizeof(nd));

        current = nd.next;
    }while (current != 0);
    
    uint8_t zero = '\0';
    lseek_wrapper(input, 0, SEEK_SET);

    int inputSize = file_size(input);
    for(int i = 0; i < inputSize; i++){
        write_wrapper(input, &zero, sizeof(zero));
    }

    lseek_wrapper(input, 0, SEEK_SET);
    lseek_wrapper(tempFd, 0, SEEK_SET);

    uint8_t byte;
    while(read_wrapper(tempFd, &byte, sizeof(byte)) == sizeof(byte)){
        write_wrapper(input, &byte, sizeof(byte));
    }
    
    if(unlink(template) == -1){
        err(9, "could not rm temp file");
    }

    close(input);
    close(tempFd);
}
