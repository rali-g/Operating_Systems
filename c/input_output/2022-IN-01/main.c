#include <err.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
typedef struct {
    uint16_t magic;
    uint16_t filetype;
    uint32_t count;
} header;

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(8, "Could not fstat!");
    }
    return st.st_size;
}

int main(int argc, char** argv) {
	if(argc != 4){
        errx(1, "Invalid argumets count!\nUsage: list.bin data.bin out.bin");
	}
	int list;
	int data;
	int out;
	if((list = open(argv[1], O_RDONLY)) == -1){
        err(2, "Could not open %s!", argv[1]);
	}
	if((data = open(argv[2], O_RDONLY)) == -1){
        err(3, "Could not open %s!", argv[2]);
	}
	if((out = open(argv[3], O_RDWR | O_TRUNC | O_CREAT, 0666)) == -1){
        err(4, "Could not open %s!", argv[3]);
	}

	header h1;
	if(read(list, &h1, sizeof(h1)) != sizeof(h1)){
        err(5, "Could not read header from file %s", argv[1]);
	}
	if(h1.magic != 0x5A4D){
        errx(6, "Invalid magic for file %s", argv[1]);
	}
	if(h1.filetype != 1){
        errx(7, "Invalid filetype for file %s", argv[1]);
	}
    int listSize = file_size(list);
    if((size_t)listSize != (sizeof(h1) + h1.count * (sizeof(uint16_t)))){
        errx(9, "Invalid list file input!");
    }

    header h2;
	if(read(data, &h2, sizeof(h2)) != sizeof(h2)){
        err(5, "Could not read header from file %s", argv[2]);
	}
	if(h2.magic != 0x5A4D){
        errx(6, "Invalid magic for file %s", argv[2]);
	}
	if(h2.filetype != 2){
        errx(7, "Invalid filetype for file %s", argv[2]);
	}
    int dataSize = file_size(data);
    if((size_t)dataSize != (sizeof(h2) + h2.count * (sizeof(uint32_t)))){
        errx(9, "Invalid data file input!");
    }
    
    uint16_t maxValue = 0;
    uint16_t c;
    lseek(list, sizeof(header), SEEK_SET);
    for(size_t i = 0; i < h1.count; i++){
        if(read(list, &c, sizeof(c)) != sizeof(c)){
            err(10, "Could not read1!");
        }
        if(c > maxValue){
            maxValue = c;
        }
    }

	header h3;
	h3.magic = 0x5A4D;
	h3.filetype = 3;
	h3.count = maxValue + 1;
    if(write(out, &h3, sizeof(h3)) != sizeof(h3)){
        err(11, "Could not write!");
    }

    //unnecessary, when i write at a curtain position the empty ones before that will be initialized automatically	
    uint64_t dummy = 0;
    for(size_t i = 0; i < maxValue; i++){
        if(write(out, &dummy, sizeof(dummy)) != sizeof(dummy)){
            err(12, "Could not write dummy!");
        }
    }
    
    uint16_t position;
    uint32_t value;
    uint64_t toWrite = 0;
    for(uint32_t i = 0; i < h1.count; i++){
        if(lseek(list, sizeof(header) + i * sizeof(uint16_t), SEEK_SET) == -1){
            err(13, "Could not lseek!");
        }
        if(read(list, &position, sizeof(position)) != sizeof(position)){
            err(14, "Could not read2!");
        }

        if(lseek(data, sizeof(header) + i * sizeof(uint32_t), SEEK_SET) == -1){
            err(15, "Could not lseek!");
        }
        if(read(data, &value, sizeof(value)) != sizeof(value)){
            err(16, "Could not read3 %d!", i);
        }

        toWrite = value;
        if(lseek(out, sizeof(header) + position * sizeof(uint64_t), SEEK_SET) == -1){
            err(17, "Could not lseek!");
        }
        if(write(out, &toWrite, sizeof(toWrite)) != sizeof(toWrite)){
            err(18, "Could not write!");
        }
    }

    close(list);
    close(data);
    close(out);
}
