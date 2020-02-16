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

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// define shorter TYPES, save typing efforts
// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;
SUPER *sp;
INODE *ip;
DIR   *dp;


#define BLKSIZE  1024
#define NMINODE   100
#define NFD        16
#define NPROC       2

typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntable *mptr;
}MINODE;

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

char buf[BLKSIZE], dbuf[BLKSIZE], sbuf[BLKSIZE];
char *dirNames[256];
int fd;
int iblock;
int n;


int get_block(int fd, int blk, char buf[ ]);
int put_block(int fd, int blk, char buf[ ]);
MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int getino(int *dev, char *pathname);
void super();
void groupDescriptor();
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
void imap();
void bmap();
void inode();
void findDir();
int search(INODE * ip, char * name);
int decFreeInodes(int dev);
int ialloc(int dev);
int balloc(int dev);



#endif
