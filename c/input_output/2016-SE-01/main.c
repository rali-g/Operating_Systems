#include "fcntl.h"
#include "err.h"
#include "unistd.h"
#include "stdint.h"

int main(int argc, char** argv) {
    if(argc != 2){
        errx(1, "Invalid number of args!");
    }
    char* file = argv[1];
    int fd;
    if((fd = open(file, O_RDWR)) < 0){
        err(2, "Cannot open file %s", file);
    }

    uint32_t counter[256] = {0};
    uint8_t curr;
    int bytes_read = 0;

    while((bytes_read = read(fd, &curr, sizeof(curr))) > 0){
        counter[curr] += 1;
    }

    if(bytes_read != 0){
        err(3, "Error while reading in %s", file);
    }

    if(lseek(fd, 0, SEEK_SET) < 0){
        err(4, "Cannot lseek file %s", file);
    }
    
    for(int i = 0; i <= 255; i++){
        uint8_t c = i;
        for(uint32_t j = 0; j < counter[i]; j++){
            if(write(fd, &c, sizeof(c)) < 0)
            {
                err(5, "Error while writing in %s", file);
            }
        }
    }

    close(fd);

    return 0;
}
