#include <err.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

int convertToInt(char* str){
    char* endPtr;
    int num = strtol(str, &endPtr, 10);
    if(str[0] == '\0' || *endPtr != '\0'){
        errx(2, "Argument should be a number!");
    }
    return num;
}

void cOption(int from, int to){
    int read_res;
    int current = 1;
    uint8_t ch;
    while((read_res = read(0, &ch, sizeof(ch))) == sizeof(ch)){
        if(ch == '\n'){
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)){
                err(8, "Could not write to stdout!");
            }
            current = 1;
            continue;
        }
        if(current >= from && current <= to){
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)){
                err(9, "Could not write to stdout!");
            }
        }
        current++;
    }

    if(read_res != 0){
        err(10, "Error while reading!");
    }
}

void dOption(char delimiter, int from, int to){
    int current = 1;
    int read_res;
    uint8_t ch;
    while((read_res = read(0, &ch, sizeof(ch))) == sizeof(ch)){
        if(ch == '\n') {
            current = 1;
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)) {
                err(11, "Could not write to stdout!");
            }
            continue;
        }
        if(current >= from && current < to){
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)) {
                err(12, "Could not write to stdout!");
            }
        }else if(current == to && ch != delimiter){
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)) {
                err(13, "Could not write to stdout!");
            }
        }
        if(ch == delimiter){
            current++;
        }
    }
    if(read_res != 0){
        err(14, "Could not read!");
    }
}

int main(int argc, char* argv[]) {
    if(argc != 3 && argc != 5) {
        errx(1, "Wrong args count");
    }

    if(strcmp(argv[1], "-c") == 0) {
        if(argc != 3) {
            errx(2, "Wrong args count");
        }
        if(strlen(argv[2]) == 1) {
            int num = convertToInt(argv[2]);
            cOption(num, num);
        } else if(strlen(argv[2]) == 3 && argv[2][1] == '-') {
            char* fromPtr = argv[2];
            fromPtr[1] = '\0';

            int from = convertToInt(fromPtr);
            int to = convertToInt(&argv[2][2]);
            if(from > to){
                errx(3, "Invalid arguments!");
            }
            cOption(from, to);
        } else {
            errx(4, "Invalid argument");
        }
    } else if(strcmp(argv[1], "-d") == 0 && argc == 5 && strcmp(argv[3], "-f") == 0) {
        if(strlen(argv[4]) == 1) {
            int fields = convertToInt(argv[4]);
            dOption(argv[2][0], fields, fields);
       } else if(strlen(argv[4]) == 3) {
            char* fromPtr = argv[4];
            fromPtr[1] = '\0';

            int from = convertToInt(fromPtr);
            int to = convertToInt(&argv[4][2]);
            if(from > to){
                errx(5, "Invalid arguments!");
            }
            dOption(argv[2][0], from, to);
        } else {
            errx(6, "Invalid arguments");
        }
    } else {
        errx(7, "Invalid arguments");
    }
}
