#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <err.h>
#include <stdio.h>

uint8_t encode(uint8_t num){
    uint8_t result = 0;
    uint8_t mask = 1 << 7;
    
    for(int i = 0; i < 4; i++){
        if((num & mask) == 0){
            result |= 1 << (6 - 2 * i);
        }else{
            result |= 1 << (7 - 2 * i);
        }
        mask >>= 1;
    }
    return result;
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid arguments count!");
	}
	int input;
	int output;
	if((input = open(argv[1], O_RDONLY)) == -1){
        err(2, "Could not open file %s", argv[1]);
	}
	if((output = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
        err(3, "Could not open file %s", argv[2]);
	}

	ssize_t res;
	uint8_t c;

	while((res = read(input, &c, sizeof(c))) == sizeof(c)){
        uint8_t result1 = encode(c);
        uint8_t result2 = encode(c << 4);
        if(write(output, &result1, sizeof(result1)) != sizeof(result1)) {
            err(4, "Could not write result1");
        }
        if(write(output, &result2, sizeof(result2)) != sizeof(result2)) {
            err(6, "Could not write result2");
        }
	}   
	if(res != 0){
        err(6, "Error while reading!");
	}
	
	close(input);
	close(output);
}
