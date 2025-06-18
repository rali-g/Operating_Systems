#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "Error while executing fstat!");
    }
    return st.st_size;
}

typedef struct {
    uint16_t offset;
    uint8_t length;
    uint8_t notUsed;
} triple;

int main(int argc, char** argv) {
	if(argc != 5) {
        errx(1, "Invalid args count!");
	}

    int fd[4];
    if((fd[0] = open(argv[1], O_RDONLY)) == -1){
        err(2, "Error while opening %s", argv[1]);
    }
    if((fd[1] = open(argv[2], O_RDONLY)) == -1){
        err(3, "Error while opening %s", argv[2]);
    }
    if((fd[2] = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1) {
        err(4, "Error while opening %s", argv[3]);
    }
    if((fd[3] = open(argv[4], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1) {
        err(5, "Error while opening %s", argv[4]);
    }

    triple idx;

    if(file_size(fd[0]) % sizeof(uint8_t) != 0){
        errx(6, "Invalid dat file!");
    }

    if(file_size(fd[1]) % sizeof(idx) != 0){
        errx(7, "Invalid idx file!");    
    }

    int read_res;
    while((read_res = read(fd[1], &idx, sizeof(idx))) == sizeof(idx)){
        if(idx.offset + idx.length > file_size(fd[0])){
            errx(8, "Invalid offset!");
        }
        if(lseek(fd[0], idx.offset, SEEK_SET) == -1){
            err(9, "Operatoin lseek has failed!");
        }
        uint8_t str[256];
        int curr_res = read(fd[0], str, idx.length);
        if(curr_res == -1){
            err(10, "Operation read failed!");
        }else if(curr_res != idx.length){
            errx(11, "Unexpected end of file!");
        }
        if(str[0] >= 'A' && str[0] <= 'Z'){
            triple newIdx;
            newIdx.length = idx.length;
            
            int offset;
            if((offset = lseek(fd[2], 0, SEEK_CUR)) == -1){ 
                err(12, "Operation lseek failed!");
            }
            newIdx.offset = offset;
            newIdx.notUsed = 0;
            if(write(fd[2], str, idx.length) != idx.length){   
                err(12, "Operation write failed!");
            }

            if(write(fd[3], &newIdx, sizeof(newIdx)) != sizeof(newIdx)){    
                err(13, "Operation write failed!");
            }
        }
    }
    if(read_res != 0){
        err(14, "Operation read failed!");
    }

    close(fd[0]);
    close(fd[1]);
    close(fd[2]);
    close(fd[3]);
}
