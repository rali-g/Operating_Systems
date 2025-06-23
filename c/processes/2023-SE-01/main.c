#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "stdlib.h"

void processFile(char* filename){
    int md5sum[2];
    if(pipe(md5sum) == -1){
        err(9, "md5sum failed");
    }
    pid_t pid = fork();
    if(pid == -1){
        err(10, "fork failed");
    }
    if(pid == 0){
        close(md5sum[0]);
        if(dup2(md5sum[1], 1) == -1){
            err(11, "dup2 failed");
        }
        close(md5sum[1]);
        if(execlp("md5sum", "md5sum", filename, (char*)NULL) == -1){
            err(12, "execlp failed"); 
        }
    }
    close(md5sum[1]);

    char hashFilename[4096];
    strcpy(hashFilename, filename);
    strncat(hashFilename, ".hash", sizeof(hashFilename) - strlen(hashFilename) - 1);

    int fd = open(hashFilename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if(fd == -1){
        err(13, "open failed");
    }

    ssize_t res;
    char ch;
    while((res = read(md5sum[0], &ch, sizeof(ch))) == sizeof(ch)){
        if(ch == ' '){
            break;
        }
        if(write(fd, &ch, sizeof(ch)) == -1){
            err(14, "write failed");
        }
    }
    if(res == -1){
        err(15, "read failed");
    }

    close(md5sum[0]);
    if(wait(NULL) == -1){
        err(15, "wait error");
    }
    close(fd);
}

int main(int argc, char** argv) {
	if(argc != 2){
        errx(1, "invalid args count");
	}

	int find[2];
	if(pipe(find) == -1){
        err(2, "pipe failed");
	}

	pid_t pid = fork();
	if(pid == -1){
        err(3, "fork failed");
	}
	if(pid == 0){
        close(find[0]);
        if(dup2(find[1], 1) == -1){
            err(4, "dup2 failed");
        }
        close(find[1]);
        if(execlp("find", "find", argv[1], "-type", "f", "-not", "-name", "*.hash", "-print0", (char*)NULL) == -1){
            err(5, "find failed");
        }
	}
	close(find[1]);

	char byte;
	char filename[4096];
	ssize_t res;
	int index = 0;
	int childrenCount = 1;

	while((res = read(find[0], &byte, sizeof(byte))) == sizeof(byte)){
        filename[index++] = byte;
        if(byte == '\0'){
            index = 0;
            pid_t curr_pid = fork();
            if(curr_pid == -1){
                err(6, "fork failed");
            }
            if(curr_pid == 0){
                close(find[0]);
                processFile(filename);
                exit(0);
            }
            childrenCount++;
        }
	}
	if(res == -1){
        err(7, "read failed");
	}
    close(find[0]);

    for(int i = 0; i < childrenCount; i++){
        int status = 0;
        if(wait(&status) == -1){
            err(7, "wait failed");
        }
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) != 0){
                errx(8, "child exited with status %d", WEXITSTATUS(status));
            }
        }else{
            errx(9, "child was killed");
        }
    }
    return 0;
}
