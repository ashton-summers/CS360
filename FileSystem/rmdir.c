#include "fs.h"
int rmDir(char *path)
{
    int ino, i = 0, pino, isEmpty;
    char parentName[256], childName[256], temp[256];
    MINODE *mip, *pip;

    // If user is trying to delete root
    if(strcmp(path, "/") == 0)
    {
        printf("Cannot delete root\n");
        return 0;
    }
    if(path[0] == '/') // If path is absolute
    {
        dev = root->dev;
        //pip = root;
    }
    else // Path is relative
    {
        dev = running->cwd->dev;
    }

    // Break the pathname into parent and child name
    strcpy(temp, path);
    strcpy(parentName, dirname(temp));
    strcpy(temp, path);
    strcpy(childName, basename(temp));
    printf("parent name: %s    child name: %s\n", parentName, childName);

    // Get inumber of the path and inode into memory
    pino = getino(&dev, parentName);
    ino = getino(&dev, temp);
    mip = iget(dev, ino);
    pip = iget(dev, pino);

    if(ino == 0)
    {
        printf("'%s' does not exist\n", temp);
        return 0;
    }


    if (dirIsEmpty(mip) == 0)
    {
        printf("This directory is not empty\n");
        return;
    }

    // Check to make sure use is super user or the uid of user and mip to remove match
    if(running->uid == mip->INODE.i_uid || running->uid == 0)
    {

        printf("mode = %x   links count = %d   refCount = %d\n", mip->INODE.i_mode, mip->INODE.i_links_count, mip->refCount);

        // If not a dir, is not empty, or is busy then iput and return -1
        if(!S_ISDIR(mip->INODE.i_mode) || mip->INODE.i_links_count > 2)
        {
            printf("Error: Either not a dir, busy, or contains data\n");
            iput(mip);
            return -1;
        }

        for(i = 0; i < 12; i++)
        {
            if(mip->INODE.i_block[i] == 0)
            {
                continue;
            }
            bdealloc(mip->dev, mip->INODE.i_block[i]);
        }

        // Deallocate the node and write back to disk
        idealloc(mip->dev, mip->ino);
        iput(mip);

        pip = iget(mip->dev, pino);
        rmChild(pip, childName);
        incFreeInodes(mip->dev);
        pip->INODE.i_links_count--;
        pip->dirty = 1;
        iput(pip);
    }
    else
    {
        printf("You do not have permission to remove this directory\n");
        return 0;
    }

    }

// Removes the child name from parent directory
int rmChild(MINODE *pip, char *name)
{
    int i = 0;
    _DIR *cur, *prev, *temp;
    char *cp, buf[BLKSIZE], sbuf[256], *cpTemp;
    char shiftBuf[BLKSIZE];
    INODE *ip;

    // Search direct data blocks of parent
    for(i = 0; i < 12; i++)
    {
        get_block(pip->dev, pip->INODE.i_block[i], buf);
        cur = (DIR *)buf;
        prev = cur;
        cp = buf;

        // Step through records to find the child name
        while(cp < &buf[BLKSIZE])
        {
            strncpy(sbuf, cur->name, cur->name_len);
            sbuf[cur->name_len] = 0;
            printf("name: %s\n", sbuf);

            // If we have found the name, remove it
            if (strcmp(sbuf, name) == 0)
            {
                 // If the last entry in the datablock
                if(cp + cur->rec_len >= buf + BLKSIZE)
                {
                    // Add record length of entry to remove to previous entry's rec len
                    prev->rec_len += cur->rec_len;
                    put_block(pip->dev, pip->INODE.i_block[i], buf);
                    return 1;
                }
                else if(cur->rec_len == BLKSIZE) // Only dir in data block
                {
                    for (i = 0; i < 12; i++)
                    {
                        pip->INODE.i_block[i + 1] = pip->INODE.i_block[i];
                    }
                    bdealloc(pip->dev, pip->INODE.i_block[i]);
                    put_block(pip->dev, pip->INODE.i_block[i], buf);
                    pip->INODE.i_size -= BLKSIZE;
                    pip->INODE.i_block[i] = 0;
                }
                else // Middle entry in the datablock
                {
                    cpTemp = cp;
                    temp = (DIR *)cp;
                    // Step to last entry so we can add removed dir's length to end
                    while(cpTemp + temp->rec_len < buf + BLKSIZE)
                    {
                        cpTemp += temp->rec_len;
                        temp = (DIR *)cpTemp;
                    }

                    // Add length to last entry
                    char *newCurCp = cp;
                    char buf2[BLKSIZE];
                    temp->rec_len += cur->rec_len;
                    cp += cur->rec_len;
                    temp = (DIR *)cp;
                    memcpy(buf2, temp, BLKSIZE - cur->rec_len);

                    // Shift all entries after removed dir to the left
                    memcpy(newCurCp, buf2, BLKSIZE - cur->rec_len);
                    put_block(pip->dev, pip->INODE.i_block[i], buf);
                    return 1;

                }
            }
            else
            {
                cp += cur->rec_len;
                prev = cur;
                cur = (DIR *)cp;
            }
        }

    }
}

// Check whether a dir is empty or not
int dirIsEmpty(MINODE *mip)
{
    int blk = 0, offset = 0;
    char *cp, sbuf[256];
    _DIR *dp;
    INODE *ip;

    // Calculate block and node of mip and point ip to it
    blk = (mip->ino - 1) / 8 + iblock;
    offset = (mip->ino - 1) % 8;
    get_block(mip->dev, blk, buf);
    ip = (INODE *)buf + offset;

    // Read in the root block of ip into buf
    get_block(mip->dev, ip->i_block[0], buf);

    dp = (DIR *)buf;
    cp = buf;

    // Step to 3rd record, if null then the dir is empty
    cp += dp->rec_len;
    dp = (DIR *)cp;
    cp += dp->rec_len;
    dp = (DIR *)cp;
    strncpy(sbuf, dp->name, dp->name_len);
    sbuf[dp->name_len] = 0;

    // If a name other than '.' or '..' has been read to sbuf
    if(sbuf[0] != '\0')
    {
        return 0;
    }
    else
    {
        return 1;
    }


}
