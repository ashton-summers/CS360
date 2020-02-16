#include "fs.h"

/*
    Opens a file and stores it in the open file table
    aka 'oft' of running process
    mode = 0|1|2|3 for R|W|RW|APPEND
*/
int openFile(char *path, int mode)
{
    int ino, i, smallestSlot;
    MINODE *mip;
    OFT *newOft = (OFT *)malloc(sizeof(OFT));

    // If the path is absolute.
    if (path[0] == '/')
    {
        dev = root->dev;
    }
    else // Path is relative.
    {
        dev = running->cwd->dev;
    }

    ino = getino(&dev, path);

    // If the file does not exist, create it
    if(ino == 0)
    {
        printf("CREATING FILE TO OPEN\n");
        if(creatFile(path) == 0)
        {
            printf("Open failed\n");
            return - 1;
        }
        ino = getino(&dev, path);
    }
    mip = iget(dev, ino);

    if (!S_ISREG(mip->INODE.i_mode))
    {
        printf("Error: Incompatible type. Not regular\n");
        return -1;
    }

    //printf("link count: %d\n", mip->refCount);
    if(checkOft(mip) == -1)
        return -1;


    // Set values of newly allocated oft
    newOft->mode = mode;
    newOft->mptr = mip;
    newOft->refCount = 1;

    switch(mode)
    {
        case 0:
            newOft->offset = 0;
            mip->INODE.i_atime = time(0L);
            break;
        case 1:
            // TODO: truncate the file
            myTruncate(mip);
            newOft->offset = 0;
            mip->INODE.i_atime = time(0L);
            mip->INODE.i_mtime = time(0L);
            break;
        case 2:
            newOft->offset = 0;
            mip->INODE.i_atime = time(0L);
            mip->INODE.i_mtime = time(0L);
            break;
        case 3:
            newOft->offset = mip->INODE.i_size;
            mip->INODE.i_atime = time(0L);
            mip->INODE.i_mtime = time(0L);
            break;
        default:
            printf("Invalid mode\n");
            return -1;
    }

    smallestSlot = findSmallestSlot();
    running->fd[smallestSlot] = newOft;

    mip->dirty = 1;
    //iput(mip);
    //printf("ref count: %d\n", mip->refCount);

    return smallestSlot;

}

// Closes the file specified by its descriptor
int closeFile(int fd)
{
    OFT *oftp;
    MINODE *mip;
    if (fd < 0 || fd >= NFD)
    {
        printf("Error: The specified file descriptor is out of range\n");
        return -1;
    }

    if(running->fd[fd] == 0)
    {
        printf("Error: The file descriptor is null\n");
        return -1;
    }

    oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if(oftp->refCount > 0){ return 0; }
    mip = oftp->mptr;
    iput(mip);
}

// Check oft to see if file is opened with incompatible mode.
int checkOft(MINODE *mip)
{
    int fdMode, i;
    for (i = 0; i < NFD; i++)
    {
        if(running->fd[i])
        {
            fdMode = running->fd[i]->mode;
            if (running->fd[i]->mptr->ino == mip->ino && fdMode == 1 || fdMode == 2 || fdMode == 3 )
            {
                switch(fdMode)
                {
                    case 1:
                        printf("Error: This file has already been opened for writing\n");
                        return -1;
                    case 2:
                        printf("Error: This file has already been opened for RW\n");
                        return -1;
                    case 3:
                        printf("Error: This file has already been opened for appending\n");
                        return -1;
                }
            }
        }
    }
    return 0;
}

/*
Finds the smallest slot in running->oft such that
running->fd[i] is null
*/
int findSmallestSlot()
{
    int i = 0;

    for(i = 0; i < NFD; i++)
    {
        if(running->fd[i] == 0)
        {
            return i;
        }
    }

    // Only hit this line if we have opened more files than we have space for
    printf("No more space in file table\n");
    return -1;
}

// Prints data in running->oft
void pfd()
{
    int i = 0;
    OFT *fd;
    printf("fd    mode    offset    INODE    SIZE\n");
    printf("--    ----    ------    -----    ----\n");

    for (i = 0; i < NFD; i++)
    {
        if(running->fd[i])
        {
            fd = running->fd[i];
            printf("%d       %d       %d      [%d, %d]   %d\n", i, fd->mode, fd->offset, fd->mptr->dev, fd->mptr->ino, fd->mptr->INODE.i_size);
        }
    }
}

/*
 Changes the position of the specified file descriptor
 to the indicated position
*/
int mylseek(int fd, int position)
{
    OFT *temp = running->fd[fd];
    int originalPosition = running->fd[fd]->offset;
    if (fd < 0 || fd >= NFD)
    {
        printf("The specified file descriptor is out of range\n");
        return -1;
    }

    if (position < 0 || position > temp->mptr->INODE.i_size)
    {
        printf("The specified position is not within range\n");
        return -1;
    }

    running->fd[fd]->offset = position;

    return originalPosition;
}

int myTruncate(MINODE *mip)
{
    int i = 0, j = 0, ibuf[256], dbuf[256];


    // Dealloc the direct blocks
    for(i = 0; i < 12; i++)
    {
        if(mip->INODE.i_block[i])
        {
            bdealloc(mip->dev, mip->INODE.i_block[i]);
            mip->INODE.i_block[i] = 0;
        }
    }

    get_block(mip->dev, mip->INODE.i_block[12], ibuf);

    // Dealloc the indirect blocks. All 256
    if (mip->INODE.i_block[12])
    {
        for(i = 0; i < 256; i++)
        {
            if(ibuf[i])
            {
                printf("ibuf[%d] in truncate\n", ibuf[i]);
                bdealloc(mip->dev, ibuf[i]);
                ibuf[i] = 0;
            }
        }

        mip->INODE.i_block[12] = 0;
    }

    get_block(mip->dev, mip->INODE.i_block[13], dbuf);

    // Dealloc all 256 * 256 double indirect blocks
    if(mip->INODE.i_block[13])
    {
        // Free the double indirect blocks
        for(i = 0; i < 256; i++)
        {
            get_block(mip->dev, dbuf[i], ibuf);

            if(dbuf[i])
            {

                for(j = 0; j < 256; j++)
                {
                    bdealloc(mip->dev, ibuf[j]);
                    ibuf[j] = 0;
                }
                bdealloc(mip->dev, dbuf[i]);
                dbuf[i] = 0;
            }

        }

        mip->INODE.i_block[13] = 0;
    }

    mip->INODE.i_size = 0;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
    mip->dirty = 1;
    iput(mip);
}
