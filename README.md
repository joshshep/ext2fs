# Ext2fs
A simulation of the Ext2fs built from scratch in C. Read about ext2: https://en.wikipedia.org/wiki/Ext2 .

Currently, the following commands are supported:
* level 1: menu|ls|cd|pwd|quit|mkdir|rmdir|creat|link|unlink|symlink|readlink
         touch|chmod
* level 2: cp|mv|pfd
* level 3: mount|umount

## Getting started
Clone the repo:
```
git clone https://github.com/joshshep/ext2fs.git
```
Change directories and build the program:
```
cd ext2fs
make
```
Run the program:
```
./ext2fs
```

## Note:
* Doesn't handle spaces in names very well at all
* similarly: quotes are ignored as well as escape sequences
