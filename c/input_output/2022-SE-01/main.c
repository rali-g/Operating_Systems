#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <err.h>
#include <sys/stat.h>

typedef struct{
    uint32_t magic;
    uint32_t count;
} dataHeader;

typedef struct __attribute__((packed)){
    uint32_t magic1;
    uint16_t magic2;
    uint16_t reserved;
    uint64_t count;
} cmpHeader;

typedef struct{
    uint16_t type;
    uint16_t reserved[3];
    uint32_t offset1;
    uint32_t offset2;
} cmpData;

int file_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) < 0)
        err(1, "Operation fstat failed!");

    return st.st_size;
}

void swap(int data, uint32_t offset1, uint32_t offset2, uint64_t byte1, uint64_t byte2){
        if (lseek(data, offset1 * sizeof(uint64_t) + sizeof(dataHeader), SEEK_SET) < 0)
                err(14, "Cant seek");
        if (write(data, &byte2, sizeof(byte2)) < 0)
                err(15, "Cant write");

        if (lseek(data, offset2 * sizeof(uint64_t) + sizeof(dataHeader), SEEK_SET) < 0)
                err(16, "Cant seek");
        if (write(data, &byte1, sizeof(byte1)) < 0)
                err(17, "Cant write");
}

int main(int argc, char** argv) {
    if(argc != 3){
        errx(1, "Invalid arguments count!");
    }
    int data;
    int cmp;
    if((data = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR)) == -1){
        err(2, "Could not open file %s", argv[1]);
    }
    if((cmp = open(argv[2], O_RDONLY)) == -1){
        err(3, "Could not open file %s", argv[2]);
    }

    dataHeader dh;
    if(read(data, &dh, sizeof(dh)) != sizeof(dh)){
        err(4, "Could not read header from data file!");
    }
    int dataSize = file_size(data);
    if(dh.magic != 0x21796F4A || (size_t)dataSize != dh.count * sizeof(uint64_t) + sizeof(dh)){
        err(5, "Invalid data file input!");
    }

    cmpHeader ch;
    cmpData cd;
    if(read(cmp, &ch, sizeof(ch)) != sizeof(ch)){
        err(6, "Could not read header from cmp file!");
    }
    int cmpSize = file_size(cmp);
    if(ch.magic1 != 0xAFBC7A37 || ch.magic2 != 0x1C27 || (size_t)cmpSize != sizeof(ch) + sizeof(cd) * ch.count){
        errx(7, "Invalid cmp file input!");
    }      

    for(uint64_t i = 0; i < ch.count; i++){
        cmpData c;
        if(read(cmp, &c, sizeof(c)) != sizeof(c)){
            err(8, "Could not read data from cmp!");
        }
        
        uint64_t byte1;
        uint64_t byte2;
        if(lseek(data, c.offset1 * sizeof(byte1) + sizeof(dh), SEEK_SET) == -1){
            err(9, "Could not lseek in data file!");
        }
        if(read(data, &byte1, sizeof(byte1)) != sizeof(byte1)){
            err(10, "Could not read byte1 from data file!");
        }
        if(lseek(data, c.offset2 * sizeof(byte2) + sizeof(dh), SEEK_SET) == -1){
            err(11, "Could not lseek in data file!");
        } 
        if(read(data, &byte2, sizeof(byte2)) != sizeof(byte2)){
            err(13, "Could not read byte2 from data file!");
        }
        
        if(c.type == 0){
            if(byte1 > byte2)
                swap(data, c.offset1, c.offset2, byte1, byte2);
        }else if(c.type == 1){
            if(byte1 < byte2)
                swap(data, c.offset1, c.offset2, byte1, byte2);
        }

    }

    close(data);
    close(cmp);
}

