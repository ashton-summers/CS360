#include "fs.h"

int mychmod(char *path, char *newMode)
{
    int ino;
    MINODE *mip;
    char buf[256];

    int mode;

    strcpy(buf, newMode);

    sscanf(buf, "%o", &mode);

    if(path[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    ino = getino(&dev, path);

    if(ino == 0)
    {
        printf("inode specified by path '%s' does not exist\n", path);
        return 0;
    }

    mip = iget(dev, ino);
    mip->INODE.i_mode = mode;
    mip->dirty = 1;
    iput(mip);

    return 1;

}

// Updates the time of an inode to most current time
int myTouch(char *path)
{
    int ino;
    MINODE *mip;
    if(path[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    ino = getino(&dev, path);

    if(ino == 0)
    {
        printf("inode specified by path '%s' does not exist\n", path);
        return 0;
    }

    mip = iget(dev, ino);

    mip->INODE.i_atime = time(0L);
    mip->INODE.i_mtime = time(0L);
    mip->INODE.i_ctime = time(0L);

    mip->dirty = 1;
    iput(mip);

    return 1;
}
