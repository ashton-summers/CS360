#include "Lab6.h"

MINODE *iget(int dev, int ino)
{
  int i, blk, disp;
  char buf[BLKSIZE];
  MINODE *mip;
  INODE *ip;
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
       mip->dirty = mip->mounted = mip->mptr = 0;
       // get INODE of ino into buf[ ]
       blk  = (ino-1)/8 + iblock;  // iblock = Inodes start block #
       disp = (ino-1) % 8;
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
//(1). mip->refCount--;

//(2). if (mip->refCount > 0) return;
     //if (!mip->dirty)       return;

//(3).  /* write INODE back to disk */

 //printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);

 //Use mip->ino to compute

     //blk containing this INODE
     //disp of INODE in blk

     //get_block(mip->dev, block, buf);

     //ip = (INODE *)buf + disp;
     //*ip = mip->INODE;

     //put_block(mip->dev, block, buf);
}


int getino(int *dev, char *pathname)
{
  int i, ino, blk, disp;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0]=='/')
     mip = iget(*dev, 2);
  else
     mip = iget(running->cwd->dev, running->cwd->ino);

  strcpy(buf, pathname);
  //tokenize(buf); // n = number of token strings

  for (i=0; i < n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, dirNames[i]);

      ino = search(mip, dirNames[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", dirNames[i]);
         return 0;
      }
      iput(mip);
      mip = iget(*dev, ino);
   }
   return ino;
}


int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}
