#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "Operation fstat failed!");
    }
    return st.st_size;
}

typedef struct{
    uint32_t magic;
    uint8_t header_version;
    uint8_t data_version;
    uint16_t count;
    uint32_t reserved1;
    uint32_t reserved2;
}header;

typedef struct{
    uint16_t offset;
    uint8_t original;
    uint8_t new;
} firstVersion;

typedef struct{
    uint32_t offset;
    uint16_t original;
    uint16_t new;
} secondVersion;

void first(int fd2, int patch, uint16_t count){
    firstVersion curr;
    for(int i = 0; i < count; i++){
        if(read(patch, &curr, sizeof(curr)) != sizeof(curr)){
            err(11, "Could not read record from patch!");
        }
        if(lseek(fd2, curr.offset * sizeof(uint8_t), SEEK_SET) == -1){
            err(12, "Could not lseek!");
        }
        uint8_t ch;
        if(read(fd2, &ch, sizeof(ch)) != sizeof(ch)){
            err(13, "Could not read!");
        }
        if(ch == curr.original){
            if(lseek(fd2, -1, SEEK_CUR) == -1){ 
                err(14, "Could not lseek!");
            }
            if(write(fd2, &curr.new, sizeof(curr.new)) != sizeof(curr.new)){
                err(15, "Could not write!");
            }
        }else{
            err(16, "Byte is not the original one!");
        }
    }
}

void second(int fd2, int patch, uint16_t count){
    secondVersion curr;
    for (int i = 0; i < count; i++) {
        if (read(patch, &curr, sizeof(curr)) != sizeof(curr)) {
            err(11, "Could not read record from patch!");
        }
        if (lseek(fd2, curr.offset * sizeof(uint16_t), SEEK_SET) == -1) {
            err(12, "Could not lseek!");
        }
        uint16_t ch;
        if (read(fd2, &ch, sizeof(ch)) != sizeof(ch)) {
            err(13, "Could not read!");
        }
        if (ch == curr.original) {
            if (lseek(fd2, -2, SEEK_CUR) == -1) {
                err(14, "Could not lseek!");
            }
            if (write(fd2, &curr.new, sizeof(curr.new)) != sizeof(curr.new)) {
                err(15, "Could not write!");
            }
        } else {
            err(16, "Byte is not the original one!");
        }
    }
}

int main(int argc, char** argv) {
	if(argc != 4){
        errx(1, "Invalid arguments count!");
	}
	int patch;
	int fd1;
	int fd2;
	if((patch = open(argv[1], O_RDONLY)) == -1){
        err(3, "Could not open file %s", argv[1]);
	}
	if((fd1 = open(argv[2], O_RDONLY)) == -1){
        err(4, "Could not open file %s", argv[2]);
	}
	if((fd2 = open(argv[3], O_RDWR | O_TRUNC | O_CREAT, 0666)) == -1){
        err(5, "Could not open file %s", argv[3]);
	}
    
    header h;
    if(read(patch, &h, sizeof(header)) != sizeof(header)){
        err(6, "Could not read header!");
    }
    if(h.magic != 0xEFBEADDE || h.header_version != 0x01){
        errx(7, "Invalid header input!");
    }

    int patchSize = file_size(patch);
    int fd1Size = file_size(fd1);
    if(h.data_version == 0x00){
        if((patchSize - sizeof(h)) % sizeof(firstVersion) != 0){
            errx(8, "Invalid input for first version!");
        }
        if(fd1Size % sizeof(uint8_t) != 0){
            errx(8, "Invalid input for %s", argv[2]);
        }
    }else if(h.data_version == 0x01){
        if((patchSize - sizeof(h)) % sizeof(secondVersion) != 0){
            errx(9, "Invalid input for second version!");
        }
        if(fd1Size % sizeof(uint16_t) != 0){
            errx(9, "Invalid input for %s", argv[2]);
        }
    }else{
        errx(10, "Invalid input for data version!");
    }

    int read_res;
    char ch;
    while((read_res = read(fd1, &ch, sizeof(ch))) == sizeof(ch)){
        if(write(fd2, &ch, sizeof(ch)) != sizeof(ch)){
            err(11, "Could not write to %s", argv[3]);
        }
    }
    if(read_res != 0){
        err(12, "Could not read %s", argv[2]);
    }
    
    int records = h.count;
    if(h.data_version == 0x00){
        first(fd2, patch, records);
    }else {
        second(fd2, patch, records);
    }

	close(patch);
	close(fd1);
	close(fd2);
}
