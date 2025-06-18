#include <err.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

bool histogram[256];
char destination[256];

void mark(const char* set){
    int length = strlen(set);
    for(int i = 0; i < length; i++){
        histogram[(int)set[i]] = 1;
    }
}

void delete(const char* input, const char* set1, int len){
    mark(set1);
    for(int i = 0; i < len; i++){
        if(histogram[(int)input[i]] == 0){
            char ch = input[i];
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)){
                err(3, "Could not write to stdout!");
            }
        }
    }
}

void squeeze(const char* input, const char* set1, int len){
    mark(set1);

    char prev = input[0];
    if(write(1, &prev, sizeof(prev)) != sizeof(prev)){
        err(3, "Could not write to stdout!");
    }
    for(int i = 1; i < len; i++){
        if((histogram[(int)input[i]] == 1) && (prev == input[i])){
            continue;
        }
        prev = input[i];
        if(write(1, &prev, sizeof(prev)) != sizeof(prev)){
            err(3, "Could not write to stdout!");
        }
    }
}

void map(const char* set1, const char* set2){
    int length = strlen(set1);
    for(int i = 0; i < length; i++){
        destination[(int)set1[i]] = set2[i];
    }
}

void substitute(const char* input, const char* set1, const char* set2, int len){
    mark(set1);
    map(set1, set2);
    for(int i = 0; i < len; i++){
        if(histogram[(int)input[i]]){
            char ch = destination[(int)input[i]];
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)){
                err(3, "Could not write to stdout!");
            }        
        }else{
            char ch = input[i];
            if(write(1, &ch, sizeof(ch)) != sizeof(ch)){
                 err(3, "Could not write to stdout!");
             }

        }
    }
}

int main(int argc, char** argv) {
    if(argc != 3){
        err(1, "Invalid args count!");
    }

    char input[4096];
    int len;
    if((len = read(0, &input, sizeof(input))) == -1){
        err(2, "Error while reading from stdin!");
    }
    
    char set1[4096];
    char set2[4096];
    if(strcmp(argv[1], "-d") == 0){
        strcpy(set1, argv[2]);
        delete(input, set1, len);
    }else if(strcmp(argv[1], "-s") == 0){
        strcpy(set1, argv[2]);
        squeeze(input, set1, len);
    }else{
        strcpy(set1, argv[1]);
        strcpy(set2, argv[2]);

        if(strlen(set1) != strlen(set2)){
            errx(5, "Args should be equal length!");
        }
        substitute(input, set1, set2, len);
    }

    exit(0);
}
