#include "../../include/level1cmd.h"

int do_touch(char *fname) {
	v_printf("touch: filename \"%s\"\n",fname);
	int dev = getStartDev(fname);
	char sbuf[MAX_LINE];
	strcpy(sbuf, fname);
	int ino = getino(&dev, fname);
	if(!ino) {
		v_printf("touch: couldn't find filename \"%s\".\n",sbuf);
		if (creat_file(sbuf) != EXIT_SUCCESS) {
			printf("touch: couldn't create file\n");
			return -1;
		}
		return EXIT_SUCCESS;
	}
	MINODE *mip = iget(dev, ino);
	INODE *ip = & mip->INODE;
	v_printf("touch: updating mtime\n");
	ip->i_mtime = time(0L);
	mip->dirty = 1;
	iput(mip);
	return EXIT_SUCCESS;
}
