#ifndef LEVEL2CMD_H
#define LEVEL2CMD_H



#include "type.h"
#include "globals.h"
#include "level1cmd.h"

//commands
int cmd_pfd(int argc, char** args);
int cmd_cat(int argc, char** args);
int cmd_mv(int argc, char** args);
int cmd_cp(int argc, char** args);


// function prototypes
int open_file(char *pathname, int mode);

int myread(int fd, char *buf, int nbytes);

int close_file(int fd);

int my_lseek(int fd, int pos);

int print_prompt();

int get_physical_bno(MINODE *mip,int lbno);

int mywrite(int fd, char *buf, int nbytes);

int write_file(int fd, char *str);

#endif
