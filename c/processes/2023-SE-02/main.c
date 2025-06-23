#include <err.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

pid_t fork_wrapper(void){
    pid_t pid = fork();
    if(pid == -1){
        err(26, "fork error");
    }
    return pid;
}

int dup2_wrapper(int oldFd, int newFd){
    int fd = dup2(oldFd, newFd);
    if(fd == -1){
        err(26, "dup2 failed");
    }
    return fd;
}

ssize_t read_wrapper(int fd, void* buff, size_t size){
    int res = read(fd, buff, size);
    if(res == -1){
        err(26, "read failed");
    }
    return res;
}

int pipe_wrapper(int fd[2]){
    int res = pipe(fd);
    if(res == -1){
        err(26, "pipe error");
    }
    return res;
}

int wait_wrapper(void){
    int status;
    if(wait(&status) == -1){
        err(26, "wait error");
    }
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(26, "child exited with status %d", WEXITSTATUS(status));
        }
    }else{
        errx(26, "child was killed");
    }
    return status;
}

int kill_wrapper(pid_t pid){
    int res = kill(pid,  SIGTERM);
    if(res == -1){
        err(26, "kill failed");
    }
    return res;
}

int main(int argc, char** argv) {
    if(argc < 2){
        errx(26, "at least 2 args");
    }

    pid_t pids[4096];
    int pipeFd[2];
    pipe_wrapper(pipeFd);
    
    for(int i = 1; i <= argc; i++){
        pid_t pid = fork_wrapper();
        if(pid == 0){
            close(pipeFd[0]);
            dup2_wrapper(pipeFd[1], 1);
            close(pipeFd[1]);

            if(execlp(argv[i], argv[i], (char*)NULL) == -1){
                err(26, "execlp failed");
            }
        }
        pids[i - 1] = pid;
    }

    close(pipeFd[1]);
    const char* pattern = "found it";
    
    char ch;
    size_t index = 0;
    while(read_wrapper(pipeFd[0], &ch, sizeof(ch)) == sizeof(ch)){
        if(ch == pattern[index]){
            index++;
        }else{
            index = 0;
        }

        if(index == strlen(pattern)){
            for(int i = 0; i < argc - 1; i++){
                kill_wrapper(pids[i]);
            }

            exit(0);
        }
    }

    for(int i = 0; i < argc - 1; i++){
         wait_wrapper();
    }

    exit(1);
}
