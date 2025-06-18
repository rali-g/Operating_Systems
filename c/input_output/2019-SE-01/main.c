#include <stdint.h>
#include <unistd.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

typedef struct{
    uint32_t id;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t start;
    uint32_t end;
}user_data;

typedef struct{
    uint32_t uid;
    uint32_t max_duration;
}user_stats;

double avg(user_data* arr, int length){
    int sum = 0;
    for(int i = 0; i < length; i++){
        sum += (arr[i].end - arr[i].start);
    }
    return sum / length;
}

double dispersion(user_data* arr, int length){
    double average = avg(arr, length);
    double sum = 0;
    for(int i = 0; i < length; i++){
        int duration = arr[i].end - arr[i].start;
        double diff = duration - average;
        sum += diff * diff;
    }
    return sum /= length;
}

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(3, "Error while executing fstat!");   
    }
    return st.st_size;
}

int main(int argc, char** argv) {
	if(argc != 2){
        errx(1, "Invalid arguments count!");
	}

	int fd;
	if((fd = open(argv[1], O_RDONLY)) == -1){
        err(2, "Could not open file %s", argv[1]);
	}
	int size = file_size(fd);
	if(size % sizeof(user_data) != 0 || (size_t)size >= 16384 * sizeof(user_data)){
        errx(4, "Invalid file input!");
	}
	int records = size / sizeof(user_data);
    user_data* arr = malloc(records * sizeof(user_data));
    if(!arr){
        err(5, "Could not allocate memory!");
    }

    ssize_t read_res;
    for(int i = 0; i < records; i++){
       if((read_res = read(fd, &arr[i], sizeof(user_data))) != sizeof(user_data)){
            err(5, "Could not read!");
       }
    }

    double disp = dispersion(arr, records);
    user_stats stats[2048];
    int stats_count = 0;
    uint32_t uid;
    uint32_t duration;
    for(int i = 0; i < records; i++){
        uid = arr[i].id;
        duration = arr[i].end - arr[i].start;
        bool found = 0;
        for (int j = 0; j < stats_count; j++) {
            if (stats[j].uid == uid) {
                if (duration > stats[j].max_duration) {
                    stats[j].max_duration = duration;
                }
                found = 1;
                break;
            }
        }   

        if (!found) {
            stats[stats_count].uid = uid;
            stats[stats_count].max_duration = duration;
            stats_count++;
        }
    }
    
    for(int i = 0; i < stats_count; i++){
        double square = (double)stats[i].max_duration * stats[i].max_duration;
        if(square > disp){
            char buff[64];
            int len = snprintf(buff, sizeof(buff), "%u, %u\n", stats[i].uid, stats[i].max_duration); 
            if(write(1, buff, len) != len){
                err(7, "Could not write to stdout!");
            }
        }
    }

    free(arr);
    close(fd);
    exit(0);
}
