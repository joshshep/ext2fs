#include "../../include/level1cmd.h"
#include "../../include/level2cmd.h"
#include "../../include/level3cmd.h"


// run as a.out [diskname]
int main(int argc, char *argv[ ]) {
	char * diskname = DFLT_DISK_PATH;
	if (argc > 1) {
		diskname = argv[1];
    v_printf("Mounting disk: %s\n",diskname);
	} else {
    v_printf("No disk provided... using default: %s\n", diskname);
  }
	int dev;
	if ((dev = open(diskname, O_RDWR)) < 0) {
		printf("open %s failed\n", diskname);
		exit(1);
	}

	v_printf("dev=%d\n",dev);

	get_block(dev, 1, buf);
	sp = (SUPER *) buf;

	//check EXT2 FS; if not EXT2 FS: exit
	v_printf("check ext2 FS on %s ", diskname);
	if (sp->s_magic != EXT2FS_MAGIC) {
		printf("\n%x %s is not an ext2 FS\n", sp->s_magic, diskname);
		exit(2);
	} else {
		v_printf("YES\n");
	}

	//print SUPER block info
	printSuper(sp);
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;

	//read GD block at (first_data_block + 1)
	get_block(dev, 2, buf);
	gp = (GD *) buf;
	iblock = gp->bg_inode_table;
	bmap = gp->bg_block_bitmap;
	imap = gp->bg_inode_bitmap;

	//Read GD block to get bmap, imap, iblock (and print their values)
	printGd(gp);


	init();

	mount_root(dev);
	v_printf("creating P0 as running process\n");

	running = &proc[0];

	// set running cwd to point at / in mem
	running->cwd = root;

	print_menu();

	root_mnt.bmap = bmap;
	root_mnt.imap = imap;
	root_mnt.iblock = iblock;
	root_mnt.mounted_inode = root;


	secret_root = *root;

	while(1) { // command processing loop
		char arg1[MAX_LINE];
		char arg2[MAX_LINE];
		char line[MAX_LINE];
		char cmd[MAX_LINE];
		line[0] = 0;
		cmd[0] = 0;
		arg1[0] = 0;
		arg2[0] = 0;
		print_prompt();

		/*
		WRITE C code to do these:

		use fgets() to get user inputs into line[128]
		kill the \r at end
		if (line is an empty string) // if user entered only \r
		continue;
		*/

		fgets(line, MAX_LINE, stdin);
		sscanf(line, " %s %s %s ", cmd, arg1, arg2);
		//parseLine(line, cmd, arg);

		/*
		// temp hack to avoid seaching for ""
		if (strcmp(arg,"") == 0) {
			strcpy(arg,".");
		}
		*/

		//Use sscanf() to extract cmd[ ] and pathname[] from line[128]
		if (cmd[0] != 0) {
			v_printf("cmd=\"%s\"",cmd);
			if (arg1[0] != 0) {
				v_printf(" arg1=\"%s\"",arg1);
				if (arg2[0] != 0) {
					v_printf(" arg2=\"%s\"",arg2);
				}
			}
			v_printf("\n");
		}


		// execute the cmd
		if (strcmp(cmd, "")==0) {
			continue;
		} else if (strcmp(cmd, "menu")==0 ||
		           strcmp(cmd, "help")==0) {
			print_menu();
		} else if (strcmp(cmd, "ls")==0 ||
		           strcmp(cmd, "ll")==0) {
			ls(arg1);
		} else if (strcmp(cmd, "cd")==0) {
			mychdir(arg1);
		} else if (strcmp(cmd, "pwd")==0) {
			pwd(running->cwd);
		} else if (strcmp(cmd, "quit")==0 ||
		           strcmp(cmd, "exit")==0) {
			quit();
		} else if (strcmp(cmd, "mkdir")==0) {
			make_dir(arg1);
		} else if (strcmp(cmd, "rmdir")==0) {
			myrmdir(arg1);
		} else if (strcmp(cmd, "creat")==0) {
			creat_file(arg1);
		} else if (strcmp(cmd, "link")==0) {
			do_link(arg1, arg2);
		} else if (strcmp(cmd, "unlink")==0) {
			do_unlink(arg1);
		} else if (strcmp(cmd, "symlink")==0) {
			do_symlink(arg1,arg2);
		} else if (strcmp(cmd, "readlink")==0) {
			do_readlink(arg1);
		} else if (strcmp(cmd, "chmod")==0) {
			do_chmod(arg1,arg2);
		} else if (strcmp(cmd, "touch")==0) {
			do_touch(arg1);
		} else if (strcmp(cmd, "stat")==0) {
			do_stat(arg1);
		}
		/////////////////////////////////////
		// level 2
		else if (  strcmp(cmd, "pfd")==0) {
			pfd();
		} else if (strcmp(cmd, "cat")==0) {
			mycat(arg1);
		} else if (strcmp(cmd, "open")==0) {
			//do_open(arg1, arg2); // open for read
		} else if (strcmp(cmd, "read")==0) {
			//do_read(arg1,arg2);
		} else if (strcmp(cmd, "write")==0) {
			//do_write(arg1,arg2);
		} else if (strcmp(cmd, "close")==0) {
			//close_file(arg1);
			//do_close(arg1);
		} else if (strcmp(cmd, "lseek")==0) {
			//do_lseek(arg1,arg2);
		} else if (strcmp(cmd, "mv")==0) {
			mv(arg1,arg2);
		} else if (strcmp(cmd, "cp")==0) {
			cp(arg1,arg2);
		}
		////////////////////////////////////
		// level 3
		else if (  strcmp(cmd, "mount")==0) {
			do_mount(arg1,arg2);
		} else if (strcmp(cmd, "umount")==0) {
			umount(arg1);
		} else {
			printf("%s: command not found\n",cmd);
		}

	}

	return 0;
}
