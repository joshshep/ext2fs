#ifndef LEVEL2CMD_H
#define LEVEL2CMD_H



#include "type.h"
#include "globals.h"
#include "level1cmd.h"

// function prototypes
int open_file(char *pathname, int mode);

int myread(int fd, char *buf, int nbytes);

int close_file(int fd);

int my_lseek(int fd, int pos);

int pfd();

int print_menu();

int print_prompt();

int mycat(char *pathname);

int get_physical_bno(MINODE *mip,int lbno);

int mywrite(int fd, char *buf, int nbytes);

int write_file(int fd, char *str);


int cp(char *src, char *dst);

int mv(char *src, char *dst);

#endif
