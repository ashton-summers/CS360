#include "fs.h"

void makeDir(char *path)
{
    MINODE *pip;
    char parent[256], child[256], temp[256];
    int pino;

    // If path is absolute
    if(path[0] == '/')
    {
        dev = root->dev;
        pip = root;
    }
    else // Path is relative
    {
        dev = running->cwd->dev;
        //pip = running->cwd;
    }

    // Copy to temp first, so we don't destroy our parent and child arrays
    // Get the dir name and the basename of path entered
    strcpy(temp, path);
    strcpy(parent, dirname(temp));
    printf("parent: %s\n", parent);
    strcpy(temp, path);
    strcpy(child, basename(temp));
    printf("child: %s\n", child);

    pino = getino(&dev, parent);
    pip = iget(dev, pino);

    // Check to make sure parent is a dir
    if(!S_ISDIR(pip->INODE.i_mode))
    {
        printf("Error: Not a directory\n");
        return;
    }
    else
    {
        // Check to make sure child does not exist already
        if (search(pip, child) == 0) // Okay to mkdir
        {
            mymkdir(pip, child);
        }
        else
        {
            printf("This directory already exists\n");
            iput(pip);
            return;
        }
    }

    // Increment paren't link count, mark as modified and write to disk
    pip->INODE.i_links_count++;
    pip->dirty = 1;
    pip->INODE.i_atime = time(0L); // touch parent's atime
    iput(pip);
}

int mymkdir(MINODE *pip, char *name)
{
    int ino, bno, i;
    char buf[BLKSIZE], *cp;
    _DIR *dp;
    MINODE *mip;

    // Allocate inode and disk block to insert
    ino = ialloc(dev);
    bno = balloc(dev);


    // Load inode into memory
    mip = iget(dev, ino);


    mip->INODE.i_mode = 0x41ed;
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->pid;
    mip->INODE.i_size = BLKSIZE;
    mip->INODE.i_links_count = 2;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 2;
    mip->INODE.i_block[0] = bno;

    for(i = 1; i < 14; i++)
        mip->INODE.i_block[i] = 0;

    mip->dirty = 1;
    iput(mip);


    // Set dir structure to start of buf area
    dp = (_DIR *)buf;
    cp = buf;

    // Write '.' information to buf
    dp->inode = ino;
    dp->rec_len = 4 * ((8 + strlen(".") + 3) / 4);
    printf(". rec len = %d  ino = %d\n", dp->rec_len, ino);
    dp->name_len = 1;
    strcpy(dp->name, ".");

    // Move to next entry. Set '..' info (parent information)
    cp += dp->rec_len;
    dp = (_DIR *)cp;

    // Write parent info
    printf("pip number: %d", pip->ino);
    dp->inode = pip->ino;
    dp->name_len = 2;
    dp->rec_len = BLKSIZE - 12; // First entry is 12 bytes
    printf(".. rec len = %d   pino = %d\n", dp->rec_len, dp->inode);
    strcpy(dp->name, "..");

    printf("Block number: %d\n", bno);

    put_block(dev, bno, buf);
    enterName(pip, ino, name);
}

// Enter child name into new data block
int enterName(MINODE *pip, int mino, char *name)
{
    char buf[BLKSIZE], buf2[BLKSIZE], *temp, *cp, sbuf[256];
    _DIR *dp;
    int i = 0, need_len, ideal_len, remaining;

    // For each block that is not 0
    for(i = 0; i < 12; i++)
    {
        if(pip->INODE.i_block[i] == 0)
        {
            break;
        }
    }
    i--;

    printf("parent data blk: %d\n", pip->INODE.i_block[0]);
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    printf("********* STEPPING TO LAST ENTRY ************\n");
    // Step to last entry in block
    while(cp + dp->rec_len < buf + BLKSIZE)
    {
        int temp2 = cp + dp->rec_len;
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;
        printf("dir entry: %s\n", dp->name);
        printf("dp rec len: %d\n", dp->rec_len);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    strncpy(sbuf, dp->name, dp->name_len);
    sbuf[dp->name_len] = 0;
    printf("last entry in parent datablock: %s\n", sbuf);
    printf("*************** END ************************\n");

    need_len = 4 * ((8 + dp->name_len + 3) / 4);
    ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
    remaining = dp->rec_len - ideal_len;

    if(remaining >= need_len)
    {
        // Trim last entry to ideal len
        dp->rec_len = ideal_len;
        // Enter new entry
        cp += dp->rec_len;
        dp = (DIR *)cp;
        dp->inode = mino;
        dp->rec_len = remaining;
        dp->name_len = strlen(name);
        strcpy(dp->name, name);
        put_block(dev, pip->INODE.i_block[i], buf);
    }
    else // No space in existing datablock, create new
    {
        i++;
        printf("NO SPACE. ALLOCATING NEW BLOCK!\n");
        int blk = balloc(dev);
        pip->INODE.i_block[i] = blk;
        pip->INODE.i_size += BLKSIZE;
        get_block(dev, blk, buf2);
        dp = (DIR *)buf2;
        dp->inode = mino;
        dp->rec_len = BLKSIZE;
        dp->name_len = strlen(name);
        strcpy(dp->name, name);
        put_block(dev, pip->INODE.i_block[i], buf2);
    }

    pip->dirty = 1;
    iput(pip);
}

// Function to create a file
int creatFile(char *path)
{
    MINODE *pip;
    char parent[256], child[256], temp[256];
    int pino;

    // If path is absolute
    if(path[0] == '/')
    {
        dev = root->dev;
        pip = root;
    }
    else // Path is relative
    {
        dev = running->cwd->dev;
        //pip = running->cwd;
    }

    // Copy to temp first, so we don't destroy our parent and child arrays
    strcpy(temp, path);
    strcpy(parent, dirname(temp));
    printf("parent: %s\n", parent);
    strcpy(temp, path);
    strcpy(child, basename(temp));
    printf("child: %s\n", child);

    pino = getino(&dev, parent);
    pip = iget(dev, pino);

     // Check to make sure parent is a dir
    if(!S_ISDIR(pip->INODE.i_mode))
    {
        printf("Error: Not a directory\n");
        return 0;
    }
    else
    {
        // Check to make sure child does not exist already
        if (search(pip, child) == 0) // Okay to mkdir
        {
            myCreat(pip, child);
            pip->dirty = 1;
            iput(pip);
            return 1;
        }
        else
        {
            printf("This directory already exists\n");
            iput(pip);
            return 0;
        }
    }

}


int myCreat(MINODE *pip, char *name)
{
     int ino, bno, i;
    char buf[BLKSIZE], *cp;
    _DIR *dp;
    MINODE *mip;

    // Allocate inode and disk block to insert
    ino = ialloc(dev);

    // Load inode into memory
    mip = iget(dev, ino);

    // Set the node's information
    mip->INODE.i_mode = 0x81A4;
    printf("mode: %x\n", mip->INODE.i_mode);
    mip->INODE.i_uid = running->uid;
    mip->INODE.i_gid = running->pid;
    mip->INODE.i_size = 0;
    mip->INODE.i_links_count = 1;
    mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 0;

    // Init node blocks as 0
    for(i = 1; i < 14; i++)
        mip->INODE.i_block[i] = 0;

    enterFileName(pip, ino, name);
    mip->dirty = 1;
    iput(mip);
    //printf("ref count in myCreat: %d\n", mip->refCount);

}

// Enters a file name to the paren't data block
int enterFileName(MINODE *pip, int mino, char *name)
{
     char buf[BLKSIZE], buf2[BLKSIZE], *temp, *cp, sbuf[256];
    _DIR *dp;
    int i = 0, need_len, ideal_len, remaining;

    // For each block that is not 0
    // Takes us to the correct block to insert into
    for(i = 0; i < 12; i++)
    {
        if(pip->INODE.i_block[i] == 0)
        {
            break;
        }
    }
    i--;

    printf("parent data blk: %d\n", pip->INODE.i_block[0]);
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    printf("********* STEPPING TO LAST ENTRY ************\n");
    int temp1 = cp + dp->rec_len;
    // Step to last entry in block
    while(cp + dp->rec_len < buf + BLKSIZE)
    {
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;
        printf("dir entry: %s\n", dp->name);
        printf("dp rec len: %d\n", dp->rec_len);
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    strncpy(sbuf, dp->name, dp->name_len);
    sbuf[dp->name_len] = 0;
    printf("last entry in parent datablock: %s\n", sbuf);
    printf("*************** END ************************\n");

    need_len = 4 * ((8 + dp->name_len + 3) / 4);
    ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
    remaining = dp->rec_len - ideal_len;

    // If there is enough space in the block still
    if(remaining >= need_len)
    {
        // Trim last entry to ideal len
        dp->rec_len = ideal_len;
        // Enter new entry
        cp += dp->rec_len;
        dp = (DIR *)cp;
        dp->inode = mino;
        dp->rec_len = remaining;
        dp->name_len = strlen(name);
        strcpy(dp->name, name);
        put_block(pip->dev, pip->INODE.i_block[i], buf);
    }
    else // No space in existing datablock, create new
    {
        i++;
        int blk = balloc(dev);
        pip->INODE.i_block[i] = blk;
        pip->INODE.i_size += BLKSIZE;
        get_block(dev, blk, buf2);
        dp = (DIR *)buf2;
        dp->inode = mino;
        dp->rec_len = BLKSIZE;
        dp->name_len = strlen(name);
        strcpy(dp->name, name);
        put_block(pip->dev, pip->INODE.i_block[i], buf2);
    }


    pip->dirty = 1;
    //iput(pip);
}



