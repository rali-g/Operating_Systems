#include <unistd.h>
#include <err.h>

int main(int argc, char** argv) {
	if(argc != 2){
        errx(1, "invalid argc count");
	}

	int catToSort[2];
	if(pipe(catToSort) == -1){
        err(2, "pipe error");
	}

    pid_t pid = fork();
    if(pid == -1){
        err(3, "fork error");
    }

    if(pid == 0){
        close(catToSort[0]);
        if(dup2(catToSort[1], 1) == -1){
            err(4, "dup2 error");
        }
        close(catToSort[1]);
        if(execlp("cat", "cat", argv[1], (char*)NULL) == -1){
            err(5, "exelp error");
        }
    }

    close(catToSort[1]);
    if(dup2(catToSort[0], 0) == -1){
        err(6, "dup2 error");
    }
    close(catToSort[0]);
    if(execlp("sort", "sort", (char*)NULL) ==  -1){
        err(7, "exelp error");
    }
}
