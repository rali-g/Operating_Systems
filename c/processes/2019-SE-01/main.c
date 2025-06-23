#include <string.h>
#include <err.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int convertToInt(char* str){
    char* endPtr;
    int res = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(1, "Argument should ne a number");        
    }
    return res;
}

void logInfo(int log, int start, int end, int exitStatus){
    char buff[1024];
    snprintf(buff, sizeof(buff), "%d %d %d\n", start, end, exitStatus);
    if(write(log, buff, strlen(buff)) != (ssize_t)strlen(buff)){
        err(10, "write failed");
    }
}

int main(int argc, char** argv) {
	if(argc < 3){
        errx(2, "Invalid args count");
	}
    
    int log = open("run.log", O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if(log == -1){
        err(3, "run.lod not opened");
    }

	int max = convertToInt(argv[1]);
	if(max < 1 || max > 9){
        errx(4, "Invalid max input");
	}
	const char* cmd = argv[2];

	int count = 0;
	int start;
	int end;

	while(count < 2){
        pid_t pid = fork();
        if(pid == -1){
            err(5, "fork failed");
        }
        if(pid == 0){
            if(argc > 3){
                execvp(cmd, argv + 2);
            }else{
                execlp(cmd, cmd, (char*)NULL);
            }
        }
        int status;
        start = time(NULL);
        if(wait(&status) == -1){
            err(5, "wait failed");
        }
        if(WIFEXITED(status)){
            end = time(NULL);
            if(WEXITSTATUS(status) != 0 && (end - start) < max){
                count++;
            }
        }else{
            errx(7, "child was killed");
        }

        logInfo(log, start, end, WEXITSTATUS(status));
	}

	close(log);
}
