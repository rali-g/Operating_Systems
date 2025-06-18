#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <err.h>

typedef struct{
    uint32_t magic;
    uint32_t count;
    uint64_t size;
} header;

ssize_t read_wrapper(int fd, void* buff, ssize_t bytesCount){
    ssize_t res = read(fd, buff, bytesCount);
    if(res == -1){
        err(4, "read failed");
    }
    return res;
}

ssize_t write_wrapper(int fd, void* buff, ssize_t bytesCount){
    ssize_t res = write(fd, buff, bytesCount);
    if(res == -1){
        err(8, "write failed");
    }
    if(res != bytesCount){
        err(8, "data not written");
    }
    return res;
}

off_t lseek_wrapper(int fd, off_t offset, int whence){
    off_t o = lseek(fd, offset, whence);
    if(o == -1){
        err(6, "lseek failed");
    }
    return o;
}

void type1(int cmp, int res, int n){
    uint8_t c;
    for(int i = 0; i <= n; i++){
        if(read_wrapper(cmp, &c, sizeof(c)) != sizeof(c)){
            err(7, "unexpected eof");
        }
        write_wrapper(res, &c, sizeof(c));
    }
}

void type2(int cmp, int res, int n){
    uint8_t c;
    if(read_wrapper(cmp, &c, sizeof(c)) != sizeof(c)){
        err(9, "unexpected eof");
    }
    for(int i = 0; i <= n; i++){
        write_wrapper(res, &c, sizeof(c));
    }
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid args count!");
	}
	int cmp = open(argv[1], O_RDONLY);
	if(cmp == -1){
        err(2, "%s not opened", argv[1]);
	}
	int res = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if(res == -1){
        err(3, "%s not opened", argv[2]);
	}

	header h;
	if(read_wrapper(cmp, &h, sizeof(h)) != sizeof(h)){
        err(4, "unexpected eof");
	}
	if(h.magic != 0x21494D46){
        errx(5, "invalid magic");
	}

	uint8_t byte;
	while(read_wrapper(cmp, &byte, sizeof(byte)) == sizeof(byte)){
	    uint8_t mask = 0x80;
	    uint8_t type = (mask & byte) >> 7;
	    uint8_t N = byte & (mask - 1);
	    if(type == 1){
            type2(cmp, res, N);
	    }else{
            type1(cmp, res, N);
	    }
	}

    close(cmp);
    close(res);
}
