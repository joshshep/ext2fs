#include "../../include/level1cmd.h"

int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
  return 0;
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
  return 0;
}

int get_block(int fd, int blk, char *buf)
{
  d_printf("get_block: fd=%d blk=%d buf=%p\n", fd, blk, (void *) buf);
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
  return 0;
}

int put_block(int fd, int blk, char *buf)
{
  d_printf("put_block: fd=%d blk=%d buf=%p\n", fd, blk, (void *) buf);
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
  return 0;
}

/*
int get_inode(int fd, int ino, INODE *inode) {
  d_printf("get_inode: fd=%d ino=%d INODE=%p\n", fd, ino, (void *) inode);

  int blk = (ino - 1) / INODES_PER_BLOCK;
  int offset = (ino - 1) % INODES_PER_BLOCK;
  d_printf("blk=%d  offset=%d\n", blk, offset);
  d_printf("group 0 inodes_start = %d\n",iblock);
  blk += iblock;
  d_printf("blk=%d  offset=%d\n", blk, offset);

  lseek(fd, (long)blk*BLKSIZE + offset*sizeof(INODE), 0);
  read(fd, inode, sizeof(INODE));

  return 0;
}
*/

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
  return 0;
}
int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  // dec free blocks count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
  return 0;
}

int balloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, bmap, buf);

  for (i=0; i < nblocks; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeBlocks(dev);

       put_block(dev, bmap, buf);

       return i+1;
    }
  }
  return 0;
}

int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

int bdealloc(int dev, int blk) {
    v_printf("bdealloc(): dev=%d ; blk=%d\n", dev, blk);
    char mybuf[BLKSIZE];

    get_block(dev, bmap, mybuf);
    clr_bit(mybuf, blk);
    put_block(dev, bmap, mybuf);
    return 0;
}

int idealloc(int dev, int ino) {
    v_printf("idealloc(): dev=%d ; ino=%d\n", dev, ino);
    char mybuf[BLKSIZE];

    get_block(dev, imap, mybuf);
    clr_bit(mybuf, ino);
    put_block(dev, imap, mybuf);
    return 0;
}


// load INODE at (dev,ino) into a minode[]; return mip->minode[]
MINODE *iget(int dev, int ino)
{
  int i, blk, disp;
  char buf[BLKSIZE];
  MINODE *mip;
  INODE *ip;
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //v_printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //v_printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
       mip->mountptr = 0;
       mip->dirty = mip->mounted = 0;
       // get INODE of ino into buf[ ]
       blk  = (ino-1)/INODES_PER_BLOCK + iblock;  // iblock = Inodes start block #
       disp = (ino-1) % INODES_PER_BLOCK;
       //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);
       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }
  printf("PANIC: no more free minodes\n");
  return 0;
}

int iput(MINODE *mip)  // dispose of a minode[] pointed by mip
{
  mip->refCount--;

  if (mip->refCount > 0 || !mip->dirty) {
    return -1;
  }
  
  INODE * ip = & mip->INODE;
  ip->i_mtime = time(0L);
  
  
  //v_printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);
  int blk  = (mip->ino-1)/INODES_PER_BLOCK + iblock;  // iblock = Inodes start block #
  int disp = (mip->ino-1) % INODES_PER_BLOCK;
  int buf[BLKSIZE];

  get_block(mip->dev, blk, (char *) buf);

  ip = (INODE *)buf + disp;
  *ip = mip->INODE;
  put_block(mip->dev, blk, (char *) buf);

  return 0;
}


int search (MINODE *mip, const char *name)
{
  INODE *ip = & mip->INODE;
  // only search the first 12 blocks (direct)
  for (int blk_index=0; blk_index < 12 && ip->i_block[blk_index]; blk_index++) {
    v_printf("i_block[%d] = %d\n",blk_index,ip->i_block[blk_index]);
    get_block(mip->dev, ip->i_block[blk_index], dbuf);

    dp = (DIR *) dbuf;
    char * cp = dbuf;

    while (cp < & dbuf[BLKSIZE]) {
      strncpy(sbuf, dp->name, dp->name_len);
      sbuf[dp->name_len] = 0;
      v_printf("%4d %4d %4d %s\n",
              dp->inode, dp->rec_len, dp->name_len, sbuf);
      if (!strcmp(sbuf,name)) {
        // found a match!
        return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *) cp;
    }
  }
  return 0;
}

int updateGlobals(int newDev) {
  /*for(int imip=0; imip<NMINODE; ++imip) {
    minode[imip].refCount = 0;
    minode[imip].dev = 0;
    minode[imip].ino = 0;
  }*/
  if (newDev == root->dev) {
    bmap = root_mnt.bmap;
    imap = root_mnt.imap;
    iblock = root_mnt.iblock;
    return 0;
  }
  int imnt;
  for (imnt=0; imnt < NMOUNT; ++imnt) {
    if (mount[imnt].dev == newDev){
      break;
    }
  }
  if (imnt == NMOUNT) {
    v_printf("error: couldn't find a corresponding mount table entry\n");
    return -1;
  }
  bmap = mount[imnt].bmap;
  imap = mount[imnt].imap;
  iblock = mount[imnt].iblock;
  return 0;
}

// char *pathname must be be mutable ?? TODO
int getino(int *dev, char *pathname)
{
  int i, ino;
  MINODE *mip;

  if (pathname[0] != 0) {
    v_printf("getino: pathname=\"%s\"\n", pathname);
  }
  if (strcmp(pathname, "/")==0) {
    return 2;
  }

  // if it's an empty pathname return the cwd ino
  if (strcmp(pathname, "")==0 || strcmp(pathname, ".")==0) {
    return running->cwd->ino;
  }

  if (pathname[0]=='/') {
    mip = iget(*dev, 2);
    ino = 2;
  } else {
    mip = iget(running->cwd->dev, running->cwd->ino);
    ino = running->cwd->ino;
  }

  ///////////////////////////////////
  /// tokenize

  char * name[64]; //max number of directories
  int n = 0;
  //split string by delimiter http://stackoverflow.com/a/4160297
  v_printf ("Splitting string \"%s\" into tokens:\n",pathname);

  char *fname = strtok (pathname,"/");
  while (fname) {
    v_printf("fname: \"%s\"\n",fname);
    name[n] = fname;
    ++n;
    fname = strtok (NULL, "/");
  }

  for (i=0; i < n; i++){
      v_printf("===========================================\n");
      v_printf("getino: i=%d name[%d]='%s'\n", i, i, name[i]);
      
      if (ino == 2 && strcmp(name[i],"..") == 0) {
        //we're at a root and trying to get past it
        v_printf("getino: we're at the root and we are trying to get past it\n");
        if (mip != root) {
          v_printf("getino: we're also mounted\n");
          //we're mounted
          int imnt;
          for (imnt=0; imnt < NMOUNT; ++imnt) {
            if (mount[imnt].dev == mip->dev){
              break;
            }
          }
          if (imnt == NMOUNT) {
            v_printf("getino: couldn't find a corresponding mount table entry\n");
            
          } else {
            v_printf("getino: found the entry in the mount table\n");
            mip = mount[imnt].mounted_inode;
            *dev = mount[imnt].dev;
            ino = mip->ino;
            //return ino;
            //updateGlobals(*dev);
          }
        }
      }
      
      v_printf("search start\n");
      ino = search(mip, name[i]);
      v_printf("search END\n");

      if (ino==0){
         iput(mip);
         v_printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(*dev, ino);
      
      if (mip->mounted == 1) {
        v_printf("getino: crossing a mount point\n");
        // the dir is mounted 
        // don't search for it... instead jump to it like a monkey!
        MOUNT * mntPtr = mip->mountptr;
        *dev = mntPtr->dev;
        ino = 2;
        mip = iget(*dev, ino);
        //updateGlobals(*dev);
      }
      
  }
  iput(mip);
  v_printf("getino() done!\n");
  return ino;
}
