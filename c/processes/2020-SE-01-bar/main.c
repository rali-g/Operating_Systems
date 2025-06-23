#include <err.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv) {
	if(argc != 2){
        errx(1, "invalid args count");
	}

	int fd = open("../2020-SE-01-foo/myfifo", O_RDONLY);
	if(fd == -1){
        err(2, "open failed");
	}
	if(dup2(fd, 0) == -1){
        err(3, "dup2 failed");
	}
	close(fd);
	if(execl(argv[1], argv[1], (char*)NULL) == -1){
        err(4, "execl failed");
	}
}
