#include "../../include/level1cmd.h"

int cmd_symlink(int argc, char** args) {
	// check number of arguments
	if (argc < 3) {
		printf("symlink: error: too few arguments\n");
		return -1;
	} else if (argc > 3) {
		printf("symlink: error: too many arguments\n");
		return -1;
	}
	char* oldName = args[1];
	char* newName = args[2];

	int dev = getStartDev(newName);

	char sbuf[MAX_LINE], sbuf2[MAX_LINE];
	strcpy(sbuf, newName); strcpy(sbuf2, newName);

	char * new_dname = dirname(sbuf);

	int new_pino = getino(&dev,new_dname);
	MINODE * new_pmip = iget(dev, new_pino);
	INODE * new_pip = & new_pmip->INODE;

	char* creat_args[2];
	creat_args[0] = "creat";
	creat_args[1] = sbuf2;
	if (cmd_creat(2, creat_args) != EXIT_SUCCESS) {
		iput(new_pmip);
		return -1;
	}

	int new_ino = getino(&dev, sbuf2);
	MINODE * new_mip = iget(dev, new_ino);
	INODE * new_ip = & new_mip->INODE;

	new_ip->i_mode = (new_ip->i_mode & 0x0FFF) | 0xA000;
	new_ip->i_size = strlen(oldName);

	v_printf("copying \"%s\" into ip->i_block\n",oldName);

	strcpy((char *)new_ip->i_block, oldName);
	new_mip->dirty = 1;

	new_pip->i_atime = time(0L);
	new_pmip->dirty = 1;

	iput(new_mip);
	iput(new_pmip);

	return 0;
}

int cmd_readlink(int argc, char** args) {
	// check number of arguments
	if (argc < 2) {
		printf("readlink: error: too few arguments\n");
		return -1;
	}
	char* pathname = args[1];
	int dev = getStartDev(pathname);
	int ino = getino(&dev, pathname);
	MINODE * mip = iget(dev, ino);
	if (!mip) {
		printf("readlink: error: file does not exist\n");
		return -2;
	}
	INODE * ip = & mip->INODE;

	if (!S_ISLNK(ip->i_mode)) {
		printf("readlink: error \"%s\" is not a symlink\n",pathname);
		iput(mip);
		return -1;
	}
	v_printf("\"%s\" is symlink\n", pathname);
	v_printf("contents:\n");

	//tricky tricky Mr. KC
	printf("%s\n",(char *) ip->i_block);

	iput(mip);
	return 0;
}
