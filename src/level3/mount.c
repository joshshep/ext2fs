#include "../../include/level3cmd.h"


//returns true if the provided filesys name is already mounted
int isMounted(char *filesys) {
	for (int i=0; i<NMOUNT; ++i) {
		if (strcmp(mount[i].name, filesys) == 0) {
			return 1;
		}
	}
	return 0;
}

int my_get_block(int fd, int blk, char *buf)
{
	d_printf("my_get_block: fd=%d blk=%d buf=%p\n", fd, blk, (void*) buf);
	my_lseek(fd, (long)blk*BLKSIZE);
	v_printf("line: %d\n",__LINE__);
	myread(fd, buf, BLKSIZE);
	v_printf("line: %d\n",__LINE__);
	return 0;
}


int isEXT2fs(int fd) {
	char buf[BLKSIZE];
	get_block(fd, 1, buf);
	SUPER *sp = (SUPER *) buf;

	//Write C code to check EXT2 FS; if not EXT2 FS: exit
	if (sp->s_magic == SUPER_MAGIC) {
		return 1;
	}
	return 0;
}

int display_cur_mnts() {

	printf("current mounts:\n");
	return 0;
}

int cmd_mount(int argc, char** args) {
	// check number of arguments
	if (argc > 3) {
		printf("mount: error: too many arguments\n");
		return -1;
	} else if (argc == 1) {
		display_cur_mnts();
		return 0;
	}
	char* filesys = args[1];
	char* mnt_pnt = args[2];
	if (isMounted(filesys)) {
		printf("mount: filesys already mounted\n");
		return 0;
	}
	int imnt;
	for (imnt=0; imnt<NMOUNT; ++imnt) {
		if (mount[imnt].dev == 0) {
			// free mount entry
			break;
		}
	}
	if (imnt==NMOUNT) {
		printf("mount: error: no free mount table entries remaining\n");
		return -1;
	}

	int curdev = running->cwd->dev;
	/*if (pathname[0] == '/') {
		curdev = root->dev;
	} else {
		curdev = running->cwd->dev;
	}

	if (!getino(&curdev, filesys)) {
		v_printf("mount: error: couldn't find filesys \"%s\"\n",filesys);
		return -1;
	}*/


	//open for RW
	int fd = open(filesys, O_RDWR);
	if (fd < 0) {
		printf("mount: couldn't find \"%s\"\n",filesys);
	}

	if (!isEXT2fs(fd)) {
		printf("mount: error: supplied fs isn't EXT2\n");
		return -1;
	}
	v_printf("mount: fs (fd=%d) is EXT2\n",fd);

	int ino = getino(&curdev, mnt_pnt);
	if (!ino) {
		printf("mount: error: couldn't find mount point\n");
		return -2;
	}
	MINODE* mip = iget(curdev, ino);
	INODE* ip = & mip->INODE;
	if (!S_ISDIR(ip->i_mode)) {
		printf("mount: error: mount point must be a dir\n");
		return -3;
	}

	mip->mounted = 1;
	mip->dirty = 1;
	mip->mountptr = & mount[imnt];
	strcpy(mount[imnt].name, mnt_pnt);
	mount[imnt].mounted_inode = mip;
	mount[imnt].dev = fd;
	return 0;
}
