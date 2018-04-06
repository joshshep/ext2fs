#ifndef LEVEL3CMD_H
#define LEVEL3CMD_H



#include "type.h"
#include "globals.h"
#include "level2cmd.h"

// function prototypes

int do_mount(char *filesys, char *mnt_pnt);

int umount(char *filesys);

#endif
