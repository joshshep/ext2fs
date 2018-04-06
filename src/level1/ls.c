#include "../../include/level1cmd.h"

#define MAX_TM_LEN (64)
// http://stackoverflow.com/a/36445616
// writes the date (from epoch) to the ls -l format
int format_time(char *buf, time_t epoch_tm) {
  time_t now = time (NULL);
  struct tm * tmfile, * tmnow;
  
  tmnow = localtime(&now);
  int this_year = tmnow->tm_year;

  tmfile = localtime(&epoch_tm);
  
  if (tmfile->tm_year == this_year) {
    // year is current output date/time
    strftime (buf, MAX_TM_LEN, "%b %e %H:%M", tmfile);
  }
  else { //if year is not current, output time/year
    strftime (buf, MAX_TM_LEN, "%b %e  %Y", tmfile);
  }
  return 0;
}

int ls_file(INODE *ip, char *fname) {
  //http://unix.stackexchange.com/questions/103114/what-do-the-fields-in-ls-al-output-mean

  //permissions
  printf( S_ISLNK(ip->i_mode) ? "l" : ((S_ISDIR(ip->i_mode)) ? "d" : "-"));
  printf( (ip->i_mode & S_IRUSR) ? "r" : "-");
  printf( (ip->i_mode & S_IWUSR) ? "w" : "-");
  printf( (ip->i_mode & S_IXUSR) ? "x" : "-");
  printf( (ip->i_mode & S_IRGRP) ? "r" : "-");
  printf( (ip->i_mode & S_IWGRP) ? "w" : "-");
  printf( (ip->i_mode & S_IXGRP) ? "x" : "-");
  printf( (ip->i_mode & S_IROTH) ? "r" : "-");
  printf( (ip->i_mode & S_IWOTH) ? "w" : "-");
  printf( (ip->i_mode & S_IXOTH) ? "x" : "-");

  //num links
  printf(" %2d",ip->i_links_count);

  // owner name
  printf(" %3d",ip->i_uid);

  // owner group
  printf(" %3d",ip->i_gid);

  // size
  printf(" %10d",ip->i_size);

  //time of last modificiation
  char tm_buf[MAX_TM_LEN];
  format_time(tm_buf, ip->i_mtime);
  printf(" %s",tm_buf);
  // time of last modification
  //printf(" %10d",ip->i_mtime);

  // todo file/dir name
  printf(" %s",fname);
  if (S_ISDIR(ip->i_mode) && fname[0] != '.') {
    printf("/");
  } else if (S_ISLNK(ip->i_mode)) {
    printf(" -> %s", (char *)ip->i_block);

  }
  printf("\n");
  return 0;
}

//call ls_file() on all of the entries in the directory
int ls_dir(MINODE *pmip) {
  INODE *ip = & pmip->INODE;
  // only list the first 12 blocks (direct)
  for (int blk_index=0; blk_index < 12 && ip->i_block[blk_index]; blk_index++) {
    v_printf("i_block[%d] = %d\n",blk_index,ip->i_block[blk_index]);
    get_block(pmip->dev, ip->i_block[blk_index], dbuf);

    dp = (DIR *) dbuf;
    char * cp = dbuf;
    while (cp < & dbuf[BLKSIZE]) {
      strncpy(sbuf, dp->name, dp->name_len);
      sbuf[dp->name_len] = 0;

      MINODE * mip = iget(pmip->dev,dp->inode);
      if (!mip) {
        printf("ls: couldn't get mip from dp->inode\n");
        return -1;//
      }
      INODE * ent_ip = & mip->INODE;

      ls_file(ent_ip,sbuf);

      cp += dp->rec_len;
      dp = (DIR *) cp;
      iput(mip);
      //DEBUG
      //break;
    }
  }
  return 0;
}

// dig out YOUR OLD lab work for ls() code
int ls(char *pathname) {
  int dev = getStartDev(pathname);
  char sbuf[MAX_LINE];
  strcpy(sbuf, pathname);
  int ino = getino(&dev, pathname);
  if (!ino) {
    printf("ls: can't find '%s'\n",sbuf);
    return -1;
  }
  v_printf("found ino from pathname: %d\n",ino);
  MINODE * mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  if (S_ISDIR(mip->INODE.i_mode)) {
    v_printf("is a DIR file\n");
    ls_dir(mip);
  } else if (S_ISREG(mip->INODE.i_mode)){
    v_printf("is a REG file (not dir)\n");

    ls_file(ip, basename(sbuf));
    //ls_file();

    //is dir

  } else {
    printf("error: is neither DIR nor REG\n");
    iput(mip);
    return -2;
  }
  
  iput(mip);

  return 0;
  /*
  WRITE C code for these:

  determine initial dev:
  if pathname[0]== '/': dev = root->dev;
  else                : dev = running->cwd->dev;

  convert pathname to (dev, ino);
  get a MINODE *mip pointing at a minode[ ] with (dev, ino);


  if mip->INODE is a file: ls_file(pathname);
  if mip->INODE is a DIR{
  step through DIR entries:
  for each name, ls_file(pathname/name);
  }
  */
}
