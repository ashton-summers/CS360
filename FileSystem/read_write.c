#include "fs.h"

// Calls 'myRead' and returns number of bytes read.
int readFile(int fd, int nbytes)
{
    char buf[BLKSIZE];
    OFT *temp;

    temp = running->fd[fd];

    if(temp)
    {
        if(temp->mode != 0 && temp->mode != 2)
        {
            printf("The file descriptor entered is not opened for R or RW\n");
            return 0;
        }
    }


    int bytesRead = (myRead(fd, buf, nbytes));
    printf("*************** READ RESULTS **************\n");
    buf[bytesRead] = 0;
    printf("%s", buf);
    printf("*************** END READ RESULTS **************\n");
    return bytesRead;
}

// Reads n bytes from the specified file.
int myRead(int fd, char buf[], int nbytes)
{
    int count = 0, lbk, startByte, blk, ibuf[256], dbuf[256], tempBytes = nbytes;
    OFT * oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    int avil = mip->INODE.i_size - oftp->offset;
    int remain;
    char *maxBuf = buf, readBuf[BLKSIZE]; // cq points at buf[ ]


  while (nbytes && avil)
  {

      lbk = oftp->offset / BLKSIZE;
      startByte = oftp->offset % BLKSIZE;

       if (lbk < 12){                     // lbk is a direct block
           blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
             get_block(mip->dev, blk, readBuf);
             //printf("ahsdfhaskdjfha\n");
       }
       else if (lbk >= 12 && lbk < 256 + 12)
       {
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            blk = (lbk - 12);
            //printf("indirect blk: %d\n", blk);
            get_block(mip->dev, ibuf[blk], readBuf);
       }
       else
       {
            //  double indirect blocks
            get_block(mip->dev, mip->INODE.i_block[13], dbuf);
            blk = ((lbk - 268) / 256);
            //printf("double indirect blk #: %d\n", blk);

            get_block(mip->dev, dbuf[blk], ibuf);
            blk = ((lbk - 268) % 256);
            //printf("indirect blk #: %d\n", blk);
            get_block(mip->dev, ibuf[blk], readBuf);
            //printf("ibuf dbl: %d\n", ibuf[blk]);
            //printf("ibuf[blk]: %d\n\n\n", ibuf[blk]);
       }


       /* copy from startByte to buf[ ], at most remain bytes in this block */
       char *cp = readBuf + startByte;
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
       //while (remain > 0)
       //{
            //*cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             //oftp->offset++;           // advance offset
             //count++;                  // inc count as number of bytes read
             //avil--; nbytes--;  remain--;
             //if (nbytes <= 0 || avil <= 0)
                // break;
       //}

       if(avil >= nbytes)
       {
            strncpy(maxBuf, cp, nbytes);
            maxBuf += nbytes;
            cp += nbytes;
            remain -= nbytes;
            oftp->offset += nbytes;
            if(oftp->offset > mip->INODE.i_size)
            {
                mip->INODE.i_size += nbytes;
            }
            nbytes -= nbytes;
            return tempBytes;
       }
       else
       {
            strncpy(maxBuf, cp, remain);
            maxBuf += remain;
            cp += remain;

            oftp->offset += avil;
            //printf("size: %d\n", mip->INODE.i_size);
           // printf("offset: %d\n", oftp->offset);
            if(oftp->offset > mip->INODE.i_size)
            {
                mip->INODE.i_size += avil;
            }
           // printf("size: %d\n", mip->INODE.i_size);
           if(avil <= remain)
           {
                tempBytes = avil;
                return tempBytes;
           }

            return remain;
       }

       // if one data block is not enough, loop back to OUTER while for more ...

   }
   //printf("myread: read %d char from file descriptor %d\n", count, fd);
   return 0;   // count is the actual number of bytes read
}


int writeFile()
{
    char line[128], buf[BLKSIZE], tempBuf[BLKSIZE];
    OFT *temp;
    int fd = 0, nbytes = 0;
    printf("Enter a file descriptor: ");
    fgets(line, 128, stdin);
    sscanf(line, "%d", &fd);
    printf("enter a string to write: ");
    fgets(buf, 128, stdin);
    temp = running->fd[fd];

    if(temp)
    {
        if(temp->mode != 1 && temp->mode != 2 && temp->mode != 3)
        {
            printf("The file descriptor entered is not opened for R or RW\n");
            return 0;
        }
    }

    nbytes = strlen(buf);

    return(myWrite(fd, buf, nbytes));
}

// Writes nbytes to the file given a descriptor
int myWrite(int fd, char buf[], int nbytes)
{
    int lbk = 0, startByte = 0, blk = 0, remain = 0, tempBytes = nbytes, dblock, doffset, dindex;
    int ibuf[256];
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->mptr;
    INODE *ip;
    char wbuf[BLKSIZE], *maxBuf = buf;
    memset(wbuf, 0, 1024);
    memset(ibuf, 0, 256);
    ip = &(mip->INODE);

    while (nbytes > 0)
    {

          lbk       = oftp->offset / BLKSIZE;
          startByte = oftp->offset % BLKSIZE;

        if (lbk < 12)
        {                         // direct block
            if (mip->INODE.i_block[lbk] == 0)
            {
                mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block                                     but MUST for I or D blocks
            }
           blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
        }
        else if (lbk >= 12 && lbk < 256 + 12)
        {
            if(ip->i_block[12] == 0 && lbk == 12)
            {
                ip->i_block[12] = balloc(mip->dev);

                get_block(mip->dev, ip->i_block[12], ibuf);
                ibuf[0] = balloc(mip->dev);
                blk = ibuf[0];
                put_block(mip->dev, ip->i_block[12], ibuf);
            }
            else
            {
                get_block(mip->dev, ip->i_block[12], ibuf);
                if(ibuf[lbk - 12] == 0)
                {
                    ibuf[lbk - 12] = balloc(mip->dev);
                }

                blk = ibuf[lbk - 12];
                put_block(mip->dev, ip->i_block[12], ibuf);
            }
        }
        else
        {
            // calculate the double indirect block # and the indirect block index
            dindex = (lbk - 268) / 256;
            doffset = (lbk -268) % 256;

            if (ip->i_block[13] == 0)
            {
                ip->i_block[13] = balloc(dev); // alloc a block
            }

            // read the block into the int buf
            get_block(mip->dev, ip->i_block[13], (char *)ibuf);

            if (ibuf[dindex] == 0) // if double indirect block is 0
            {
                ibuf[dindex] = balloc(dev); // alloc a block
            }

            dblock = ibuf[dindex]; // set block to indirect block # that the double indirect block points to

            put_block(mip->dev, ip->i_block[13], (char *)ibuf); // write int buf back to disk
            get_block(mip->dev, dblock, (char *)ibuf); // get double indirect block int int buf

            if(ibuf[doffset] == 0) // if the indirect block is 0
            {
                ibuf[doffset] = balloc(dev); // alloc block
            }
            blk = ibuf[doffset]; // set block to the block that the indirect block points to

            put_block(mip->dev, dblock, (char *)ibuf); // write indirect block back to disk

        }

        /* all cases come to here : write to the data block */
        get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]
        char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
        remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

        if(remain >= nbytes)
        {
            strncpy(cp, maxBuf, nbytes);
            maxBuf += nbytes;
            cp += nbytes;
            remain -= nbytes;
            oftp->offset += nbytes;
            if(oftp->offset > ip->i_size)
            {
                ip->i_size += nbytes;
            }
            nbytes -= nbytes;
        }
        else
        {
            strncpy(cp, maxBuf, remain);
            maxBuf += remain;
            cp += remain;
            nbytes -= remain;

            if(oftp->offset > ip->i_size)
            {
                ip->i_size += remain;
            }

                remain -= remain;
        }
        put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
    }

     // loop back to while to write more .... until nbytes are written
    mip->dirty = 1;       // mark mip dirty for iput()
    iput(mip);
    printf("wrote %d chars into file descriptor fd=%d\n", tempBytes, fd);
    return tempBytes;
}


// Displays the contents of a file to the screen
int myCat(char *path)
{
    char myBuf[BLKSIZE];
    int n = 0, fd = -1;

    printf("*************** FILE CONTENTS ***************\n");
    fd = openFile(path, 0);

    if(fd == -1)
    {
        return -1;
    }
    while(n = myRead(fd, myBuf, 1024)) // Read n bytes.
    {
        myBuf[n] = 0;
        printf("%s", myBuf);
        memset(myBuf, 0, 1024);
    }
    printf("\n*************** END FILE CONTENTS ***************\n");

    closeFile(fd);

}

// Copies contents of the source to the destination
int myCp(char source[], char dest[])
{
    char temp[BLKSIZE];
    int fd, gd, n;
    char buf[BLKSIZE];

    strcpy(temp, source);
    dest[strlen(dest)] = 0;
    fd = openFile(source, 0);
    gd = openFile(dest, 1);

    pfd();

    if (fd == -1 || gd == -1)
    {
        return -1;
    }

    while(n = myRead(fd, buf, BLKSIZE))
    {
        myWrite(gd, buf, n);
    }
    closeFile(fd);
    closeFile(gd);
}

//
int myMV(char source[], char dest[])
{
    MINODE *mip;
    char temp[256];
    int fd;

    strcpy(temp, source);

  fd = openFile(source, 0);


    if(fd < 0)
    {
        printf("Source file does not exist\n");
        return -1;
    }

    mip = running->fd[fd]->mptr;

    if(mip->dev == dev)
    {
        myLink(source, dest);
        myUnlink(temp);
        closeFile(fd);
        //printf("link count: %d\n", mip->refCount);
        mip->dirty = 1;
        iput(mip);
        return 1;
    }
    else
    {
      myCp(source, dest);
      myUnlink(source);
    }
}


