#include <err.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

void run(const char* cmd, char* args[], int count){
    pid_t pid = fork();
    if(pid == -1){
        err(6, "fork error");
    }
    if(pid == 0){
        if(count == 1){
            if(execlp(cmd, cmd, args[0], (char*)NULL) == -1){
                err(7, "execlp error");
            }
        }else if(count == 2){
            if(execlp(cmd, cmd, args[0], args[1], (char*)NULL) == -1){
                err(8, "execlp error");
            }
        }
    }
    int status;
    if(wait(&status) == -1){
        err(9, "wait failed");
    }
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) != 0){
            errx(10, "child exited with status %d", WEXITSTATUS(status));
        }
    }else{
        errx(11, "child was killed");
    }
}

int main(int argc, char** argv) {
	if(argc > 2){
        errx(1, "Too much args");
	}
    
    const char* cmd = "echo";
	if(argc == 2){
        if(strlen(argv[1]) > 4){
            errx(2, "commands length is too long");
        }
        cmd = argv[1];
	}

	char buff[1024];
	int bytes = read(0, buff, sizeof(buff - 1));
	if(bytes == -1){
        err(3, "read buff error");
	}
	buff[bytes] = '\0';

	char* args[2];
	int count = 0;

	char* token = strtok(buff, " \n");
    while(token != NULL){
        if(strlen(token) > 4){
            errx(4, "string length is too long");
        }
        args[count++] = token;
        if(count == 2){
            run(cmd, args, count);
            count = 0;
        }
        token = strtok(NULL, " \n");
    }
    if(count > 0){
        run(cmd, args, count);
    }
}
