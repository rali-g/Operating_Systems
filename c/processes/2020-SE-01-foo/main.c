#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char** argv) {
	if(argc != 2){
        errx(1, "invalid args count");
	}

    if(mkfifo("myfifo", 0666) == -1 && errno != EEXIST){
        err(2, "mkfifo failed");
    }

    int fd = open("myfifo", O_WRONLY);
    if(fd == -1){
        err(3, "open failed");
    }

    if(dup2(fd, 1) == -1){
        err(4, "dup2 failed");
    }
    close(fd);
    if(execlp("cat", "cat", argv[1], (char*)NULL) == -1){
        err(5, "exec failed");
    }
}
