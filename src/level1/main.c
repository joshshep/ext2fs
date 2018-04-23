#include "../../include/level1cmd.h"
#include "../../include/level2cmd.h"
#include "../../include/level3cmd.h"

typedef struct funcNamePair {

	char name[MAX_LEN_ARG];
	int (*func) (int argc, char **args);

} FuncNamePair;

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



	// this is just a hack to allow for alphabetizing arguments
	#define NUM_FUNC_NAME_PAIRS (100)
	FuncNamePair funcNamePairs[NUM_FUNC_NAME_PAIRS] = {
		{"menu",     cmd_print_menu},
		{"help",     cmd_print_menu},
		{"ls",       cmd_ls},
		{"ll",       cmd_ls},
		{"cd",       cmd_cd},
		{"pwd",      cmd_pwd},
		{"quit",     cmd_quit},
		{"exit",     cmd_quit},
		{"mkdir",    cmd_mkdir},
		{"rmdir",    cmd_rmdir},
		{"creat",    cmd_creat},
		{"link",     cmd_link},
		{"unlink",   cmd_unlink},
		{"symlink",  cmd_symlink},
		{"readlink", cmd_readlink},
		{"chmod",    cmd_chmod},
		{"touch",    cmd_touch},
		{"stat",     cmd_stat},
		/////////////////////////////////////
		// level 2
		{"pfd", cmd_pfd},
		{"cat", cmd_cat},
		{"mv",  cmd_mv},
		{"cp",  cmd_cp},
		////////////////////////////////////
		// level 3
		{"mount",  cmd_mount},
		{"umount", cmd_umount}
	};
	/*
	printf("Function names:\n");
	for (int i=0; i<NUM_FUNC_NAME_PAIRS; ++i) {
		printf("%s\n", funcNamePairs[i].name);
	}
	*/

	// allocate the memory for our arguments
	// TODO: should we malloc arguments individually and every loop?
	//       i.e. instead of allocating them here in a for loop, allocate them in
	//       parseLine()
	char** args = (char**) malloc(MAX_ARGS);
	for (int iarg=0; iarg<MAX_ARGS; ++iarg) {
		args[iarg] = (char*) malloc(MAX_LEN_ARG);
	}

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
		// TODO: portably flush input
		// get rid of the newline char
		line[strcspn(line, "\n")] = 0;
		// first element in args should be the command
		int argc;
		parseLine(line, &argc, args);
		printStrings(argc, args);
		//parseLine(line, cmd, arg);

		// run the function that corresponds to the input string command
		if (!argc) {
			continue;
		}
		// TODO: sort then binary search (or we could hash)
		for (int icmd=0; icmd< NUM_FUNC_NAME_PAIRS; ++icmd) {
			if (funcNamePairs[icmd].name[0] == 0) {
				break;
			}
			if (strcmp(funcNamePairs[icmd].name, args[0]) == 0) {
				funcNamePairs[icmd].func(argc, args);
				break;
			}
		}
	}
	for (int iarg=0; iarg<MAX_ARGS; ++iarg) {
		free(args[iarg]);
	}
	free(args);

	return 0;
}
