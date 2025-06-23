#include <stdint.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(2, "input should be number");
    }
    return num;
}

int dup2_wrapper(int newFd, int oldFd) {
    int fd = dup2(newFd, oldFd);
    if(fd == -1) {
        err(1, "dup2");
    }

    return fd;
}

ssize_t read_wrapper(int fd, void* buff, ssize_t bytes) {
    ssize_t res = read(fd, buff, bytes);
    if(res == -1) {
        err(2, "read");
    }
    return res;
}

ssize_t write_wrapper(int fd, const void* buff, ssize_t bytes) {
    ssize_t res = write(fd, buff, bytes);
    if(res == -1) {
        err(3, "write");
    }
    return res;
}

void pipe_wrapper(int fdPipe[2]) {
    if(pipe(fdPipe) == -1) {
        err(4, "pipe");
    }
}

pid_t fork_wrapper(void) {
    pid_t pid = fork();
    if(pid == -1) {
        err(5, "fork");
    }
    return pid;
}

int main(int argc, char** argv) {
	if(argc != 4){
        errx(1, "invalid args count");
	}
    
    int n = convertToInt(argv[2]);
    
    int fdUrandom = open("/dev/urandom", O_RDONLY);
    if(fdUrandom == -1){
        err(2, "open failed");
    }

    int fdNull = open("/dev/null", O_WRONLY);
    if(fdNull == -1){
        err(3, "open failed");
    }

    int fdOut = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if(fdOut == -1){
        err(4, "open failed");
    }

    int status = 0;
    for(int i = 0; i < n; i++){
        int fdPipe[2];
        pipe_wrapper(fdPipe);

        pid_t pid = fork();
        if(pid == 0){
            close(fdUrandom);
            close(fdPipe[1]);
            close(fdOut);

            dup2_wrapper(fdPipe[0], 0);
            close(fdPipe[0]);

            dup2_wrapper(fdNull, 1);
            dup2_wrapper(fdNull, 0);
            close(fdNull);

            execl(argv[1], argv[1], (char*)NULL);
            err(5, "execl failed");
        }

        close(fdPipe[0]);

        char buff[UINT16_MAX];
        uint16_t toWrite;
    
        read_wrapper(fdUrandom, &toWrite, sizeof(toWrite));
        read_wrapper(fdUrandom, &buff, toWrite);
        write_wrapper(fdPipe[1], &buff, toWrite);

        close(fdPipe[1]);
    
        int result;
        if(wait(&result) == -1){
            err(6, "wait failed");
        }
        if(WIFSIGNALED(result)){
            write_wrapper(fdOut, &buff, toWrite);
            status = 42;
            break;
        }
    }

    close(fdOut);
    close(fdNull);
    close(fdUrandom);

    return status;
}
