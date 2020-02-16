#include "fs.h"

// Mounts file system to a new mounting point
int mount(char* name, char *mountPoint)
{
    int i = 0, openMountSlot = 0, fd = 0, pino, mdev;
    char buf[BLKSIZE], dirname[128], temp[128];
    SUPER *msp;
    GD *mgp;
    MountEntry entry;
    MINODE *mip;

    // If mount with no parameters, print all mount devices
    if (name[0] == '\0')
    {
        for(i = 0; i < NMOUNT; i++)
        {
            if (strcmp(mountTable[i].name, "") != 0)
            {
                printf("mountTable[%d]: %s mounted to %s\n", i, mountTable[i].name, mountTable[i].mountpoint);
            }
        }
        return -1;
    }


    // Check to see if name is already mounted
    for (i = 0; i < NMOUNT; i++)
    {
        if (mountTable[i].name && strcmp(name, mountTable[i].name) == 0)
        {
            printf("The filesystem with name specified has already been mounted\n");
            return -1;
        }
    }

    for (i = 0; i < NMOUNT; i++)
    {
        if (strcmp(mountTable[i].name, "") == 0)
        {
            break;
        }
    }

    openMountSlot = i;

    fd = open(name, O_RDWR);
    if(fd < 0)
    {
        printf("File failed to open\n");
        return -1;
    }


    mdev = fd;
    get_block(fd, 1, buf);
    msp = (SUPER *)buf;

    if(msp->s_magic != 0xEF53)
    {
        printf("This is not an ext2 filesystem\n");
        return 1;
    }

    entry.ninodes = msp->s_inodes_count;
    entry.nblocks = msp->s_blocks_count;
    get_block(mdev, 2, buf);

    mgp = (GD *)buf;

    entry.bmap = mgp->bg_block_bitmap;
    entry.imap = mgp->bg_block_bitmap;
    entry.startBlock = mgp->bg_inode_table;
    printf("start block: %d\n", mgp->bg_inode_table);
    entry.dev = mdev;
    strcpy(entry.name, name);

    strcpy(entry.mountpoint, mountPoint);
    mountTable[openMountSlot] = entry;

    pino = getino(&dev, mountPoint);
    mip = iget(dev, pino);
    entry.inode = mip;

    // Check to make sure mount point is dir
    printf("%x\n", mip->INODE.i_mode);
    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Cannot mount. %s is not a directory\n", mountPoint);
        return -1;
    }

    // Make sure mount point is not busy
    if (mip == running->cwd)
    {
        printf("Cannot mount: %s is in use\n", mountPoint);
    }

    mountTable[openMountSlot].inode = mip;

    printf("ino: %d  dev: %d\n", mountTable[1].inode->ino, mountTable[1].dev);
    mip->mounted = 1;
    mip->refCount++;
    mip->mptr = &mountTable[openMountSlot];
    mip->dirty = 1;

    iput(mip);

    return 0;

}

// Unmounts file system from mounting point
int unmount (char *filesystem)
{
    int i = 0, flag = -1, ino = 0, mountSlot = 0;
    MINODE *mip;

    // Search mount table to make sure the filesys is mounted
    for(i = 0; i < NMOUNT; i++)
    {
        if(strcmp(mountTable[i].name, filesystem) == 0)
        {
            flag = 1;
            mountSlot = i;
        }
    }

    if (flag < 0)
    {
        printf("The specified file system is not mounted\n");
        return -1;
    }


    mip = mountTable[mountSlot].inode;
    strcpy(mountTable[mountSlot].name, "");

    // Make sure the file system is not still active
    for (i = 0; i < NMINODE; i++)
    {
        if(mountTable[mountSlot].dev == minode[i].dev)
        {
            printf("dev: %d  dev m: %d\n", minode[i].dev, mountTable[mountSlot].dev);
            printf("Cannot unmount. File system still active\n");
            return -1;
        }
    }

    mip->mounted = 0;
    iput(mip);


    return 0;

}









