#include "../../include/level1cmd.h"

int mychdir(char * pathname) {
	int dev;
	if (pathname[0] == '/') {
		v_printf("mychdir: starting at root\n");
		dev = root->dev;
		int imnt;
		for (imnt=0; imnt < NMOUNT; ++imnt) {
			if (mount[imnt].dev == running->cwd->dev){
				v_printf("getino: crossing over mount points\n");
				running->cwd = &secret_root;
				v_printf("cd: is dir file\n");
				return 0;
			}
		}
	} else {
		v_printf("mychdir: not starting at root\n");
		dev = running->cwd->dev;
	}
	int ino = getino(&dev, pathname);
	if (!ino) {
		printf("mychdir: can't find '%s'\n",pathname);
		return -1;
	}

	if (ino==2 && strcmp(pathname,"..")==0) {
		int imnt;
		for (imnt=0; imnt < NMOUNT; ++imnt) {
			if (mount[imnt].dev == running->cwd->dev){
				v_printf("getino: crossing over mount points\n");
				running->cwd = &secret_root;
				v_printf("cd: is dir file\n");
				return 0;
			}
		}
	}

	v_printf("found ino from pathname: %d\n",ino);
	MINODE * mip = iget(dev, ino);

	if (S_ISDIR(mip->INODE.i_mode)) {
		v_printf("is a DIR file\n");
	} else if (S_ISREG(mip->INODE.i_mode)){
		v_printf("is a REG file (not dir)\n");
		printf("error: pathname must be a dir\n");
		return -2;
		//ls_file();

		//is dir

	} else {
		printf("error: is neither DIR nor REG\n");
		return -3;
	}

	iput(running->cwd);

	v_printf("setting new cwd\n");
	running->cwd = mip;

	/*
	Write C code to do these:

	determine initial dev as in ls()
	convert pathname to (dev, ino);
	get a MINODE *mip pointing at (dev, ino) in a minode[ ];

	if mip->INODE is NOT a DIR: reject and print error message;

	if mip->INODE is a DIR{
		dispose of OLD cwd;
		set cwd to mip;
	}
	*/
	return 0;
}

// get's the name of the child of pmip that has ino
// writes the result to name
int getChildNameFromIno(MINODE* pmip, int ino, char *name) {
	// only search the first 12 blocks (direct)
	INODE *pip = &pmip->INODE;
	for (int blk_index=0; blk_index < 12 && pip->i_block[blk_index]; blk_index++) {
		int bno = pip->i_block[blk_index];
		//v_printf("i_block[%d] = %d\n",blk_index, bno);
		get_block(pmip->dev, bno, dbuf);

		dp = (DIR *) dbuf;
		char * cp = dbuf;

		while (cp < & dbuf[BLKSIZE]) {
			//v_printf("%4d %4d %4d %s\n",
			//        dp->inode, dp->rec_len, dp->name_len, sbuf);
			if (ino == dp->inode) {
				// found a match!
				strncpy(name, dp->name, dp->name_len);
				name[dp->name_len] = 0;
				return 0;
			}
			cp += dp->rec_len;
			dp = (DIR *) cp;
		}
	}
	name[0] = 0;
	//we couldn't find the inode in the parent directory
	return -1;
}

int rpwd(MINODE *mip) {
	// get the parents inode number
	int pino = search(mip, "..");//getino(&mip->dev, pathname);
	if (!pino) {
		printf("pwd: error: couldn't find parent\n");
	}
	//v_printf("mip->ino=%d ; pino=%d\n",mip->ino,pino);
	if (pino != mip->ino) {
		// the parent isn't the child (i.e. we aren't at the root yet)
		MINODE *pmip = iget(mip->dev, pino);
		if (!pmip) {
			printf("pwd: error: we couldn't find the parent that we just successfully searched for\n");
			return -1;
		}
		char name[MAX_LEN_DIR_NAME];
		if (getChildNameFromIno(pmip, mip->ino, name) != 0) {
			//the inode isn't in the parent's directory entries?!
			printf("pwd: error: can't find child?!\n");
			return -2;
		}
		//v_printf("name: \"%s\"\n",name);
		rpwd(pmip);

		printf("/%s",name);
		iput(pmip);
	}
	return 0;
}

int pwd_inline(MINODE *mip) {
	int pino = search(mip, "..");
	if (mip->ino == pino) {
		//v_printf("pwd: is root\n");
		printf("/");
	} else {
		rpwd(mip);
	}
	return 0;
}

int pwd(MINODE *mip) {
	pwd_inline(mip);
	printf("\n");
	return 0;
}

/*
int rpwd(MINODE *mip) {
	int pino = get_child_ino(mip, "..");//getino(&mip->dev, pathname);
	if (!pino) {
		printf("pwd: error: couldn't find parent\n");
	}
	v_printf("mip->ino=%d ; pino=%d\n",mip->ino,pino);
	if (pino != mip->ino) {
		MINODE *pmip = iget(mip->dev, pino);
		INODE *pip = & pmip->INODE;
		char name[MAX_LEN_DIR_NAME];
		if (getChildNameFromIno(pmip, mip->ino, name) != 0) {
			//the inode isn't in the parent's directory entries?!
			v_printf("pwd: error: can't find child?!\n");
			return -1;
		}
		v_printf("name: \"%s\"\n",name);
		rpwd(pmip);

		printf("/%s",name);
		iput(pmip);
	} else {
	}
	return 0;
}

int pwd(MINODE *mip) {
	if (mip == root) {
		v_printf("pwd: is root\n");
		printf("/\n");
		return 0;
	}

	rpwd(mip);
	printf("\n");
	return 0;
}*/

/*
int rpwd(MINODE *mip) {
	char pathname[3] = "..";
	int pino = getino(&mip->dev, pathname);//get_child_ino(mip, "..");//getino(&mip->dev, pathname);
	if (!pino) {
		printf("pwd: error: couldn't find parent\n");
	}
	//v_printf("mip->ino=%d ; pino=%d\n",mip->ino,pino);
	if (pino != mip->ino) {
		MINODE *pmip = iget(mip->dev, pino);
		INODE *pip = & pmip->INODE;
		char name[MAX_LEN_DIR_NAME];
		if (getChildNameFromIno(pmip, mip->ino, name) != 0) {
			//the inode isn't in the parent's directory entries?!
			v_printf("pwd: error: can't find child?!\n");
			return -1;
		}
		//v_printf("name: \"%s\"\n",name);
		running->cwd = pmip;
		rpwd(pmip);

		printf("/%s",name);
		iput(pmip);
	} else {
	}
	return 0;
}

int pwd_inline(MINODE *mip) {
	if (mip == root) {
		//v_printf("pwd: is root\n");
		printf("/");
		return 0;
	}
	MINODE *origcwd = running->cwd;
	rpwd(mip);
	running->cwd = origcwd;
	return 0;
}

int pwd(MINODE *mip) {
	pwd_inline(mip);
	printf("\n");
	return 0;
}*/
