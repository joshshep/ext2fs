#include "../../include/level2cmd.h"

int mywrite(int fd, char* buf, int nbytes) {

	//v_printf("mywrite: fd=%d buf=%p nbytes=%d\n",fd,(void *) buf, nbytes);
	OFT* oftp = running->fd[fd];
	//v_printf("IN write file offset: %ld\n",oftp->offset);
	MINODE* mip = oftp->inodeptr;
	INODE* ip = & mip->INODE;

	int cnt = 0;
	char* cq = buf;
	while (nbytes > 0) {
		int lbno = oftp->offset / BLKSIZE;
		int startByte = oftp->offset  % BLKSIZE;
		int bno;

		bno = get_physical_bno(mip, lbno);

		char wbuf[BLKSIZE];
		get_block(mip->dev, bno, wbuf);

		/* copy from startByte to buf[ ], at most remain bytes in this block */
		char* cp = wbuf + startByte;
		int remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

		// number of bytes to write in *this* block
		int bytes2write = MIN(remain, nbytes);
		memcpy(cp, cq, bytes2write);

		cnt += bytes2write;
		oftp->offset += bytes2write;
		if (oftp->offset > ip->i_size) {
			ip->i_size += bytes2write;
		}
		cq += bytes2write;
		nbytes -= bytes2write;

		put_block(mip->dev, bno, wbuf);

	}
	//ip->i_size = oftp->offset; //cnt should equal what nbytes was initially
	// TODO should we update the ip->i_size field?
	//TODO
	/*
	OPTIMIZATION OF write Code

As in read(), the above inner while(remain > 0) loop can be optimized:
Instead of copying one byte at a time and update the control variables on each
byte, TRY to copy only ONCE and adjust the control variables accordingly.

*/
	mip->dirty = 1;
	//v_printf("mywrite: wrote %d char to file descriptor %d\n", cnt, fd);
	//v_printf("OUT write file offset: %ld\n",oftp->offset);
	return cnt;
}

int write_file(int fd, char* str) {

	OFT* oftp = running->fd[fd];
	if (oftp->mode < 1 || oftp->mode > 3) {
		//not W,WR, APPEND
		printf("write_file: error: invalid mode for fd\n");
		return -1;
	}
	//TODO malloc()/memcpy()
	char buf[BLKSIZE*BLKSIZE];
	strcpy(buf, str);
	int nbytes = strlen(str);
	return mywrite(fd, buf, nbytes);
}
