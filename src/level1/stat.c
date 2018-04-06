#include "../../include/level1cmd.h"

void getModeStr(char *destStr, int i_mode)
{
  destStr[0] = S_ISLNK(i_mode) ? 'l' : ((S_ISDIR(i_mode)) ? 'd' : '-');
  destStr[1] = (i_mode & S_IRUSR) ? 'r' : '-';
  destStr[2] = (i_mode & S_IWUSR) ? 'w' : '-';
  destStr[3] = (i_mode & S_IXUSR) ? 'x' : '-';
  destStr[4] = (i_mode & S_IRGRP) ? 'r' : '-';
  destStr[5] = (i_mode & S_IWGRP) ? 'w' : '-';
  destStr[6] = (i_mode & S_IXGRP) ? 'x' : '-';
  destStr[7] = (i_mode & S_IROTH) ? 'r' : '-';
  destStr[8] = (i_mode & S_IWOTH) ? 'w' : '-';
  destStr[9] = (i_mode & S_IXOTH) ? 'x' : '-';
  destStr[10] = 0;
}


int do_stat(char *pathname)
{
  int dev = getStartDev(pathname);

  int ino = getino(&dev, pathname);

  if (!ino)
  {
    printf("stat: error: couldn't find \"%s\"\n",pathname);
    return -1;
  }

  MINODE *mip = iget(dev, ino);
  if (!mip)
    return -2;
  INODE *ip = & mip->INODE;

  char mode_str[64];
  getModeStr(mode_str, ip->i_mode);

  // 8 == len(1024 * (12 + 256 + 256*256))
  printf("File: '%s'\n",pathname);
  printf("Size: %10d Blocks: \n", ip->i_size);
  printf("Device: %10d Inode: %d Links: %d\n", dev, ino, ip->i_links_count);
  printf("Access: %s Uid: %3d  Gid: %3d\n", mode_str, ip->i_uid, ip->i_gid);
  /*printf("Access: %10d\n");
  printf("Modify: %10d\n");
  printf("Change: %10d\n");
  printf("Birth: -\n");*/

  /*
  File: '.'
  Size: 4096      	Blocks: 8          IO Block: 4096   directory
  Device: 801h/2049d	Inode: 4196412     Links: 2
  Access: (0775/drwxrwxr-x)  Uid: ( 1000/  joshua)   Gid: ( 1000/  joshua)
  Access: 2017-06-10 08:11:00.850413545 -0700
  Modify: 2017-05-30 13:35:24.805211000 -0700
  Change: 2017-05-30 13:35:24.805211000 -0700
  Birth: -
  */

  iput(mip);

  return EXIT_SUCCESS;
}
