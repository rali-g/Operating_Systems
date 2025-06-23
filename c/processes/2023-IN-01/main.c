#include <err.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>

const char* WORDS[] = {
    "tic ",
    "tac ", 
    "toe\n"
};

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(2, "input should be number");
    }
    return num;
}

void handleMessages(int totalWords, int fromFd, int toFd){
    bool written_final_signal = false;
    int count;
    ssize_t res;
    while((res = read(fromFd, &count, sizeof(count))) == sizeof(count)){
        if(count >= totalWords){
            if(!written_final_signal){
                if(write(toFd, &count, sizeof(count)) == -1){
                    err(19, "write error");
                }
            }
            return;
        }
        if(write(1, WORDS[count % 3], strlen(WORDS[count % 3])) == -1){
            err(20,  "write failed");
        }

        int next = count + 1;
        if(write(toFd, &next, sizeof(next)) == -1){
            err(21, "write failed");
        }
        if(next >= totalWords){
            written_final_signal = true;
        }
    }
    if(res == -1){
        err(22, "read failed");
    }
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "invalid args count");
	}
    int children = convertToInt(argv[1]);
    if(children < 1 || children > 7){
        errx(2, "invalid nc");
    }
    int totalWords = convertToInt(argv[2]);
    if(totalWords < 1 || totalWords > 35){
        errx(3, "invalid wc");
    }

    int pipes = children + 1;
    int pipeFd[8][2];

    for(int i = 0; i < pipes; i++){
        if(pipe(pipeFd[i]) == -1){
            err(4, "pipe failed");
        }
    }

    for(int i = 1; i <= children; i++){
        pid_t pid = fork();
        if(pid == -1){
            err(5, "fork failed");
        }
        if(pid == 0){
            int fromFd = pipeFd[i - 1][0];
            int toFd = pipeFd[i][1];

            for(int j = 0; j < pipes; j++){
                if(pipeFd[j][0] != fromFd){
                    close(pipeFd[j][0]);
                }
                if(pipeFd[j][1] != toFd){
                    close(pipeFd[j][1]);
                }
            }

            handleMessages(totalWords, fromFd, toFd);

            close(fromFd);
            close(toFd);

            exit(0);
        }
    }
    
    int fromFd = pipeFd[children][0];
    int toFd = pipeFd[0][1];

    for(int i = 0; i < pipes; i++){
        if(pipeFd[i][0] != fromFd){
            close(pipeFd[i][0]);
        }
        if(pipeFd[i][1] != toFd){
            close(pipeFd[i][1]);
        }
    }

    if(write(1, WORDS[0], strlen(WORDS[0])) == -1){
        err(10, "write failed");
    }

    int next = 1;
    if(write(toFd, &next, sizeof(next)) == -1){
        err(11, "write");
    }

    handleMessages(totalWords, fromFd, toFd);

    close(fromFd);
    close(toFd);

    for(int i = 0; i < children; i++){
        pid_t pid;
        int status;
        if((pid = wait(&status)) == -1){
            err(7, "wait failed");
        }

        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) != 0){
                errx(8, "child %d exited with status %d", pid, WEXITSTATUS(status));
            }
        }else{
            errx(9, "child %d was killed", pid);
        }
    }

    return 0;
}
