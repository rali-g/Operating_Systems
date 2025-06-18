#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>

int file_size(int fd){  
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(5, "Operation fstat failed!");
    }
    return st.st_size;
}

typedef struct {
    uint16_t offset;
    uint8_t original;
    uint8_t new;
} triple;

int main(int argc, char** argv) {
	if(argc != 4){
        errx(1, "Invalid args count");
	}
    
    int fd[3];
    if((fd[0] = open(argv[1], O_RDONLY)) == -1){
        err(2, "Operation open failed!");
    }
    if((fd[1] = open(argv[2], O_RDONLY)) == -1){ 
        err(3, "Operation open failed!");
    }
    if((fd[2] = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){    
        err(4, "Operation open failed!");
    }

    int size1 = file_size(fd[0]);
    int size2 = file_size(fd[1]);
    if(size1 != size2 || size1 % sizeof(uint8_t) != 0){
        errx(6, "Invalid input for files");
    }

    uint8_t ch1;
    uint8_t ch2;
    ssize_t res;

    while((res = read(fd[0], &ch1, sizeof(ch1))) == sizeof(ch1)){
        if(read(fd[1], &ch2, sizeof(ch2)) != sizeof(ch2)){
            err(7, "Operation read failed!");
        }
        if(ch1 != ch2){
            triple curr;

            int offset;
            if((offset = lseek(fd[0], 0, SEEK_CUR)) == -1){
                err(8, "Operation lseek failed!");
            }
            curr.offset = offset - 1;
            curr.original = ch1;
            curr.new = ch2;

            if(write(fd[2], &curr, sizeof(curr)) != sizeof(curr)){
                err(9, "Operation write failed!");
            }
        }
    }

    if(res != 0){
        err(10, "Operation read failed!");
    }

    close(fd[0]);
    close(fd[1]);
    close(fd[2]);
}
