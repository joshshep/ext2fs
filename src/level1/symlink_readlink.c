#include "../../include/level1cmd.h"

int cmd_symlink(int argc, char** args) {
	if (argc < 2) {
		printf("symlink: error: too few arguments\n");
		return -1;
	} else if (argc > 2) {
		printf("symlink: error: too many arguments\n");
		return -1;
	}
	char* oldName = args[1];
	char* newName = args[2];

	int dev;
	if (newName[0] == '/') {
		v_printf("do_symlink: starting at root\n");
		dev = root->dev;
	} else {
		v_printf("do_symlink: not starting at root\n");
		dev = running->cwd->dev;
	}

	char sbuf[MAX_LINE], sbuf2[MAX_LINE];
	strcpy(sbuf, newName); strcpy(sbuf2, newName);

	char * new_dname = dirname(sbuf);

	int new_pino = getino(&dev,new_dname);
	MINODE * new_pmip = iget(dev, new_pino);
	INODE * new_pip = & new_pmip->INODE;

	if (creat_file(sbuf2) != EXIT_SUCCESS) {
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
	if (argc < 2) {
		printf("readlink: error: too few arguments\n");
		return -1;
	}
	char* pathname = args[1];
	int dev;
	if (pathname[0] == '/') {
		v_printf("readlink: starting at root\n");
		dev = root->dev;
	} else {
		v_printf("readlink: not starting at root\n");
		dev = running->cwd->dev;
	}
	int ino = getino(&dev, pathname);
	MINODE * mip = iget(dev, ino);
	INODE * ip = & mip->INODE;

	if (!S_ISLNK(ip->i_mode)) {
		v_printf("readlink: error \"%s\" is not a symlink\n",pathname);
		iput(mip);
		return -1;
	}
	v_printf("\"%s\" is symlink\n", pathname);
	printf("contents:\n");

	//tricky tricky Mr. KC
	printf("%s\n",(char *) ip->i_block);

	iput(mip);
	return 0;
}
