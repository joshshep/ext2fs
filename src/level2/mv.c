#include "../../include/level2cmd.h"

int cmd_mv(int argc, char** args) {
	// check number of arguments
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
		//link src->dst
		char* cmd_args[3];
		cmd_args[0] = "link";
		cmd_args[1] = src;
		cmd_args[2] = dst;
		if (cmd_link(3, cmd_args) != EXIT_SUCCESS) {
			printf("mv: error: couldn't link files\n");
			return -1;
		}

		//unlink src
		cmd_args[0] = "unlink";
		if (cmd_unlink(2, cmd_args) != EXIT_SUCCESS) {
			printf("mv: error: couldn't unlink src file\n");
			return -1;
		}
	} else {
		//different devices
		v_printf("mv: src and dst are DIFF device\n");
		//copy src->dest
		char* cmd_args[3];
		cmd_args[0] = "cp";
		cmd_args[1] = src;
		cmd_args[2] = dst;
		if (cmd_cp(3, cmd_args) != EXIT_SUCCESS) {
			printf("mv: error: couldn't copy file\n");
			return -1;
		}
		
		//unlink src
		cmd_args[0] = "unlink";
		if (cmd_unlink(2, cmd_args) != EXIT_SUCCESS) {
			printf("mv: error: couldn't unlink src file\n");
			return -1;
		}
	}
	return 0;
}
