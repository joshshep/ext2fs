#include "../../include/level2cmd.h"


//TODO
/*
dup(int fd):
{
	verify fd is an opened descriptor;
	duplicates (copy) fd[fd] into FIRST empty fd[ ] slot;
	increment OFT's refCount by 1;
}

dup2(int fd, int gd):
{
	CLOSE gd fisrt if it's already opened;
	duplicates fd[fd] into fd[gd];
}

As you already know, these are needed for I/O redirections
*/

int close_file(int fd) {
	if (fd < 0 || fd >= NFD) {
		printf("close_file: error: fd out of range\n");
		return -1;
	}
	OFT * oftp = running->fd[fd];
	if (!oftp) {
		printf("close_file: error: bad oft in running->fd[fd]\n");
		return -2;
	}
	running->fd[fd] = 0;
	--oftp->refCount;
	if (oftp->refCount > 0) {
		return 0;
	}

	// last user of this OFT entry ==> dispose of the Minode[]
	MINODE * mip = oftp->inodeptr;
	iput(mip);

	return 0;
}

// returns the original offset of fd
int my_lseek(int fd, int pos) {
	OFT * oftp = running->fd[fd];
	if (!oftp) {
		printf("lseek: error: bad oft in running->fd[fd]\n");
		return -1;
	}
	MINODE *mip = oftp->inodeptr;
	INODE *ip = & mip->INODE;
	if (pos < 0 || pos >= ip->i_size) {
		printf("lseek: error: position out of bounds of file");
	}
	int origOffset = oftp->offset;
	oftp->offset = pos;

	return origOffset;
}
/*
This function displays the currently opened files as follows:

			fd     mode    offset    INODE
		 ----    ----    ------   --------
			 0     READ    1234   [dev, ino]
			 1     WRITE      0   [dev, ino]
		--------------------------------------
to help the user know what files has been opened.
*/
int pfd() {
	printf("  fd     mode    offset    INODE\n");
	printf(" ----    ----    ------   --------\n");
	for (int fd=0; fd<NFD; fd++) {
		OFT * oftp = running->fd[fd];
		if (!oftp) {
			continue;
		}
		printf("  %2d    ", fd);
		switch(oftp->mode) {
			case 0:
				printf("READ  ");
				break;
			case 1:
				printf("WRITE ");
				break;
			case 2:
				printf("R/W   ");
				break;
			case 3:
				printf("APPEND");
				break;
			default:
				printf("pfd: error: invalid file mode\n");
				return -1;
		}
		MINODE *mip = oftp->inodeptr;
		printf("%6ld   [%2d,%d]\n",oftp->offset, mip->dev, mip->ino);

	}
	printf("--------------------------------------\n");
	return 0;
}

//mode = 0|1|2|3 for R|W|RW|APPEND
//returns file descriptor
int open_file(char *pathname, int mode) {
	v_printf("open_file(): pathname=\"%s\" ; mode=%d\n", pathname, mode);
	int dev = getStartDev(pathname);
	char sbuf[MAX_LINE];
	strcpy(sbuf, pathname);
	int ino = getino(&dev, sbuf);
	if (!ino) {
		// if the mode is a write type, then we should make the file
		if (mode < 1 || mode > 3) {
			printf("open_file: error: mode is read AND can't find '%s'\n",pathname);
			return -1;
		}
		strcpy(sbuf, pathname);
		creat_file(sbuf);
		strcpy(sbuf, pathname);
		ino = getino(&dev, sbuf);

		if (!ino) {
			printf("open_file: error: we can't find the file we just made\n");
			return -1;
		}

	}
	v_printf("found ino from pathname: %d\n",ino);
	MINODE * mip = iget(dev, ino);
	INODE *ip = & mip->INODE;
	//v_printf("open: opening file of size %d\n",ip->i_size);

	if (S_ISDIR(ip->i_mode)) {
		v_printf("is a DIR file\n");
		printf("error: \"%s\" must be a reg file\n",pathname);
		return -2;
	} else if (S_ISREG(ip->i_mode)){
		v_printf("is a REG file (not dir)\n");
		//ls_file();
		//is dir
	} else {
		printf("error: is neither DIR nor REG\n");
		return -3;
	}
	//if (oft->)
	/*
	TODO
	Check whether the file is ALREADY opened with INCOMPATIBLE mode:
					 If it's already opened for W, RW, APPEND : reject.
					 (that is, only multiple R are OK)
	*/

	// TODO oftp iget() and iputs()?
	OFT *oftp = NULL;
	OFT *first_empty_oftp = NULL;
	int i;
	for (i=0; i<NOFT; i++) {
		oftp = & oft[i];
		if (oftp->inodeptr == mip) {
			v_printf("found mip in open file table\n");
			if (oft->mode != 0) { // 0 is read
				v_printf("error: incompatible mode\n");
				return -1;
			}
			first_empty_oftp = oftp;
			break;
		} else if (oftp->refCount == 0) {
			//TODO is this how we should check if it's free?
			//v_printf("found an empty entry in Open File Table\n");
			if (!first_empty_oftp) {
				first_empty_oftp = oftp;
			}
		}
	}
	if (!first_empty_oftp ) {
		printf("open_file: error: no room in open file table\n");
		return -1;
	}

	if (i >= NOFT) {
		v_printf("making NEW entry for oft\n");
	}

	oftp = first_empty_oftp;

	v_printf("setting oftp offset\n");
	switch(mode) {
		case 0: // R
			oftp->offset = 0;
			break;
		case 1: // W
			truncate(mip);
			oftp->offset = 0;
			break;
		case 2: // RW
			oftp->offset = 0;
			break;
		case 3: // APPEND
			oftp->offset = ip->i_size;
			break;
		default:
			printf("open(): erro: invalid mode (%d)\n", mode);
			return -4;

	}
	//NFD == 16
	for (i=0; i<NFD; i++) {
		if (!running->fd[i]) {
			break;
		}
	}
	if (i >= NFD ) {
		printf("open_file(): error: couldn't find a place for OFT entry");
		return -5;
	}

	running->fd[i] = oftp;
	oftp->inodeptr = mip;
	++oftp->refCount;

	ip->i_atime = time(0L);
	if (mode != 0) {
		/*
	 8. update INODE's time field
				for R: touch atime.
				for W|RW|APPEND mode : touch atime and mtime
		*/
		ip->i_mtime = time(0L);
	}


	mip->dirty = 1;

	//v_printf("open: file of size %d\n",ip->i_size);

	return i;
}
