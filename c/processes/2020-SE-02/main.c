#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <err.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
        if(argc!=3){
            errx(1, "Wrong arg count!");
        }

        int in = open(argv[1], O_RDONLY);

        int out = open(argv[2], O_CREAT | O_TRUNC | O_WRONLY, 0666);

        if (in == -1){
                err(2, "Could not open input file!");
        }

        if (out == -1){
                err(3, "Could not create output file!");
        }
        int fd[2];
        if (pipe(fd) == -1){
                err(4, "Could not create a pipe!");
        }

        pid_t pid = fork();
        if (pid == -1){
                err(5, "Could not fork!");
        }

        if (pid==0)
        {
            close(fd[0]);
            if (dup2(fd[1], 1) == -1){
                err(6, "Could not write to pipe!");
            }
            close(fd[1]);
            execlp("cat", "cat", argv[1], (char*)NULL);
            err(7, "Could not exec!");
        }
               
        close(fd[1]);
        int status;
        wait(&status);

        if (WIFEXITED(status)){
            if (WEXITSTATUS(status)!=0){
                     errx(8, "Child exited with status %d", WEXITSTATUS(status));
                }
        }else{
            errx(9, "Child has been killed!");
        }

        uint8_t byte;
        int readSize;

        while ((readSize = read(fd[0], &byte, sizeof(uint8_t)))>0){
            if (byte==0x7D){
                read(fd[0], &byte, sizeof(uint8_t));
                uint8_t newByte = byte^0x20;
                write(out, &newByte, sizeof(uint8_t));
            }else if (byte==0x55){
                continue;
            }else{
                write(out, &byte, sizeof(uint8_t));
            }
        }

        if (readSize != 0){
            err(10, "Error while reading data from stream!");
        }

        close(fd[0]);
        close(in);
        close(out);

        return 0;
}
