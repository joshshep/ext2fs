#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "type.h"

MINODE minode[NMINODE];        // global minode[ ] array
MINODE *root;                  // root pointer: the /
PROC   proc[NPROC], *running;  // PROC; using only proc[0]
OFT    oft[NOFT];
MOUNT  mount[NMOUNT];
// TODO change BLKSIZE from a macro  (defined at runtime instead)

//int fd, dev;                               // file descriptor or dev
int nblocks, ninodes, bmap, imap, iblock;  // FS constants

//#include "iget_iput_getino.c"  // YOUR iget_iput_getino.c file with
                               // get_block/put_block, tst/set/clr bit functions
//
char line[128], cmd[64], pathname[64];
char buf[BLKSIZE];              // define buf1[ ], buf2[ ], etc. as you need
char dbuf[BLKSIZE];
char sbuf[BLKSIZE];
MINODE secret_root;
MOUNT root_mnt;

#define MAX_LEN_DIR_NAME (128)
#define MAX_LINE (128)
#define MAX_ARGS (64)
#define MAX_LEN_ARG (64)


#define NUM_DIRECT_BLK (12)
#define INDIRECT_BLK (NUM_DIRECT_BLK)
#define NUM_INDIRECT_BLK (BLKSIZE/sizeof(int))
#define DOUBLE_INDIRECT_BLK (INDIRECT_BLK + 1)

// each indirect block has 256 block numbers in it
// each of those block numbers corresponds to a block of size BLKSIZE
#define INDIRECT_BLK_SIZE (NUM_INDIRECT_BLK*BLKSIZE)

//each direct block has BLKSIZE bytes. there are NUM_DIRECT_BLK's
#define DIRECT_BLK_SIZE (NUM_DIRECT_BLK*BLKSIZE)

//http://stackoverflow.com/a/1644898
//#define VERBOSE
#ifdef VERBOSE
#  define v_printf(...) \
          do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
#  define v_printf(...)
#endif

#ifdef DEBUG
#  define d_printf(...) \
          do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
#  define d_printf(...)
#endif

#define MIN(x,y) ((x < y) ? (x): (y))


#define EXT2FS_MAGIC (0xEF53)

#define DFLT_DISK_PATH ("disks/dirs.img")
#endif
