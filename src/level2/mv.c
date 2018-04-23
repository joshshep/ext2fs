#include "../../include/level2cmd.h"

int cmd_mv(int argc, char** args) {
	if (argc > 2) {
		printf("mv: error: too many arguments\n");
		return -1;
	} else if (argc < 2) {
		printf("mv: error: missing operand\n");
		return -1;
	}
	char* src = args[1];
	char* dst = args[2];
	int srcdev = getStartDev(src);

	int dstdev = getStartDev(dst);

	int srcino = getino(&srcdev, src);
	if(!srcino) {
		printf("mv: \"%s\" does not exist\n",src);
		return -1;
	}

	char sbuf[MAX_LINE];
	strcpy(sbuf, dst);
	char *dname = dirname(sbuf);

	int dstino = getino(&dstdev, dname);
	if (!dstino) {
		printf("mv: \"%s\" does not exist\n",dname);
		return -2;
	}

	if (srcdev == dstdev) {
		// same device
		v_printf("mv: src and dst are SAME device\n");
		do_link(src, dst);
		//if (S_I)
		do_unlink(src);
	} else {
		//different devices
		v_printf("mv: src and dst are DIFF device\n");
		cp(src, dst);
		do_unlink(src);
	}
	return 0;
}
