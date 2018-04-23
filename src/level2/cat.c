#include "../../include/level2cmd.h"

/*

TODO: do what cat actually does

*/
int cmd_cat(int argc, char** args) {
	char* filename = args[1];
	char buf[BLKSIZE+1]; //one for the null terminator
	v_printf("cat: open file for read\n");
	int fd = open_file(filename, 0); //open file for READ
	if (fd < 0) {
		return -1;
	}
	v_printf("fd: %d\n",fd);
	v_printf("cat: output:\n");
	v_printf("----------------------------------\n");

	int n;
	do {
		n = myread(fd, buf, BLKSIZE);
		buf[n] = 0; // null terminated string (for printing);
		printf("%s",buf);
		// print can print BLKSIZE + 1 characters (including null terminator)
	} while (n == BLKSIZE);
	v_printf("----------------------------------\n");
	close_file(fd);
	v_printf("cat: close file\n");

	return 0;
}
