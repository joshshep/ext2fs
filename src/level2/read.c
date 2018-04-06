#include "../../include/level2cmd.h"

//takes a logical bno (lblk) and returns an actual/usable bno
//note: if lbno is greater than the available bnos, it will allocate then using 
// bdealloc
int zero_blk(int dev, int bno) {
  char buf[BLKSIZE];
  memset(buf,0,BLKSIZE);
  put_block(dev, bno, buf);
  return 0;
}

// get the bno from dev. If bno is zero, allocate a new block and assign it
// note you still need to put_block() (if not direct)
int access_bno_safe(int dev, unsigned int *bno) {
  if (!*bno) {
    *bno = balloc(dev);
    zero_blk(dev, *bno); // necessary
    return 1;
  }
  return 0;
}

// convert a logical block into a physical one (that's usable)
int get_physical_bno(MINODE *mip,int lbno) {
  int bno;
  INODE *ip = & mip->INODE;
  
  
  if (lbno < NUM_DIRECT_BLK) {
    //direct blocks
    access_bno_safe(mip->dev, &ip->i_block[lbno]);
    
    bno = ip->i_block[lbno];
    
    //too verbose
    //v_printf("get_phys_bno: direct block (i_block[%d]==%d)\n", lbno, bno);
  } else if (lbno >= NUM_DIRECT_BLK && 
            lbno < NUM_INDIRECT_BLK + NUM_DIRECT_BLK) {
    // indirect blocks TODO
    int blks_in_indirect = lbno - NUM_DIRECT_BLK;
    char buf[BLKSIZE];
    
    access_bno_safe(mip->dev, &ip->i_block[INDIRECT_BLK]);
    int indirect_bno = ip->i_block[INDIRECT_BLK];
    
    get_block(mip->dev, indirect_bno, buf);
    unsigned int *bnos = (unsigned int *) buf;
    if (access_bno_safe(mip->dev, &bnos[blks_in_indirect])) {
      put_block(mip->dev, indirect_bno, buf);
    }
    bno = bnos[blks_in_indirect];
    //too verbose
    //v_printf("read: INdirect block (i_block[%d]==%d)\n", INDIRECT_BLK, indirect_bno);
    //v_printf("read: INdirect block (indirect_blk[%d]==%d)\n", blks_in_indirect, bno);
  } else {
    // double indirect blocks TODO
    int blks_in_d_indirect = lbno - NUM_INDIRECT_BLK - NUM_DIRECT_BLK;
    char buf[BLKSIZE];
    access_bno_safe(mip->dev, &ip->i_block[DOUBLE_INDIRECT_BLK]);
    int double_indirect_bno = ip->i_block[DOUBLE_INDIRECT_BLK];
    
    get_block(mip->dev, double_indirect_bno, buf);
    unsigned int *indirect_bnos = (unsigned int *) buf;
    
    if (access_bno_safe(mip->dev, &indirect_bnos[blks_in_d_indirect / NUM_INDIRECT_BLK])) {
      put_block(mip->dev, double_indirect_bno, buf);
    }
    
    int indirect_bno = indirect_bnos[blks_in_d_indirect / NUM_INDIRECT_BLK];
    get_block(mip->dev, indirect_bno, buf);
    unsigned int *bnos = (unsigned int *) buf;
    
    if (access_bno_safe(mip->dev, &bnos[blks_in_d_indirect % NUM_INDIRECT_BLK])) {
      put_block(mip->dev, indirect_bno, buf);
    }
    bno = bnos[blks_in_d_indirect % NUM_INDIRECT_BLK];
    /*
    // too verbose
    v_printf("read: DOUBLE indirect block (i_block[%d]==%d)\n", 
              DOUBLE_INDIRECT_BLK, double_indirect_bno);
    v_printf("read: DOUBLE indirect block (d_indirect_blk[%d]==%d)\n", 
              blks_in_d_indirect / NUM_INDIRECT_BLK, indirect_bno);
    v_printf("read: DOUBLE indirect block (indirect_blk[%d]==%d)\n", 
              blks_in_d_indirect % NUM_INDIRECT_BLK, bno);
    */
  }
  return bno;
}

//returns the *actual* number of bytes read
/* we have 2 limitation to how many bytes we read from fd into buf
  - the size of the file
  - nbytes (function parameter)
So every time we read and copy a block we must check 3 values: the two from
above and how many bytes are remaining in the buffer
*/
int myread(int fd, char *buf, int nbytes) {
  //v_printf("myread: fd=%d buf=%p nbytes=%d\n",fd,(void *) buf, nbytes);
  OFT *oftp = running->fd[fd];
  MINODE *mip = oftp->inodeptr;
  //v_printf("read: ino=%d\n",mip->ino);
  INODE *ip = & mip->INODE;
  
  int fsize = ip->i_size;
  int cnt = 0;
  int avail = fsize - oftp->offset;
  char *cq = buf;
  //v_printf("read: (before loop) fsize=%d avail=%d\n",fsize, avail);
  while (nbytes && avail) {
    int lbno = oftp->offset / BLKSIZE;
    int startByte = oftp->offset  % BLKSIZE;
    int bno;

    bno = get_physical_bno(mip, lbno);
    
    char readbuf[BLKSIZE];
    get_block(mip->dev, bno, readbuf);
    
    /* copy from startByte to buf[ ], at most remain bytes in this block */
    char *cp = readbuf + startByte;   
    int remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
    
    //v_printf("read: remain=%d ; nbytes=%d ; avail=%d\n",remain,nbytes,avail);
    
    // number of bytes to read in *this* block
    int bytes2read = MIN(remain, nbytes);
    bytes2read = MIN(bytes2read, avail);
    //     dest, src, size
    memcpy(cq, cp, bytes2read);
    
    cnt += bytes2read;
    oftp->offset += bytes2read;
    nbytes -= bytes2read;
    cq += bytes2read;
    avail -= bytes2read;
 
  }
  //printf("myread: read %d char from file descriptor %d\n", cnt, fd);  
  return cnt;   // count is the actual number of bytes read
}


int read_file(int fd, int nbytes) {
  //TODO
  //verify fd is indeed opened for RD or RW
  return myread(fd, buf, nbytes);
}

