#include "../../include/level1cmd.h"

int getStartDev(const char *pathname) {
	int dev;
	if (pathname[0] == '/') {
		v_printf("starting at root\n");
		dev = root->dev;
	} else {
		v_printf("not starting at root\n");
		dev = running->cwd->dev;
	}
	return dev;
}

int printSuper(SUPER *sp) {
	v_printf("********** SUPER block ************\n");
	v_printf("s_magic = %x\n", sp->s_magic);

	v_printf("s_inodes_count = %d\n", sp->s_inodes_count);
	v_printf("s_blocks_count = %d\n", sp->s_blocks_count);

	v_printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
	v_printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
	v_printf("s_first_data_block = %d\n", sp->s_first_data_block);


	v_printf("s_log_block_size = %d\n", sp->s_log_block_size);
	// v_printf("s_log_frag_size = %d\n", sp->s_log_frag_size);

	v_printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
	// v_printf("s_frags_per_group = %d\n", sp->s_frags_per_group);
	v_printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);


	v_printf("s_mnt_count = %d\n", sp->s_mnt_count);
	v_printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

	v_printf("s_magic = %x\n", sp->s_magic);

	//TODO use strftime instead
	v_printf("s_mtime = %s\n", strtok(ctime((time_t *) &sp->s_mtime),"\n"));
	v_printf("s_wtime = %s\n", strtok(ctime((time_t *)&sp->s_wtime),"\n"));
	v_printf("\n");
	return 0;
}

int printGd(GD *gp) {
	v_printf("************* group 0 info ****************\n");
	v_printf("Blocks bitmap block %10d\n",gp->bg_block_bitmap);
	v_printf("Inodes bitmap block %10d\n",gp->bg_inode_bitmap);
	v_printf("Inodes table block  %10d\n",gp->bg_inode_table);
	v_printf("Free blocks count   %10d\n",gp->bg_free_blocks_count);
	v_printf("Free inodes count   %10d\n",gp->bg_free_inodes_count);
	v_printf("Directories count   %10d\n",gp->bg_used_dirs_count);
	v_printf("inodes_start        %10d\n",gp->bg_inode_table);
	v_printf("\n");
	return 0;
}



int init() {
	/*
	WRITE C code to initialize

	(1).All minode's refCount=0;
	(2).proc[0]'s pid=1, uid=0, cwd=0, fd[ ]=0;
	proc[1]'s pid=2, uid=1, cwd=0, fd[ ]=0;
	*/
	for (int i=0; i<NMINODE; i++) {
		minode[i].refCount = 0;
	}
	for (int i=0; i<NOFT; i++) {
		oft[i].refCount = 0;
	}
	proc[0].pid =  1;
	proc[0].uid =  0;
	proc[0].cwd =  0;
	proc[0].fd[0] =   0; // DEBUG

	proc[1].pid =  2;
	proc[1].uid =  1;
	proc[1].cwd =  0;
	proc[1].fd[0] =   0;

	return 0;
}

// load root INODE and set root pointer to it
int mount_root(int dev) {
	v_printf("mount_root()\n");
	root = iget(dev, 2);
	return 0;
}


int cmd_quit(int argc, char** args) {
	v_printf("quitting prog\n");
	v_printf("writing inodes back to disk\n");
	for (int i=0; i<NMINODE; i++) {
		if (minode[i].refCount > 0 && minode[i].dirty) {
			minode[i].refCount = 1;
			iput(minode);
		}
	}
	exit(0);  // terminate program
	return 0;
}

int pause() {
	printf("Press any key to continue...");
	getchar();
	return 0;
}

int printStrings(int argc, char** strings){
	for (int i=0; i<argc; ++i) {
		printf("\"%s\"\n",strings[i]);
	}
	return 0;
}

/*
TODO: dynamic allocation?
For now, the most command arguments is MAX_ARGS - 1 (minus 1 because of the command itself)
and the longest command argement is MAX_LEN_ARG
Note: line *will* be edited
*/
int parseLine(char* line, int* argc, char** args) {
	char* tok;
	int iarg = 0;
	tok = strtok(line, " ");
	while( tok && iarg < MAX_ARGS) {
		args[iarg][0] = 0;
		strncat(args[iarg], tok, MAX_LEN_ARG - 1);
		tok = strtok(NULL, " ");
		++iarg;
	}
	*argc = iarg;
	return 0;
}


int get_ideal_len(int len) {
	return 4 * ((8 + len + 3) / 4);
}

int enter_name(MINODE *pmip, int myino, const char *myname, int file_type) {
	v_printf("enter_name: myname='%s'\n",myname);
	INODE *ip = & pmip->INODE;
	int new_rec_len = get_ideal_len(strlen(myname));

	int blk_index;
	for (blk_index=0; blk_index < 12 && ip->i_block[blk_index]; blk_index++) {
		int bno = ip->i_block[blk_index];
		v_printf("i_block[%d] = %d\n",blk_index,bno);
		get_block(pmip->dev, bno, dbuf);

		dp = (DIR *) dbuf;
		char * cp = dbuf;


		// get last entry
		while (cp + dp->rec_len < & dbuf[BLKSIZE]) {
			cp += dp->rec_len;
			dp = (DIR *) cp;
		}

		int last_rec_ideal_len = get_ideal_len(dp->name_len);
		int space_remaining = dp->rec_len - last_rec_ideal_len;
		v_printf("new_rec_len: %d ; space_remaining: %d\n",new_rec_len,
		                                                   space_remaining);
		if (space_remaining >= new_rec_len) {
			//there's space in this block so we can add it here
			v_printf("enter_name(): we can add an entry in this block!\n");
			dp->rec_len = last_rec_ideal_len;

			cp+= dp->rec_len;
			dp = (DIR *) cp;

			dp->inode = myino;
			dp->rec_len = dbuf + BLKSIZE - cp;
			dp->name_len = strlen(myname);
			dp->file_type = file_type; //EXT2_FT_DIR;
			strcpy(dp->name, myname);

			put_block(pmip->dev, bno, dbuf);
			return 0;
		}


	}

	// no space in any of the allocated blocks!
	// we must allocate a new block
	v_printf("enter_name(): we must allocate a new block\n");

	int bno = balloc(pmip->dev);
	ip->i_block[blk_index] = bno;

	ip->i_size += BLKSIZE;
	pmip->dirty = 1;
	get_block(pmip->dev, bno, dbuf);

	char *cp = dbuf;
	dp = (DIR *) cp;

	dp->inode = myino;
	dp->rec_len = BLKSIZE;
	dp->name_len = strlen(myname);
	dp->file_type = file_type; //EXT2_FT_DIR;
	strcpy(dp->name, myname);

	put_block(pmip->dev, bno, dbuf);

	return 0;
}

//helper functions for truncate()
int bdealloc_indirect(int dev, int indirect_bno) {

	char buf[BLKSIZE];
	get_block(dev, indirect_bno, buf);
	int *bnos = (int *) buf;
	for (int i=0; i<NUM_INDIRECT_BLK; i++) {
		int bno = bnos[i];
		if (!bno) {
			break;
		}
		bdealloc(dev, bno);
	}
	bdealloc(dev, indirect_bno);
	return 0;
}

int bdealloc_double_indirect(int dev, int double_indirect_bno) {

	char buf[BLKSIZE];
	get_block(dev, double_indirect_bno, buf);
	int *indirect_bnos = (int *) buf;
	for (int i=0; i<NUM_INDIRECT_BLK; i++) {
		int indirect_bno = indirect_bnos[i];
		if (!indirect_bno) {
			break;
		}
		bdealloc_indirect(dev, indirect_bno);
	}
	bdealloc(dev, double_indirect_bno);
	return 0;
}

int truncate(MINODE *mip) {
	/*
	1. release mip->INODE's data blocks;
		a file may have 12 direct blocks, 256 indirect blocks and 256*256
		double indirect data blocks. release them all.
	*/
	INODE *ip = & mip->INODE;
	//direct blocks
	int i;
	for (i=0; i<NUM_DIRECT_BLK; i++) {
		int bno = ip->i_block[i];
		if (!bno) {
			break;
		}
		bdealloc(mip->dev, bno);
	}
	int indirect_bno = ip->i_block[INDIRECT_BLK];

	if (indirect_bno) {
		v_printf("truncate(): indirect\n");
		//we deallocated all of the direct blocks
		//indirect blocks
		bdealloc_indirect(mip->dev, indirect_bno);

		int double_indirect_bno = ip->i_block[DOUBLE_INDIRECT_BLK];
		if (double_indirect_bno) {
			v_printf("truncate(): double indirect\n");
			bdealloc_double_indirect(mip->dev, double_indirect_bno);
		}
	}

	ip->i_size = 0;
	mip->dirty = 1;
	return 0;
}
