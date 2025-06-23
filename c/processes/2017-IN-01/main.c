#include <unistd.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <sys/wait.h>
 
void pipe_wrapper(int fd[2]){
    if(pipe(fd) == -1){
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
        err(8, "wait error");
    }
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(9, "child exited with status %d", WEXITSTATUS(status));
        }
    }else{
        errx(10, "child was killed");
    }
}

int main(void) {
	int cutToSort[2];
	pipe_wrapper(cutToSort);
	pid_t pid1 = fork_wrapper();
	if(pid1 == 0){
        close(cutToSort[0]);
        dup2_wrapper(cutToSort[1], 1);
        close(cutToSort[1]);

        if(execlp("cut", "cut", "-d:", "-f7", "/etc/passwd", (char*)NULL) == -1){
            err(4, "exelp error");
        }
	}
	close(cutToSort[1]);

	int sortToUniq[2];
    pipe_wrapper(sortToUniq);
    pid_t pid2 = fork_wrapper();
    if(pid2 == 0){
        close(sortToUniq[0]);
        dup2_wrapper(cutToSort[0], 0);
        close(cutToSort[0]);
        dup2_wrapper(sortToUniq[1], 1);
        close(sortToUniq[1]);

        if(execlp("sort", "sort", (char*)NULL) == -1){
            err(5, "exelp error");
        }
    }
    
    close(sortToUniq[1]);
    close(cutToSort[0]);

    int uniqToSort[2];
    pipe_wrapper(uniqToSort);
    pid_t pid3 = fork_wrapper();
    
    if(pid3 == 0){
        close(uniqToSort[0]);
        dup2_wrapper(sortToUniq[0], 0);
        close(sortToUniq[0]);
        dup2_wrapper(uniqToSort[1], 1);
        close(uniqToSort[1]);

        if(execlp("uniq", "uniq", "-c", (char*)NULL) == -1){
            err(6, "exelp error");
        }
    }

    close(uniqToSort[1]);
    close(sortToUniq[0]);

    pid_t pid4 = fork_wrapper();
    if(pid4 == 0){
        dup2_wrapper(uniqToSort[0], 0);
        close(uniqToSort[0]);

        if(execlp("sort", "sort", "-n", (char*)NULL) == -1){
            err(7, "exelp error");
        }
    }

    close(uniqToSort[0]);

    for(int i = 0; i < 4; i++){
        wait_wrapper();
    }
}
