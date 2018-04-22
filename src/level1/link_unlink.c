#include "../../include/level1cmd.h"

int do_link(char *oldFname, char *newFname) {

	int dev;
	if (oldFname[0] == '/') {
		v_printf("link: starting at root\n");
		dev = root->dev;
	} else {
		v_printf("link: not starting at root\n");
		dev = running->cwd->dev;
	}

	int ino = getino(&dev, oldFname);
	v_printf("link(): oldfname ino=%d\n",ino);
	MINODE *mip = iget(dev, ino);

	char *dname = dirname(newFname);
	char *new_bname = basename(newFname);
	v_printf("link(): (new file) dname=\"%s\" ; bname=\"%s\"\n", dname, new_bname);


	int new_pino = getino(&dev, dname);
	MINODE *new_pmip = iget(dev, new_pino);

	if (new_pmip->dev != mip->dev) {
		printf("link: error cannot link across devices\n");
	}

	enter_name(new_pmip, ino, new_bname, EXT2_FT_REG_FILE);

	INODE * ip = & mip->INODE;
	ip->i_links_count++;
	mip->dirty = 1;

	INODE * new_pip = & new_pmip->INODE;
	new_pip->i_atime = time(0L);
	new_pmip->dirty = 1;

	iput(new_pmip);

	iput(mip);
	return 0;
}


int do_unlink(char *pathname) {
	int dev;
	if (pathname[0] == '/') {
		v_printf("unlink: starting at root\n");
		dev = root->dev;
	} else {
		v_printf("unlink: not starting at root\n");
		dev = running->cwd->dev;
	}
	int ino = getino(&dev, pathname);
	if (!ino) {
		printf("unlink: can't find '%s'\n",pathname);
		return -1;
	}
	v_printf("found ino from pathname: %d\n",ino);
	MINODE * mip = iget(dev, ino);
	INODE * ip = & mip->INODE;
	if (S_ISDIR(ip->i_mode)) {
		printf("unlink: error: is a DIR file\n");
		return -1;
	} else if (S_ISREG(ip->i_mode)){
		v_printf("is a REG file\n");
	} else if (S_ISLNK(ip->i_mode)) {
		v_printf("is a LNK file\n");
	} else {
		printf("unlink: error: invalid file mode. Must be REG or LNK\n");
		return -2;
	}

	if (!(ip->i_mode & S_IWUSR) || ip->i_uid != running->uid ) {
		printf("unlink: error: the permissions to do not allow that\n");
		iput(mip);
		return -1;
	}

	--ip->i_links_count;
	mip->dirty = 1;
	if (ip->i_links_count <= 0) {
		truncate(mip);
		idealloc(mip->dev, mip->ino);
	}
	iput(mip);

	char *dname = dirname(pathname);
	char *bname = basename(pathname);
	int pino = getino(&dev, dname);
	if (!pino) {
		v_printf("unlink: error: couldn't find parent ino\n");
		return -3;
	}
	MINODE *pmip = iget(dev, pino);
	rm_child_entry(pmip,bname);
	pmip->dirty = 1;
	iput(pmip);
	return 0;
}
