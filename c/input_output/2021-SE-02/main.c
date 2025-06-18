#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <err.h>

int file_size(int fd) {
    struct stat st;

    if(fstat(fd, &st) == -1) {
        err(1, "fstat");
    }

    return st.st_size;
}

uint8_t decode(uint8_t byte){
    uint8_t result = 0;
    for(int i = 0; i < 4; i++){
        int code = byte & 3;
        byte >>= 2;

        if(code != 1 && code != 2){
            errx(7,"Invalid input file!");
        }
        if(code != 1){
            result |= (1 << i);
        }
    }
    return result;
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid arguments count!");
	}

	int input = open(argv[1], O_RDONLY);
    if(input == -1) {
        err(2, "%s", argv[1]);
    }

    if(file_size(input) % 2 != 0) {
        errx(3, "Invalid input file size");
    }

    int output = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0600);
    if(output == -1) {
        err(4, "open %s", argv[2]);
    }

    uint8_t encodedByte[2];
    ssize_t res;

    while((res = read(input, &encodedByte, sizeof(encodedByte)) == sizeof(encodedByte))){
        uint8_t result = (decode(encodedByte[0]) << 4) | decode(encodedByte[1]);
        if(write(output, &result, sizeof(result)) != sizeof(result)){
            err(5, "Could not write to output!");
        }
    }
    if(res != 0){
        err(6, "Could not read properly!");
    }
    
    close(input);
    close(output);
}
