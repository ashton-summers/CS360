#include "fs.h"
MINODE *iget(int dev, int ino)
{
  int i, blk, disp;
  char buf[BLKSIZE];
  MINODE *mip;
  INODE *ip;
  printf("dev: %d\n", dev);
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino)
    {
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
  for (i=0; i < NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev; mip->ino = ino;  // assing to (dev, ino)
       mip->dirty = mip->mounted = mip->mptr = 0;

       // get INODE of ino into buf[ ]
       blk  = (ino-1)/8 + iblock;  // iblock = Inodes start block #
       disp = (ino-1) % 8;
       //printf("iget: ino=%d blk=%d disp=%d\n", ino, blk, disp);
       get_block(dev, blk, buf);
       ip = (INODE *)buf + disp;
       //printf("blk = %d  disp = %d  ip = %d\n", blk, disp, ip->i_block[0]);
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }
  printf("PANIC: no more free minodes\n");
  return 0;
}

// Write inode in memory back to disk
int iput(MINODE *mip)  // dispose of a minode[] pointed by mip
{
    int blk, disp;

    mip->refCount--;
    if (mip->refCount > 0) return;
    if (!mip->dirty) return;
    //printf("iput: dev=%d ino=%d\n", mip->dev, mip->ino);
    blk = (mip->ino - 1) / 8 + iblock;
    disp = (mip->ino - 1) % 8;
    get_block(mip->dev, blk, buf);
    ip = (INODE *)buf + disp;
    memcpy(ip, &mip->INODE, 128);
    put_block(mip->dev, blk, buf);
}


int getino(int *dev, char *pathname)
{
  int i, ino, blk, disp;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  //printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;

  if (pathname[0]=='/')
     mip = iget(*dev, 2);
  else
     mip = iget(running->cwd->dev, running->cwd->ino);

  strcpy(pathName, pathname);
  tokenize(); // n = number of token strings

  for (i=0; i < n; i++){
      //printf("===========================================\n");
      //printf("getino: i=%d name[%d]=%s\n", i, i, dirNames[i]);

      ino = search(mip, dirNames[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", dirNames[i]);
         return 0;
      }
      iput(mip);
      if(mip->mounted)
      {

            mip = getmounted(dev, mip);
            printf("dev after change: %d\n", dev);
            ino = mip->ino;
      }
      else
      {
        mip = iget(*dev, ino);
      }


      //printf("ref count in getino: %d\n", mip->refCount);
   }
    if (mip->mounted)
    {
      mip = getmounted(dev, mip);
      ino = mip->ino;
    }
   iput(mip);
   return ino;
}
