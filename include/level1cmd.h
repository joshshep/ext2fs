#ifndef LEVEL1CMD_H
#define LEVEL1CMD_H



#include "type.h"
#include "globals.h"




// function prototypes

int search (MINODE *mip, const char *name);

int get_block(int dev, int blk, char *buf);

int printSuper(SUPER * sp);

int printGd(GD *gp);

int init();

int mount_root(int dev);

int ls(char *pathname);

int mychdir(char * pathname);

int quit();

int make_dir(const char * pathname);

int myrmdir(char * pathname);

int creat_file(const char *pathname);

int do_link(char * oldFile, char * newFile);

int do_unlink(char * pathname);

int do_symlink(char * oldFile, char * newFile);

int do_readlink(char *pathname);

int getino(int *dev, char * pathname);

MINODE * iget(int dev, int ino);

int iput(MINODE * mip);


int get_block(int fd, int blk, char * buf);

int put_block(int fd, int blk, char * buf);


int parseLine(char* line, int* argc, char args[MAX_ARGS][MAX_LEN_ARG]);

int pwd(MINODE *mip);

int ialloc(int dev);

int balloc(int dev);

int idealloc(int dev, int ino);

int bdealloc(int dev, int bno);


int get_ideal_len(int len);


int enter_name(MINODE *pmip, int ino, const char *name, int file_type);

int pwd_inline(MINODE *mip);

int truncate(MINODE *mip);

int rm_child_entry(MINODE *pmip, char *name);

int do_chmod(char *permissions, char *filename);

int do_touch(char *filename);

int do_stat(char *pathname);

int getStartDev(const char *pathname);

int printStrings(int argc, char strings[MAX_ARGS][MAX_LEN_ARG]);
#endif
