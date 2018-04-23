#include "../../include/level1cmd.h"

int mymkdir(MINODE *pmip, const char *name) {
	int ino = ialloc(pmip->dev);
	int bno = balloc(pmip->dev);
	v_printf("mkdir(): new ino=%d ; bno=%d\n",ino,bno);

	MINODE * mip = iget(pmip->dev, ino);
	INODE * ip = &mip->INODE;

	ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
	ip->i_uid  = running->uid;	// Owner uid
	ip->i_gid  = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes
	ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;             // new DIR has one data block

	// memset(& i_block[1], 0, 14 );
	for (int i=1; i<15; ++i) {
		ip->i_block[i] = 0;
	}

	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk

	char blkbuf[BLKSIZE];
	get_block(pmip->dev, bno, blkbuf);

	char * cp = blkbuf;
	DIR * dp = (DIR *) blkbuf;

	dp->inode     = ino;
	int IDEAL_LEN = get_ideal_len(strlen(".."));
	dp->rec_len   = IDEAL_LEN;
	dp->name_len  = strlen(".");
	dp->file_type = EXT2_FT_DIR;
	dp->name[0]   = '.';

	cp += dp->rec_len;
	dp = (DIR*) cp;

	dp->inode     = pmip->ino;
	dp->rec_len   = (BLKSIZE - (cp - blkbuf));

	dp->name_len  = strlen("..");
	dp->file_type = EXT2_FT_DIR;
	strcpy(dp->name, "..");

	put_block(pmip->dev, bno, blkbuf);

	enter_name(pmip, ino, name, EXT2_FT_DIR);

	return 0;
}

/*

TODO: make multiple directories

*/
int cmd_mkdir(int argc, char** args) {
	// check number of arguments
	if (argc < 2) {
		printf("mkdir: error: too few arguments\n");
		return -1;
	}
	char* pathname = args[1];

	int dev = getStartDev(pathname);
	char bname_buf[MAX_LINE];
	char dname_buf[MAX_LINE];
	strcpy(bname_buf, pathname);
	strcpy(dname_buf, pathname);
	char * bname = basename(bname_buf);
	char * dname = dirname(dname_buf);

	v_printf("mkdir: dname: %s ; bname: %s\n",dname, bname);

	int pino = getino(&dev, dname);
	if (!pino) {
		printf("mkdir(): error: couldn't find parent directory \"%s\"\n", dname);
		return -1;
	}
	MINODE *pmip = iget(dev, pino);
	int ino = search(pmip, bname);
	if (ino) {
		printf("mkdir(): error: file \"%s\" already exists\n",pathname);
		return -2;
	}
	v_printf("mkdir(): parent ino = %d\n",pino);
	//error check
	mymkdir(pmip, bname);

	INODE * pip = & pmip->INODE;
	++pip->i_links_count;
	pip->i_atime = time(0L);

	pmip->dirty = 1;
	iput(pmip);

	return 0;
}


int mycreat(MINODE *pmip, const char *name) {
	int ino = ialloc(pmip->dev);
	int bno = balloc(pmip->dev);
	v_printf("mycreat(): new ino=%d ; bno=%d ; name=\"%s\"\n",ino,bno,name);

	MINODE * mip = iget(pmip->dev, ino);
	INODE * ip = &mip->INODE;

	ip->i_uid  = running->uid;	// Owner uid
	ip->i_gid  = running->gid;	// Group Id

	//not dir
	//ip->i_mode = 0x81A4;
	//ip->i_size = BLKSIZE;		// Size in bytes
	//ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_mode = 0x81A4;
	ip->i_size = 0;		// Size in bytes
	ip->i_links_count = 1;


	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks
	ip->i_block[0] = bno;             // new DIR has one data block

	for (int i=1; i<15; ++i) {
		ip->i_block[i] = 0;
	}

	enter_name(pmip, ino, name, EXT2_FT_REG_FILE);
	mip->dirty = 1;               // mark minode dirty
	iput(mip);                    // write INODE to disk


	return 0;
}

int cmd_creat(int argc, char** args) {
	// check number of arguments
	if (argc < 2) {
		printf("creat: error: too few arguments\n");
		return -1;
	}
	char* pathname = args[1];

	int dev = getStartDev(pathname);
	char dname_buf[MAX_LINE], bname_buf[MAX_LINE];
	strcpy(dname_buf, pathname);
	strcpy(bname_buf, pathname);
	char * bname = basename(bname_buf);
	char * dname = dirname(dname_buf);

	v_printf("creat_file: dname: %s ; bname: %s\n",dname, bname);

	int pino = getino(&dev, dname);
	if(!pino) {
		printf("creat: error: directory \"%s\" does not exist.\n", dname);
		return -1;
	}

	MINODE *pmip = iget(dev, pino);

	int cand_pino = search(pmip, bname);
	if(cand_pino) {
		printf("creat: error: cannot create \"%s\". File already exists.\n", pathname);
		iput(pmip);
		return -2;
	}

	v_printf("creat_file(): parent ino = %d\n", pino);
	//error check
	mycreat(pmip, bname);

	INODE * pip = & pmip->INODE;

	//not dir
	//++pip->i_links_count;


	pip->i_atime = time(0L);

	pmip->dirty = 1;
	iput(pmip);

	return 0;
}
