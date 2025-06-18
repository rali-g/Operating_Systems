#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef struct pair {
    uint32_t offset;
    uint32_t length;
} pair;

int file_size(int fd) {
    struct stat st;

    if(fstat(fd, &st) == -1) {
        err(5, "Something wrong with stat!");
    }
    return st.st_size;
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid number of args!");
	}

    int fd1;
    int fd2;
    int fd3;

    if((fd1 = open(argv[1], O_RDONLY)) == -1){
        err(2, "File %s could not open", argv[1]);
    }

    if((fd2 = open(argv[2], O_RDONLY)) == -1){
         err(3, "File %s could not open", argv[2]);
    }
    
    if((fd3 = open("f3", O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
        err(4, "File %s could not open", "f3");
    }
    
    uint32_t size1 = file_size(fd1);
    uint32_t size2 = file_size(fd2);
    if(size1 % 8 != 0 || size2 % 4 != 0){
        errx(5, "Invalid input for files");
    }

    pair p;
    ssize_t read_size;
    
    while((read_size = read(fd1, &p, sizeof(p))) == sizeof(p)){
        if((p.offset + p.length) * sizeof(uint32_t) > size2) {
            errx(6, "Invalid offset %d", p.offset);
        }

        if(lseek(fd2, p.offset * sizeof(uint32_t), SEEK_SET) == -1){
            err(7, "Could not lseek!");
        }
        uint32_t num;
        for(uint32_t i = 0; i < p.length; i++){
            int read_res = read(fd2, &num, sizeof(uint32_t));
            if(read_res == 0){
                err(8, "End of file!");
            }else if(read_res == -1){
                err(9, "Error while reading");
            }

            if(write(fd3, &num, sizeof(num)) != sizeof(num)){
                err(10, "Error while writing");
            }
        }
    }

    if(read_size != 0){
        err(11, "Read error");
    }
    
    close(fd1);
    close(fd2);
    close(fd3);
}
