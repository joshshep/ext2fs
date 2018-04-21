#include "../../include/level3cmd.h"

int umount(char *filesys) {
	v_printf("umount: filesys=\"%s\"\n",filesys);
	int imnt;
	for (imnt=0; imnt<NMOUNT; ++imnt) {
		if (strcmp(mount[imnt].name, filesys) == 0) {
			break;
		}
	}
	if (imnt == NMOUNT) {
		printf("umount: error: couldn't find supplied filesys in mount table\n");
		return -1;
	}
	v_printf("umount: found name in mount table\n");

	for(int imip=0; imip<NMINODE; ++imip) {
		if (minode[imip].dev == mount[imnt].dev) {
			//printf("umount: error: filesys busy\n");
			//return -2;
		}
	}

	v_printf("umount: the mounted mip is not busy\n");
	v_printf("umount: dellocating mip/unmouting dir\n");
	MINODE *mip = mount[imnt].mounted_inode;
	mip->mounted = 0;
	iput(mip);
	return 0;
}
