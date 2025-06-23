#include <unistd.h>
#include <err.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(1, "fstat failed");
    }
    return st.st_size;
}

typedef struct{
    char filename[8];
    uint32_t offset;
    uint32_t length;
}tuple;

void wait_wrapper(void) {
    int status;
    if(wait(&status) == -1) {
        err(12, "wait");
    }

    if(WIFEXITED(status)) {
        if(WEXITSTATUS(status) != 0) {
            errx(15, "Child exited with status %d", WEXITSTATUS(status));
        }
    } else {
         errx(16, "Child was killed");
    }
}

void xor(tuple t, int pipeFd){
    int fd = open(t.filename, O_RDONLY);
    if(fd == -1){
        err(8, "open failed");
    }
    if((size_t)file_size(fd) < (t.offset + t.length) * sizeof(uint16_t)){
        errx(9, "invalid file format");
    }

    if(lseek(fd, t.offset * sizeof(uint16_t), SEEK_SET) ==  -1){
        err(10, "lseek failed");
    }

    uint32_t numsRead = 0;
    uint16_t num;
    uint16_t xorRes = 0;
    ssize_t res;
    while((res = read(fd, &num, sizeof(num))) == sizeof(num) && numsRead < t.length){
        xorRes ^= num;
        numsRead++;
    }
    if(res == -1){
        err(11, "read failed");
    }
    if(write(pipeFd, &xorRes, sizeof(xorRes)) != sizeof(xorRes)){
        err(12, "write failed");   
    }

    close(fd);
}

int main(int argc, char** argv) {
	if(argc != 2){
        errx(2, "invalid argc count");
	}

	int in = open(argv[1], O_RDONLY);
	if(in == -1){
        err(3, "open %s failed", argv[1]);
	}

	if(file_size(in) % sizeof(tuple) != 0 || file_size(in) / sizeof(tuple) > 8){
        errx(4, "invalid input format");
	}
    
    int res;
    tuple t;
    uint16_t xorRes = 0;

    int childrenCount = 0;
    int pipeFd[2];
    if(pipe(pipeFd) == -1){
        err(5, "pipe failed");
    }

    while((res = read(in, &t, sizeof(t))) == sizeof(t)){
        childrenCount++;
        
        pid_t pid = fork();
        if(pid == -1){
            err(6, "fork failed");
        }
        if(pid == 0){
            close(in);
            close(pipeFd[0]);
            xor(t, pipeFd[1]);
            close(pipeFd[1]);
            exit(0);
        }
    }
    if(res != 0){
        err(7, "read failed");
    }
    
    close(pipeFd[1]);
    uint16_t childXor;
    ssize_t res2;
    while((res2 = read(pipeFd[0], &childXor, sizeof(childXor))) == sizeof(childXor)){
        xorRes ^= childXor;
    }
    if(res2 == -1){
        err(20, "read failed");
    }
    close(pipeFd[0]);

    for(int i = 0; i < childrenCount; i++){
        wait_wrapper();
    }

    char buff[256];
    snprintf(buff, sizeof(buff), "result: %.4X\n", xorRes);
    if(write(1, buff, strlen(buff)) != (ssize_t)strlen(buff)){
        err(21, "write error");
    }

	close(in);
}
