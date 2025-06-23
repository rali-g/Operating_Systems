#include <err.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

int main(void) {
    char buff[1024];

	int res;
	while(true){ 
	    const char* cmd = ">";
	    if(write(1, cmd, strlen(cmd)) != (ssize_t)strlen(cmd)){
            err(1, "write error");
	    }
        res = read(0, buff, sizeof(buff));
        if(res == -1){
            err(2, "read error");
        }
        buff[res - 1] = '\0';
        
        if(strcmp(buff, "exit") == 0){
            exit(0);
        }
        
        pid_t pid = fork();
        if(pid == -1){
            err(3, "fork error");
        }
        if(pid == 0){
            char command[4096];
            strcpy(command, "/bin/");
            strcat(command, buff);
            if(execl(command, buff, (char*)NULL) == -1){
                err(4, "execl error");
            }
        }
        
        int status;
        if(wait(&status) == -1){
            err(5, "wait error");
        }
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status) != 0){
                errx(6, "child exited with %d", WEXITSTATUS(status));
            }
        }else{
            errx(7, "child was killed :(");
        }
	}
	if(res == -1){
        err(8, "read error");
	}
}
