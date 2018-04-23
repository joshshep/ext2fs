#include "../../include/level1cmd.h"

int cmd_print_menu(int argc, char** args) {
	
	printf("~~~~~~~~~commands~~~~~~~~~~\n");
	printf("level 1: menu|ls|cd|pwd|quit|mkdir|rmdir|creat|link|unlink|symlink|readlink\n");
	printf("         touch|chmod\n");
	printf("level 2: cp|mv|pfd\n");
	printf("level 3: mount|umount\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	return 0;
}



int print_prompt() {

	printf("joshshell:");

	pwd_inline(running->cwd);

	printf("$ ");

	return 0;
}
