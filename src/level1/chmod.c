#include "../../include/level1cmd.h"

/*
chmod multiple files
*/
int cmd_chmod(int argc, char** args) {
	if (argc < 3) {
		printf("chmod: error: too few arguments\n");
		return -1;
	}
	char *permissions = args[1];
	char *fname       = args[2];
	v_printf("chmod: permissions=\"%s\" fname=\"%s\"\n",permissions, fname);

	int dev = getStartDev(fname);

	int ino = getino(&dev, fname);
	if (!ino) {
		printf("chmod: can't find '%s'\n",fname);
		return -1;
	}
	v_printf("chmod: found ino from fname: %d\n",ino);
	MINODE * mip = iget(dev, ino);
	INODE *ip = &mip->INODE;

	// octal
	int perms = strtol(permissions, NULL, 8);

	//reset permissions
	ip->i_mode = ip->i_mode & 0xF000;
	ip->i_mode |= perms;

	mip->dirty = 1;
	iput(mip);

	return 0;
}
