#include "fs.h"


// Allocates a new inode and returns its number
int ialloc(int dev)
{
  int  i;
  char buf[BLKSIZE];



  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0)
    {
       set_bit(buf,i);
       put_block(dev, imap, buf);
       decFreeInodes(dev);
       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
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
       put_block(dev, bmap, buf);
       decFreeBlocks(dev);

       return i+1;
    }
  }
  printf("balloc(): no more free blocks\n");
  return 0;
}

void idealloc(int dev, int ino)
{
  int i;
  char buf[BLKSIZE];


  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);

  // update free inode count in SUPER and GD
  incFreeInodes(dev);
}

void bdealloc(int dev, int bitToFree)
{
    int i;
    char buf[BLKSIZE];

    // Get block bitmap
    get_block(dev, bmap, buf);
    clr_bit(buf, bitToFree);

    // Write block to disk
    put_block(dev, bmap, buf);
    incFreeBlocks(dev);
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

// Increment the number of free blocks available in superblock and group
// descriptor bitmaps
int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    // Get super block
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    // Get group descriptior
    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
}

// Decrement the number of free inodes
int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  printf("num free inodes: %d\n", sp->s_free_inodes_count);
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

// Decrement the number of free blocks
int decFreeBlocks(int dev)
{
  char buf[BLKSIZE];

  //dec free block count in super and gd
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}




int tst_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;  j = bit % 8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

// Sets bit
int set_bit(char *buf, int bit)
{
  int i, j;
  i = bit / 8;
  j = bit % 8;
  buf[i] |= (1 << j);
  return 0;
}

// Clears specified bit
int clr_bit(char *buf, int bit)
{
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

