#include <fcntl.h>
#include <err.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(5, "Operation fstat failed!");
    }
    return st.st_size;
}

int main(int argc, char** argv) {
	if(argc != 4){
        errx(1, "Invalid arguments count!");
	}
	int scl;
	int sdl;
	int output;
	if((scl = open(argv[1], O_RDONLY)) == -1){
        err(2, "Could not open file %s", argv[1]);
	}
	if((sdl = open(argv[2], O_RDONLY)) == -1){
        err(3, "Could not open file %s", argv[2]);
	}
	if((output = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
        err(4, "Could not open file %s", argv[3]);
	}

	int scl_size = file_size(scl);
	int sdl_size = file_size(sdl);
	if(sdl_size % sizeof(uint16_t) != 0){
        errx(5, "Wrong input for sdl file!");
	}

	int count = sdl_size / sizeof(uint16_t);
	int needed = count / 8;
	if(count % 8 != 0){
        needed++;
	}

	if(scl_size * 8 < needed){
		err(5, "invalid scl");
	}

	uint8_t scl_byte;
	uint16_t sdl_data;
	ssize_t read_res;

    while((read_res = read(scl, &scl_byte, sizeof(scl_byte))) == sizeof(scl_byte)){
        for(uint8_t mask = 0x80; mask != 0; mask >>= 1){
            if((read_res = read(sdl, &sdl_data, sizeof(sdl_data))) != sizeof(sdl_data)){
                err(6, "Could not read from sdl file!");
            }
	    if(read_res == 0){
                errx(7, "There are no left nums in sdl!");
            }
            uint8_t bit = (scl_byte & mask);
            if(bit != 0){
                if(write(output, &sdl_data, sizeof(sdl_data)) != sizeof(sdl_data)){
                    err(8, "Could not write to output file!");
                }
            }
        }
    }
    if(read_res != 0){
        err(9, "Error while reading!");
    }

    close(scl);
    close(sdl);
    close(output);
}
