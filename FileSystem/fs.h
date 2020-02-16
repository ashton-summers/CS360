#ifndef LAB6_H
#define LAB6_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <ext2fs/ext2_fs.h>
#include <linux/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 _DIR;    // need this for new version of e2fs
struct stat mystat, *s;

GD    *gp;
SUPER *sp;
INODE *ip;
_DIR   *dp;


#define BLKSIZE  1024
#define NMINODE   100
#define NFD        16
#define NPROC       2
#define NMOUNT     10

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct MountEntry *mptr;
}MINODE;

typedef struct mountentry
{
    char name[64];
    char mountpoint[64];
    int nblocks;
    int ninodes;
    int bmap;
    int imap;
    int startBlock;
    int dev;
    MINODE *inode;

}MountEntry;

typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          uid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

MINODE minode[NMINODE];        // global minode[ ] array
MINODE *root;                  // root pointer: the /
PROC   proc[NPROC], *running;  // PROC; using only proc[0]
MountEntry mountTable[NMOUNT];


char buf[BLKSIZE], dbuf[BLKSIZE], sbuf[BLKSIZE];
char cmd[256], pathName[256], pathTemp[256];
char rootDir[256];
char *dirNames[256], pwdName[256];
char wd[256], prevWd[256];
int fd, dev, iblock, n, ninodes, nblocks, bmap, imap, pwdI, nfreeInodes, nfreeBlocks,
originalDev;



void tokenize();

// UTIL FUNCTIONS
int get_block(int fd, int blk, char buf[ ]);
int put_block(int fd, int blk, char buf[ ]);
int iput(MINODE *mip);
int getino(int *dev, char *pathname);
void super();
void groupDescriptor();
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);

int search(INODE * ip, char * name);
int decFreeInodes(int dev);
int incFreeInodes(int dev);
int incFreeBlocks(int dev);
void idealloc(int dev, int ino);
void bdealloc(int dev, int bitToFree);
int ialloc(int dev);
int balloc(int dev);
void printMenu();
MINODE* getmounted(int * dev, MINODE * mip);
int findmountentry(MINODE * mip);
int findmountpoint(MINODE * mip);
//END UTIL FUNCTIONS

int init();
int mount_root();
int changeDir (char *pathname);
int ls(char *pathname);
void pwd(MINODE *cwd, char name[], int i);

// MKDIR_CREAT.C FUNCTIONS
void makeDir(char *path);
int mymkdir(MINODE *pip, char *name);
int enterName(MINODE *pip, int mino, char *name);
int creatFile(char *path);
int myCreat(MINODE *pip, char *name);
int creatLink(MINODE *pip, char *name, int ino);
int enterFileName(MINODE *pip, int mino, char *name);
// END MKDIR_CREAT.C FUNCTIONS

// RMDIR.C FUNCTIONS
int rmDir(char *path);
int rmChild(MINODE *pip, char *name);
int dirIsEmpty(MINODE *mip);
// END RMDIR.C FUNCTIONS

// LINK_UNLINK_SYMLINK.C FUNCTIONS
int myLink(char *oldFileName, char *newFileName);
int myUnlink(char *path);
int mySymLink(char *oldFileName, char *newFileName);
int readLink(char *file, char *buf);
// LINK_UNLINK_SYMLINK.C FUNCTIONS

// OPEN_CLOSE.C FUNCTIONS
int openFile(char *path, int mode);
int closeFile(int fd);
int checkOft(MINODE *mip);
int findSmallestSlot();
void pfd();
int mylseek(int fd, int position);
// END OPEN_CLOSE.C FUNCTIONS

//READ_WRITE.C FUNCTIONS
int readFile(int fd, int nbytes);
int myRead(int fd, char *buf, int nbytes);
int writeFile();
int myWrite(int fd, char buf[], int nbytes);
int myCat(char *path);
int myTruncate(MINODE *mip);
int myCp(char source[], char dest[]);
int myMV(char source[], char dest[]);
//READ_WRITE.C FUNCTIONS

//TOUCH_CHMOD.C FUNCTIONS
int mychmod(char *path, char *newMode);
int myTouch(char *path);
//END TOUCH_CHMOD.C FUNCTIONS


//MOUNT_UMOUNT.C FUNCTIONS
int mount(char* name, char *mountPoint);
int unmount (char *filesystem);
//END MOUNT_UMOUND.C FUNCTIONS


#endif
