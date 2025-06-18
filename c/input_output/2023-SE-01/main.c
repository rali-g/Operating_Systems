#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <err.h>
#include <stdio.h>

bool isMessageValid(int fd, uint8_t* n){
    uint8_t sum = 0x55;
    if(read(fd, n, sizeof(uint8_t)) == -1){
        return false;
    }
    sum ^= *n;
    
    uint8_t ch;
    for(int i = 3; i < *n; i++){
        if(read(fd, &ch, sizeof(ch)) != sizeof(ch)){
            return false;
        }
        sum ^= ch;
    }

    uint8_t checkSum;
    if(read(fd, &checkSum, sizeof(checkSum)) != sizeof(checkSum)){
        return false;
    }
    return (checkSum == sum);
}

void writeMessage(int fd1, int fd2, uint8_t n){
    uint8_t ch;
    for(uint8_t i = 0; i < n; i++){
        if(read(fd1, &ch, sizeof(ch)) != sizeof(ch)){
            err(12, "Could not read ch!");
        }
        if(write(fd2, &ch, sizeof(ch)) != sizeof(ch)){
            err(13, "Could not write ch!");
        }
    }
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid arguments count!");
	}
	int fd1;
	int fd2;
	if((fd1 = open(argv[1], O_RDONLY)) == -1){
        err(2, "Could not open file %s", argv[1]);
	}
	if((fd2 = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
        err(3, "Could not open file %s", argv[2]);
	}
    
    ssize_t res;
    uint8_t ch;
    while((res = read(fd1, &ch, sizeof(ch))) == sizeof(ch)){
        if(ch == 0x55){
            uint8_t n;
            int currPosition = lseek(fd1, 0, SEEK_CUR);
            if(currPosition == -1){
                err(4, "Could not lseek!");
            }

            if(isMessageValid(fd1, &n)){
                if(lseek(fd1, currPosition -1, SEEK_SET) == -1){
                    err(5, "Could not lseek!");
                }
                writeMessage(fd1, fd2, n);
            }

            if(lseek(fd1, currPosition, SEEK_SET) == -1){
                err(6, "Could not lseek!");
            }
        }
    }
    if(res != 0){
        err(7, "Could not read!");
    }

    close(fd1);
    close(fd2);
}
