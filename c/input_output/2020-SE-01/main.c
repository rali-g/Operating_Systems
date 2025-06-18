#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <err.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint16_t start;
    uint16_t count;
} interval_t;

int file_size(int fd){
    struct stat st;
    if(fstat(fd, &st) == -1){
        err(2, "Could not execute fstat!");
    }
    return st.st_size;
}

int open_rd(const char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) err(2, "Could not open file %s", filename);
    return fd;
}

int open_wr(const char* filename) {
    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fd == -1) err(3, "Could not open output file %s", filename);
    return fd;
}

uint16_t read_count(int fd){
    uint8_t header[16];
    if(read(fd, header, sizeof(header)) != sizeof(header)){
        err(4, "Could not read header!");
    }
    uint16_t count = (((uint16_t)header[5] << 8) | header[4]);
    return count;
}

void read_postfix(int fd_post, int fd_cruc, uint16_t start, uint16_t count){
    if(lseek(fd_post, sizeof(uint32_t) * start, SEEK_CUR) == -1){
        err(10, "Could not lseek!");
    }

    uint32_t c;
    for(uint16_t i = 0; i < count; i++){
        if(read(fd_post, &c, sizeof(c)) != sizeof(c)){
            err(11, "Could not read!");
        }
        if(write(fd_cruc, &c, sizeof(c)) != sizeof(c)) {
            err(12, "Could not write to crucifixus");
        }
    }
}

void read_prefix(int fd_pre, int fd_cruc, uint16_t start, uint16_t count) {
    if (lseek(fd_pre, sizeof(uint8_t) * start, SEEK_CUR) == -1) {
        err(13, "Could not lseek prefix!");
    }

    uint8_t c;
    for (uint16_t i = 0; i < count; i++) {
        if (read(fd_pre, &c, sizeof(c)) != sizeof(c)) {
            err(14, "Could not read prefix!");
        }
        if (write(fd_cruc, &c, sizeof(c)) != sizeof(c)) {
            err(15, "Could not write prefix to crucifixus!");
        }
    }
}

void read_suffix(int fd_suf, int fd_cruc, uint16_t start, uint16_t count) {
    if (lseek(fd_suf, sizeof(uint64_t) * start, SEEK_CUR) == -1) {
        err(16, "Could not lseek suffix!");
    }

    uint64_t c;
    for (uint16_t i = 0; i < count; i++) {
        if (read(fd_suf, &c, sizeof(c)) != sizeof(c)) {
            err(17, "Could not read suffix!");
        }
        if (write(fd_cruc, &c, sizeof(c)) != sizeof(c)) {
            err(18, "Could not write suffix to crucifixus!");
        }
    }
}

void read_infix(int fd_inf, int fd_cruc, uint16_t start, uint16_t count) {
    off_t offset = sizeof(uint16_t) * start;
    if (lseek(fd_inf, offset, SEEK_CUR) == -1) {
        err(21, "Could not lseek infix!");
    }
    uint16_t c;
    for (uint16_t i = 0; i < count; i++){
        if (read(fd_inf, &c, sizeof(c)) != sizeof(c)) {
            err(22, "Could not read infix!");
        }
        if (write(fd_cruc, &c, sizeof(c)) != sizeof(c)) {
            err(23, "Could not write infix to crucifixus!");
        }
    }
}

int main(int argc, char** argv) {
	if(argc != 7){
        errx(1, "Invalid arguments count!");
	}
    
    int fd_aff = open_rd(argv[1]);
    int fd_post = open_rd(argv[2]);
    int fd_pre = open_rd(argv[3]);
    int fd_inf = open_rd(argv[4]);
    int fd_suf = open_rd(argv[5]);
    int fd_cruc = open_wr(argv[6]);
    
    uint16_t aff_count = read_count(fd_aff);
    uint16_t post_count = read_count(fd_post);
    uint16_t pre_count = read_count(fd_pre);
    uint16_t inf_count = read_count(fd_inf);
    uint16_t suf_count = read_count(fd_suf);

    printf("%u\n", aff_count);
    //exit(0);

    if(file_size(fd_aff) % sizeof(uint16_t) != 0){
        errx(5, "Invalid affix file input!");
    }
    if(file_size(fd_post) % sizeof(uint32_t) != 0){
        errx(6, "Invalid postfix file input");
    }
    if(file_size(fd_pre) % sizeof(uint8_t) != 0){
        errx(7, "Invalid prefix file input!");
    }
    if(file_size(fd_inf) % sizeof(uint16_t) != 0){
        errx(8, "Invalid infix file input!");
    }
    if(file_size(fd_suf) % sizeof(uint64_t) != 0){
        errx(9, "Invalid suffix file input!");
    }

    for(int i = 0; i < aff_count; i++){
        interval_t post, pre, inf, suf;
        if (read(fd_aff, &post, sizeof(post)) != sizeof(post) ||
            read(fd_aff, &pre, sizeof(pre)) != sizeof(pre) ||
            read(fd_aff, &inf, sizeof(inf)) != sizeof(inf) ||
            read(fd_aff, &suf, sizeof(suf)) != sizeof(suf)) {
            errx(20, "Could not read intervals from affix");
        }
        
        if (post.start + post.count > post_count) {
            errx(21, "Postfix interval out of range!");
        }
        if (pre.start + pre.count > pre_count) {
            errx(22, "Prefix interval out of range!");
        }
        if (inf.start + inf.count > inf_count) {
            errx(23, "Infix interval out of range!");
        }
        if (suf.start + suf.count > suf_count) {
            errx(24, "Suffix interval out of range!");
        }

        read_postfix(fd_post, fd_cruc, post.start, post.count);
        read_prefix(fd_pre, fd_cruc, pre.start, pre.count);
        read_infix(fd_inf, fd_cruc, inf.start, inf.count);
        read_suffix(fd_suf, fd_cruc, suf.start, suf.count);   
    }

    close(fd_aff);
    close(fd_post);
    close(fd_pre);
    close(fd_inf);
    close(fd_suf);
    close(fd_cruc);
}
