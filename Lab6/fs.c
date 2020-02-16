#include "Lab6.h"

// prints the bit map for inodes
void imap()
{
  char buf[BLKSIZE];
  int  imap, ninodes;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  imap = gp->bg_inode_bitmap;
  printf("bmap = %d\n", imap);

  // read inode_bitmap block
  get_block(fd, imap, buf);

  for (i=0; i < ninodes; i++)
  {
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n\n");
}

// prints the bit map for block
void bmap()
{
  char buf[BLKSIZE];
  int  bmap, ninodes;
  int  i;

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  printf("bmap = %d\n", bmap);

  // read inode_bitmap block
  get_block(fd, bmap, buf);

  for (i=0; i < ninodes; i++)
  {
    (tst_bit(buf, i)) ?	putchar('1') : putchar('0');
    if (i && (i % 8)==0)
       printf(" ");
  }
  printf("\n\n");
}

void findDir()
{
    int i = 0, block = 0, offset = 0;
    // Read SUPER block at offset 1024
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    printf("Checking ext2 FS...");
    if (sp->s_magic != 0xEF53) {
        printf("Magic: %x; this is not an EXT2 fs\n", sp->s_magic);
        exit(2);
    }
    else
        printf("FS loaded\n");

    super();
    // Read GD block at (block0 + 1)
    get_block(fd, 2, buf);
    gp = (GD *)buf;

    printf("GD info: %d %d %d %d %d %d %d\n",
        gp->bg_block_bitmap,
        gp->bg_inode_bitmap,
        gp->bg_inode_table,
        gp->bg_free_blocks_count,
        gp->bg_free_inodes_count,
        gp->bg_used_dirs_count);

    iblock = gp->bg_inode_table;
    printf("Inodes beginning block: %d\n", iblock);

    // Read first inode block to get root inode -- block2
    get_block(fd, iblock, buf);
    ip = (INODE *)buf + 1;

     printf(" -- Root node info --\n");
    //printf("mode=%4x  ", ip->inode);
    printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
    //printf("size=%d\n", i->i_size);
    printf("time=%s", ctime(&ip->i_ctime));
    printf("link=%d\n", ip->i_links_count);
    printf("i_block[0]=%d\n", ip->i_block[0]);
    //rootblock = ip->i_block[0];
    printf(" -- -- -- -- -- -- --\n");

    printf("******* ROOT DIR ENTRIES ********\n");

    if (!dirNames[0])
    {
        search(ip, ".");
        int j;
        printf(" --------- DISK BLOCKS ----------\n");
         for (j = 0; j < ip->i_blocks; j++) {
            printf("iblock[%2d] = %d\n", j, ip->i_block[j]);
        }
        return;
    }
    // Search for the inode in block
    for(i = 0; i < n; i++)
    {
        printf("searching for %s in %x\n", dirNames[i], ip);
        printf("i_block[0] = %d\n", ip->i_block[0]);
        int ino = search(ip, dirNames[i]);
        if (ino == 0)
        {
            printf("Cannot find name '%s'\n", dirNames[i]);
            return;
        }
        printf("found %s: ino = %d\n", dirNames[i], ino);
        block = (ino - 1) / 8 + iblock;
        offset = (ino - 1) % 8;
        printf("blk = %d   offset = %d\n\n", block, offset);
        get_block(fd, block, buf);
        ip = (INODE *)buf + offset;

        printf("size = %d   blocks = %d\n", 1024, ip->i_blocks);
        printf(" --------- DISK BLOCKS ----------\n");
        int j;
        for (j = 0; j < ip->i_blocks; j++) {
            printf("iblock[%2d] = %d\n", j, ip->i_block[j]);
        }
    }
}

// Accepts an INODE to root directory
// Searches directory for 'name' file
int search(INODE * ip, char * name)
{
    int i, found = 0;
    char * cp;

    if (!ip) {
        printf("Inode argument cannot be null\n");
        exit(1);
    }

    if (!S_ISDIR(ip->i_mode)) {
        printf("Inode must be for directory\n");
        exit(2);
    }

    // Look at rootblock
    get_block(fd, ip->i_block[0], dbuf);
    dp = (DIR *)dbuf;
    cp = dbuf;

    while (cp < &dbuf[BLKSIZE]) {
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;

        // If we found the correct node,
        // store its number
        if (strcmp(sbuf, name) == 0) {
            found = dp->inode;
        }

        printf("%4d %4d %4d %s\n",
            dp->inode, dp->rec_len, dp->name_len, sbuf);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    return found;
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];

  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)
{
  int  i, ninodes, bmap;
  char buf[BLKSIZE];

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("ialloc(): no more free inodes\n");
  return 0;
}

int balloc(int dev)
{
  int  i, ninodes, bmap;
  char buf[BLKSIZE];

  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  ninodes = sp->s_inodes_count;
  printf("ninodes = %d\n", ninodes);

  // read Group Descriptor 0
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  printf("bmap = %d\n", bmap);

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("balloc(): no more free inodes\n");
  return 0;
}

void groupDescriptor()
{

  // read SUPER block
  get_block(fd, 2, buf);
  gp = (GD *)buf;

  printf("EXT2 FS OK\n");
  printf("bg_block_bitmap = %d\n", gp->bg_block_bitmap);
  printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);
  printf("bg_inode_table = %d\n", gp->bg_inode_table);
  printf("bg_inode_bitmap = %d\n", gp->bg_inode_bitmap);
  printf("bg_free_blocks_count = %d\n", gp->bg_free_blocks_count);
  printf("bg_free_inodes_count = %d\n\n", gp->bg_free_inodes_count);

}

void super()
{
  // read SUPER block
  get_block(fd, 1, buf);
  sp = (SUPER *)buf;

  // check for EXT2 magic number:

  printf("s_magic = %x\n", sp->s_magic);
  if (sp->s_magic != 0xEF53){
    printf("NOT an EXT2 FS\n");
    exit(1);
  }

  printf("EXT2 FS OK\n");

  printf("s_inodes_count = %d\n", sp->s_inodes_count);
  printf("s_blocks_count = %d\n", sp->s_blocks_count);

  printf("s_free_inodes_count = %d\n", sp->s_free_inodes_count);
  printf("s_free_blocks_count = %d\n", sp->s_free_blocks_count);
  printf("s_first_data_blcok = %d\n", sp->s_first_data_block);


  printf("s_log_block_size = %d\n", sp->s_log_block_size);
  //printf("s_log_frag_size = %d\n", sp->s_log_frag_size);

  printf("s_blocks_per_group = %d\n", sp->s_blocks_per_group);
  //printf("s_frags_per_group = %d\n", sp->s_frags_per_group);
  printf("s_inodes_per_group = %d\n", sp->s_inodes_per_group);


  printf("s_mnt_count = %d\n", sp->s_mnt_count);
  printf("s_max_mnt_count = %d\n", sp->s_max_mnt_count);

  printf("s_magic = %x\n", sp->s_magic);

  printf("s_mtime = %s", ctime(&sp->s_mtime));
  printf("s_wtime = %s\n\n", ctime(&sp->s_wtime));


}

void inode()
{
  char buf[BLKSIZE];

  // read GD
  get_block(fd, 2, buf);
  gp = (GD *)buf;
  /****************
  printf("%8d %8d %8d %8d %8d %8d\n",
	 gp->bg_block_bitmap,
	 gp->bg_inode_bitmap,
	 gp->bg_inode_table,
	 gp->bg_free_blocks_count,
	 gp->bg_free_inodes_count,
	 gp->bg_used_dirs_count);
  ****************/
  iblock = gp->bg_inode_table;   // get inode start block#
  printf("inode_block=%d\n", iblock);

  // get inode start block
  get_block(fd, iblock, buf);

  ip = (INODE *)buf + 1;         // ip points at 2nd INODE

  printf("mode=%4x ", ip->i_mode);
  printf("uid=%d  gid=%d\n", ip->i_uid, ip->i_gid);
  printf("size=%d\n", ip->i_size);
  printf("time=%s", ctime(&ip->i_ctime));
  printf("link=%d\n", ip->i_links_count);
  printf("i_block[0]=%d\n", ip->i_block[0]);

 /*****************************
  u16  i_mode;        // same as st_imode in stat() syscall
  u16  i_uid;                       // ownerID
  u32  i_size;                      // file size in bytes
  u32  i_atime;                     // time fields
  u32  i_ctime;
  u32  i_mtime;
  u32  i_dtime;
  u16  i_gid;                       // groupID
  u16  i_links_count;               // link count
  u32  i_blocks;                    // IGNORE
  u32  i_flags;                     // IGNORE
  u32  i_reserved1;                 // IGNORE
  u32  i_block[15];                 // IMPORTANT, but later
 ***************************/
}


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

