#ifndef LEVEL1CMD_H
#define LEVEL1CMD_H



#include "type.h"
#include "globals.h"




//commands
int cmd_print_menu(int argc, char** args);
int cmd_ls(int argc, char** args);
int cmd_cd(int argc, char** args);
int cmd_pwd(int argc, char** args);
int cmd_quit(int argc, char** args);
int cmd_mkdir(int argc, char** args);
int cmd_rmdir(int argc, char** args);
int cmd_creat(int argc, char** args);
int cmd_link(int argc, char** args);
int cmd_unlink(int argc, char** args);
int cmd_symlink(int argc, char** args);
int cmd_readlink(int argc, char** args);
int cmd_chmod(int argc, char** args);
int cmd_touch(int argc, char** args);
int cmd_stat(int argc, char** args);


// function prototypes

int search (MINODE *mip, const char *name);

int get_block(int dev, int blk, char *buf);

int printSuper(SUPER * sp);

int printGd(GD *gp);

int init();

int mount_root(int dev);






int getino(int *dev, char * pathname);

MINODE * iget(int dev, int ino);

int iput(MINODE * mip);


int get_block(int fd, int blk, char * buf);

int put_block(int fd, int blk, char * buf);



int ialloc(int dev);

int balloc(int dev);

int idealloc(int dev, int ino);

int bdealloc(int dev, int bno);


int get_ideal_len(int len);


int enter_name(MINODE *pmip, int ino, const char *name, int file_type);

int pwd_inline(MINODE *mip);

int truncate(MINODE *mip);

int rm_child_entry(MINODE *pmip, char *name);

int getStartDev(const char *pathname);

int printStrings(int argc, char** strings);

int parseLine(char* line, int* argc, char** args);

#endif
