#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_NUM 4194304
#define MAX_RAM (9 * 1024 * 1024)
#define CHUNK_SIZE (MAX_RAM / sizeof(uint32_t))

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "Operation fstat failed!");
    }
    return st.st_size;
}

int cmp(const void* a, const void* b){
    uint32_t x = *(const uint32_t*)a;
    uint32_t y = *(const uint32_t*)b;
    if(x < y) return -1;
    if(x > y) return 1;
    return 0;
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid args count!");
	}

    int fd1;
    int fd2;
    if((fd1 = open(argv[1], O_RDONLY)) == -1){
        err(3, "Could not open %s", argv[1]);
    }
    if((fd2 = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
        err(4, "Could not open %s", argv[2]);
    }
    int size1 = file_size(fd1);
    if(size1 % sizeof(uint32_t) != 0 || (unsigned long)size1 > MAX_NUM * sizeof(uint32_t)){
        errx(5, "Invalid input!");
    }

    int total_numbers = size1 / sizeof(uint32_t);
    uint32_t* buffer = malloc(size1);
    if(!buffer){
        err(6, "Malloc has failed!");
    }
    
    char tmp[64];
    int chunks = total_numbers / CHUNK_SIZE;
    for(int i = 0; i < chunks; i++){
        if(read(fd1, buffer, CHUNK_SIZE * sizeof(uint32_t)) != CHUNK_SIZE * sizeof(uint32_t)){
            err(7, "Could not read input file!");        
        }   
        qsort(buffer, CHUNK_SIZE, sizeof(uint32_t), cmp);
        snprintf(tmp, sizeof(tmp), "chunk_%d", i);
        int fd_tmp;
        if((fd_tmp = open(tmp, O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
            err(8, "Error while opening tmp file!");
        }
        if(write(fd_tmp, buffer, CHUNK_SIZE * sizeof(uint32_t)) != CHUNK_SIZE * sizeof(uint32_t)){
            err(8, "Could not write in tmp file!");
        }
        close(fd_tmp);
    }
    
    free(buffer);
    close(fd1);
    close(fd2);
}
