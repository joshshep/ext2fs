#include "../../include/level1cmd.h"
#include "../../include/level2cmd.h"
#include "../../include/level3cmd.h"

typedef struct funcNamePair {

	char name[MAX_LINE];
	int (*func) (char **args);

} FuncNamePair;
int testFunc(char **args)
{
	if (!args)
		printf("no args\n");
	else
		printf("there are args\n");
	return 0;
}
int testFunc2(char **args)
{
	if (!args)
		printf("no args2\n");
	else
		printf("there are args2\n");
	return 0;
}
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

	char arg1[MAX_LINE];
	char arg2[MAX_LINE];
	char line[MAX_LINE];
	char cmd[MAX_LINE];

	// this is just a hack to allow for alphabetizing arguments
	#define NUM_FUNC_NAME_PAIRS (100)
	FuncNamePair funcNamePairs[NUM_FUNC_NAME_PAIRS] = {
		{"menu",     print_menu},
		{"help",     print_menu},
		{"ls",       ls},
		{"ll",       ls},
		{"cd",       mychdir},
		{"pwd",      pwd},
		{"quit",     quit},
		{"exit",     quit},
		{"mkdir",    make_dir},
		{"rmdir",    myrmdir},
		{"creat",    creat_file},
		{"link",     do_link},
		{"unlink",   do_unlink},
		{"symlink",  do_symlink},
		{"readlink", do_readlink},
		{"chmod",    do_chmod},
		{"touch",    do_touch},
		{"stat",     do_stat},
		/////////////////////////////////////
		// level 2
		{"pfd", pfd},
		{"cat", mycat},
		{"mv",  mv},
		{"cp",  cp},
		////////////////////////////////////
		// level 3
		{"mount",  do_mount},
		{"umount", umount}
	};
	/*
	printf("Function names:\n");
	for (int i=0; i<NUM_FUNC_NAME_PAIRS; ++i) {
		printf("%s\n", funcNamePairs[i].name);
	}
	*/

	char args[MAX_ARGS][MAX_LEN_ARG];
	args[0];
	args[0][0];
	while(1) { // command processing loop

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
		// TODO: portably flush input
		// get rid of the newline char
		line[strcspn(line, "\n")] = 0;
		// first element in args should be the command
		int argc;
		parseLine(line, &argc, args);
		printStrings(argc, args);
		//parseLine(line, cmd, arg);

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
