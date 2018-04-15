#include "../../include/level1cmd.h"

// parent mip
// child name
int rm_child_entry(MINODE* pmip, char* name) {
  INODE* pip = & pmip->INODE; // parent inode pointer
  for (int blk_index=0; blk_index < 12 && pip->i_block[blk_index]; ++blk_index) {
    int bno = pip->i_block[blk_index];
    v_printf("i_block[%d] = %d\n", blk_index, bno);
    get_block(pmip->dev, bno, dbuf);

    dp = (DIR *) dbuf;
    DIR * prev_dp = dp;
    char * cp = dbuf;

    while (cp < & dbuf[BLKSIZE]) {
      strncpy(sbuf, dp->name, dp->name_len);
      sbuf[dp->name_len] = 0;
      v_printf("%4d %4d %4d %s\n",
              dp->inode, dp->rec_len, dp->name_len, sbuf);
      if (strcmp(name, sbuf) == 0) {
        // found a match!

        if (cp == dbuf && cp + dp->rec_len == dbuf + BLKSIZE) {
          //beginning of the block
          v_printf("rm_child(): removing from beginning (and deleting blk)\n");
          bdealloc(pmip->dev, bno);
          pip->i_size -= BLKSIZE;

          //shift 'em all down
          while(pip->i_block[blk_index+1] && blk_index < 12 - 1) {
            ++blk_index;
            char jbuf[BLKSIZE];
            get_block(pmip->dev, pip->i_block[blk_index], jbuf);
            put_block(pmip->dev, pip->i_block[blk_index - 1], jbuf);
          }

        } else if (cp + dp->rec_len == & dbuf[BLKSIZE]) {
          //end of the block
          v_printf("rm_child(): removing from the end\n");
          prev_dp->rec_len += dp->rec_len;
          put_block(pmip->dev, bno, dbuf);

        } else {
          //middle of the block
          v_printf("rm_child(): removing from front or middle (not deleting blk)\n");

          //get last entry
          DIR* last_entry = dp;
          char* cp2 = cp;
          while (cp2 + last_entry->rec_len < & dbuf[BLKSIZE]) {
            cp2 += last_entry->rec_len;
            last_entry = (DIR*) cp2;
          }
          v_printf("rmdir: last_entry old rec_len: %d ; ",last_entry->rec_len);
          last_entry->rec_len += dp->rec_len;
          v_printf("new: %d\n",last_entry->rec_len);

          char* start = cp + dp->rec_len;
          char* end = dbuf + BLKSIZE;
          memmove(cp, start, end - start);

          put_block(pmip->dev, pip->i_block[blk_index], dbuf);
        }



        pmip->dirty = 1;
        pip->i_mtime = pip->i_atime = time(0L);
        //iput(pmip);
        return 0;
      }
      prev_dp = dp;
      cp += dp->rec_len;
      dp = (DIR*) cp;
    }
  }
  return -1;
}

int hasChildren(MINODE *mip)
{
  INODE *ip = & mip->INODE;
  // only list the first 12 blocks (direct)
  for (int blk_index=0; blk_index < 12 && ip->i_block[blk_index]; blk_index++) {
    v_printf("i_block[%d] = %d\n",blk_index,ip->i_block[blk_index]);
    get_block(mip->dev, ip->i_block[blk_index], dbuf);

    dp = (DIR *) dbuf;
    char * cp = dbuf;
    while (cp < & dbuf[BLKSIZE]) {
      strncpy(sbuf, dp->name, dp->name_len);
      sbuf[dp->name_len] = 0;

      if ( strcmp(sbuf,".") != 0 && strcmp(sbuf,"..") != 0) {
        return 1;
      }

      cp += dp->rec_len;
      dp = (DIR *) cp;

      //DEBUG
      //break;
    }
  }
  return 0;
}

int myrmdir(char *pathname) {
  int dev = getStartDev(pathname);

  char* bname = basename(pathname);
  char* dname = dirname(pathname);
  v_printf("myrmdir(): dname=\"%s\" ; bname=\"%s\"\n",dname,bname);

  int pino = getino(&dev, dname);
  if (!pino) {
    printf("myrmdir: can't find '%s'\n",pathname);
    return -1;
  }
  v_printf("found pino from pathname: %d\n",pino);
  MINODE* pmip = iget(dev, pino);
  if (!pmip)
    return -6;
  int ino = getino(&pmip->dev, bname);
  v_printf("found ino from pathname: %d\n",ino);
  MINODE* mip = iget(pmip->dev, ino);

  INODE* ip = & mip->INODE;

  if (S_ISDIR(mip->INODE.i_mode)) {
    v_printf("is a DIR file\n");
  } else if (S_ISREG(mip->INODE.i_mode)){
    v_printf("is a REG file (not dir)\n");
    printf("rmdir: error: pathname must be a dir\n");
    iput(mip);
    return -2;
  } else {
    printf("rmdir: error: is neither DIR nor REG\n");
    iput(mip);
    return -3;
  }
  //v_printf("refCount: %d\n",mip->refCount);
  --ip->i_links_count;
  v_printf("new links count: %d\n",ip->i_links_count);
  if (ip->i_links_count > 2 || hasChildren(mip)) {
    //dir not empty
    // the second check is necessary for files that don't increase i_links_count
    // e.g. reg files
    printf("rmdir: error: directory must be empty\n");
    iput(mip);
    return -4;
  }

  if (!(ip->i_mode & S_IWUSR) || ip->i_uid != running->uid ) {
    printf("rmdir: error: the permissions to do not allow that\n");
    iput(mip);
    return -5;
  }
/*
  if (mip->refCount == 1) {
    printf("error: minode is BUSY\n");
    iput(mip);
    return -5;
  }
  */
  //assume only direct blocks
  //for (int i=0; i<12 && ip->i_block[i]; i++) {
  for (int i=0; i<12 ; i++) {
    if (ip->i_block[i]) {
      bdealloc(mip->dev, ip->i_block[i]);
    }
  }
  idealloc(mip->dev, mip->ino);
  iput(mip);


  if (rm_child_entry(pmip, bname) != 0) {
    printf("rmdir: error: failed to remove child entry\"%s\"\n",bname);
    return -6;
  }

  if (S_ISDIR(ip->i_mode)) {
    --(pmip->INODE).i_links_count;
  }



  iput(pmip);
  return 0;
}
