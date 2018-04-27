#include "../../include/level2cmd.h"

int cmd_cp(int argc, char** args) {
	// check number of arguments
	if (argc > 3) {
		printf("cp: error: too many arguments\n");
		return -1;
	} else if (argc < 3) {
		printf("cp: error: missing operand\n");
		return -1;
	}
	char* src = args[1];
	char* dst = args[2];
	int fd = open_file(src, 0);
	if (fd < 0 ) {
		return -1;
	}
	int gd = open_file(dst, 1);
	if (gd < 0 ) {
		return -1;
	}
	int n;
	char buf[BLKSIZE+1];
	do {
		n = myread(fd, buf, BLKSIZE);
		v_printf("\ncp: read %d bytes\n",n);
		buf[n] = 0;
		//v_printf("block = %s",buf);
		n = mywrite(gd, buf, n);
		v_printf("\ncp: write %d bytes\n",n);
	} while(n == BLKSIZE);
	v_printf("cp: close fd (%d) and gd (%d)\n",fd,gd);
	close_file(fd);
	close_file(gd);
	return 0;
}
