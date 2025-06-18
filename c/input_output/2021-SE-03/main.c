#include <err.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "Operation fstat failed!");
    }
    return st.st_size;
}

int main(int argc, char** argv) {
	if(argc != 3){
        errx(1, "Invalid arguments count!");
	}
    
    int in;
    int out;
    if((in = open(argv[1], O_RDONLY)) == -1){
        err(3, "Could not open %s", argv[1]);
    }
    if((out = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1){
        err(4, "Could not open %s", argv[2]);
    }

    int size = file_size(in);
    if(size % sizeof(uint16_t) != 0){
        errx(5, "Invalid input file!");
    }
    int n;
    char buff[1024];
    n = snprintf(buff, sizeof(buff), "#pragma once\n#include<stdint.h>\n\nuint32_t arrN = %d;\nconst uint16_t arr[] = {" , size / 2);
    if(n < 0){
        err(1, "Operation snprintf has failed!");
    }else if((size_t)n >= sizeof(buff)){
        errx(2, "Buffer is too small!");
    }
    if(write(out, buff, strlen(buff)) != (ssize_t)strlen(buff)){
        err(6, "Could not write to output file!");
    }
    uint16_t ch;
    for(int i = 0; i < size / 2 - 1; i++){
        if(read(in, &ch, sizeof(ch)) != sizeof(ch)){
            err(7, "Could not read from input file!");
        }
        char num[32];
        n = snprintf(num, sizeof(num), "%d,", ch);
        if(n < 0){
            err(8, "snprintf failed!");
        }
        if(write(out, num, strlen(num)) != (ssize_t)strlen(num)){
            err(9, "Write has failed!");
        }
    }

    if(read(in, &ch, sizeof(ch)) != sizeof(ch)){
        err(7, "Could not read from input file!");
    }
    
    char num[32];
    n = snprintf(num, sizeof(num), "%d};\n", ch);
    if(n < 0){
        err(8, "snprintf failed!");
    }
    if(write(out, num, strlen(num)) != (ssize_t)strlen(num)){
        err(9, "Write has failed!");
    }

    close(in);
    close(out);
}
