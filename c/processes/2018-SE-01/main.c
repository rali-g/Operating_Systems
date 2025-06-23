#include <err.h>
#include <sys/wait.h>
#include <unistd.h>

void pipe_wrapper(int fd[2]){
    int res = pipe(fd);
    if(res == -1){
        err(1, "pipe error");
    }
}

pid_t fork_wrapper(void){
    pid_t pid = fork();
    if(pid == -1){
        err(2, "fork error");
    }
    return pid;
}

int dup2_wrapper(int oldFd, int newFd){
    int fd = dup2(oldFd, newFd);
    if(fd == -1){
        err(3, "dup2 error");
    }
    return fd;
}

void wait_wrapper(void){
    int status;
    if(wait(&status) == -1){
        err(4, "wait error");
    }
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(5, "child exited with status %d", WEXITSTATUS(status));
        }
    }else{
        errx(6, "chid was killed");
    }
}

int main(void) {
    int findToSort[2];
    pipe_wrapper(findToSort);
    pid_t pid1 = fork_wrapper();

    if(pid1 == 0){
        close(findToSort[0]);
        dup2_wrapper(findToSort[1], 1);
        close(findToSort[1]);

        if(execlp("find", "find", "-type", "f", "-exec", "stat", "-c", "%Y", "{}", ";", (char*)NULL) == -1){
            err(7, "execlp error");
        }
    }

    close(findToSort[1]);

    int sortToHead[2];
    pipe_wrapper(sortToHead);
    pid_t pid2 = fork_wrapper();

    if(pid2 == 0){
        close(sortToHead[0]);
        dup2_wrapper(sortToHead[1], 1);
        dup2_wrapper(findToSort[0], 0);
        close(sortToHead[1]);
        close(findToSort[0]);

        if(execlp("sort", "sort", "-nr", (char*)NULL) == -1){
            err(8, "execlp error");
        }
    }

    close(findToSort[0]);
    close(sortToHead[1]);

    int pid3 = fork_wrapper();
    if(pid3 == 0){
        dup2_wrapper(sortToHead[0], 0);
        close(sortToHead[0]);
        if(execlp("head", "head", "-n1", (char*)NULL) == -1){
            err(9, "execlp error");
        }
    }
    close(sortToHead[0]);

    for(int i = 0; i < 3; i++){
        wait_wrapper();
    }
    return 0;
}
