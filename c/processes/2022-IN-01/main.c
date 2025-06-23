#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <err.h>
#include <stdlib.h>

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(1, "arg should be a number");
    }
    return num;
}

void handleParent(int n, int d, int fdFromChild, int fdToChild){
    for(int i = 0; i < n; i++){
        if(write(1, "Ding\n", 5) == -1){
            err(23, "write failed");
        }

        int dummy1;
        if(write(fdToChild, &dummy1, sizeof(dummy1)) == -1){
            err(24, "write failed");
        }

        int dummy2;
        if(read(fdFromChild, &dummy2, sizeof(dummy2)) == -1){
            err(25, "read failed");
        }

        sleep(d);
    }
}

void handleChild(int fdFromParent, int fdToParent){
    int res;
    int dummy1;
    while((res = read(fdFromParent, &dummy1, sizeof(dummy1))) == sizeof(dummy1)){
        if(write(1, "Dong\n", 5) == -1){
            err(20, "write failed");
        }

        int dummy2;
        if(write(fdToParent, &dummy2, sizeof(dummy2)) == -1){
            err(21, "write failed");
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
    int n = convertToInt(argv[1]);
    if(n < 0 || n > 9){
        errx(2, "num should be positive and one-digit");
    }
    int d = convertToInt(argv[2]);
    if(d < 0 || d > 9){
        errx(3, "num should be positive and one-digit");
    }

    int parentToChild[2];
    int childToParent[2];

    if(pipe(parentToChild) == -1){
        err(4, "pipe failed");
    }
    if(pipe(childToParent) == -1){
        err(5, "pipe failed");
    }

    pid_t pid = fork();
    if(pid == -1){
        err(6, "fork error");
    }
    if(pid == 0){
        close(childToParent[0]);
        close(parentToChild[1]);

        handleChild(parentToChild[0], childToParent[1]);

        close(childToParent[1]);
        close(parentToChild[0]);
        exit(0);
    }
    close(parentToChild[0]);
    close(childToParent[1]);

    handleParent(n, d, childToParent[0], parentToChild[1]);

    close(childToParent[0]);
    close(parentToChild[1]);

    int status;
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

    return 0;
}
