#include "fs.h"

// Finds the command in our function table array
int findCmd(char *command)
{
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

// Init file system
int init()
{
    int i = 0;
    for(i = 0; i < NMINODE; i++)
    {
        minode[i].refCount = 0;
    }
    for (i = 0; i < NFD; i++)
    {
        proc[0].fd[i] = 0;
        proc[1].fd[i] = 0;
    }
    proc[0].pid = 1;
    proc[0].uid = 0;
    proc[0].cwd = 0;
    proc[1].pid = 2;
    proc[1].uid = 1;
    proc[1].cwd = 0;
    originalDev = dev;

     // Get super block info
     // read SUPER block
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;
    nfreeInodes = sp->s_free_inodes_count;
    nfreeBlocks = sp->s_free_blocks_count;
    printf("ninodes=%d nblocks=%d nfreeInodes=%d nfreeBlocks=%d\n",
    ninodes, nblocks, nfreeInodes, nfreeBlocks);

    // Get group descriptor info
    get_block(fd, 2, buf);
    gp = (GD *)buf;

    printf("GD info: %d %d %d %d %d %d\n",
        gp->bg_block_bitmap,
        gp->bg_inode_bitmap,
        gp->bg_inode_table,
        gp->bg_free_blocks_count,
        gp->bg_free_inodes_count,
        gp->bg_used_dirs_count);

    iblock = gp->bg_inode_table;
    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;

    // Add the first mount device to mount table
    mountTable[0].bmap = bmap;
    mountTable[0].imap = imap;
    mountTable[0].dev = dev;
    strcpy(mountTable[0].name, "diskimage");
    mountTable[0].startBlock = iblock;
    mountTable[0].ninodes = ninodes;
    mountTable[0].nblocks = nblocks;
    strcpy(mountTable[0].mountpoint, "/");
    printf("bmap = %d   imap = %d  inode_start(iblock) = %d\n", bmap, imap, iblock);

}

int mount_root()
{
    printf("mount_root()\n");
    root = iget(dev, 2);
    mountTable[0].inode = root;
}

int get_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[ ])
{
  lseek(fd, (long)blk*BLKSIZE, 0);
  write(fd, buf, BLKSIZE);
}


// Accepts an INODE to root directory
// Searches directory for 'name' file
int search(INODE *ip, char * name)
{
    int i, found = 0;
    char * cp;

    if (!ip) {
        printf("Inode argument cannot be null\n");
        exit(1);
    }

    if (!S_ISDIR(ip->i_mode)) {
        printf("INode must be for directory\n");
        exit(2);
    }

    // Look at rootblock
    get_block(fd, ip->i_block[0], dbuf);
    dp = (_DIR *)dbuf;
    cp = dbuf;

    while (cp < &dbuf[BLKSIZE]) {
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;

        // If we found the correct node,
        // store its number
        if (strcmp(sbuf, name) == 0) {
            found = dp->inode;
            return found;
        }

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    return found;
}


// gets an inode's minode and checks if it's
// mounted
MINODE* getmounted(int * dev, MINODE * mip) {
    int entryno;
    MINODE * temp;

    // For downward traversal, look if the current mip is mounted
    // if it is, look in the mounted table for the correct entry,
    // modify the dev#, and return the correct inumber
    if (mip->mounted) {
        printf("Reached mounting point... Looking for mount.\n");
        entryno = findmountpoint(mip);

        if (entryno < 0) { printf("Mount entry not found.\n"); return -1; }

        printf("Mount entry found! Mount entry: %s\n", mountTable[entryno].name);

        *dev = mountTable[entryno].dev;

        printf("Returning inode %d\n", mountTable[entryno].inode->ino);
        temp = iget(*dev, 2);

        return temp;
    }
    // For upward traversal, look if the current mip is root
    // if it us, get the mount entry of mip in the mounttable,
    // then perform a search for its mountlocation (char[64])
    // on the base-system mountpoint. Return this ino, and modify
    // dev# accordingly.
    else if (mip->ino == 2) {

        printf("Device is mounted... Looking for mount point.\n");
        entryno = findmountentry(mip);

        if (entryno < 0) { printf("Mount point not found.\n"); return -1; }

        printf("Mount point found! Mount point: %s\n", mountTable[entryno].mountpoint);

        temp = iget(mountTable[0].dev, mountTable[entryno].mountpoint);

        if (!temp) { printf("Error: Could not get inode at point %s\n", mountTable[entryno].mountpoint); return 0; }

        *dev = mountTable[0].dev;

        return temp;
    }
}

// Looks for an mip inside a mount entry
// returns -1 if not found
int findmountentry(MINODE * mip) {
    int j = 0;
    for (; j < NMOUNT; j++) {
        if (mountTable[j].inode == mip) {
            return j;
        }
    }
    return -1;
}

// looks for a mounting point inside a mount entry
// returns -1 if not found
int findmountpoint(MINODE * mip) {
    if (mip && mip->mptr)
        return ((int)mip->mptr - (int)&mountTable[0]) / sizeof(MountEntry);

    return -1;
}



void tokenize()
{
    char *s;
    int i = 1;
    char temp[256];
    strcpy(temp, pathName);
     s = strtok(temp, "/ \n");
     if (s)
     {
        dirNames[0] = (char *)malloc(strlen(s) + 1);
        strcpy(dirNames[0], s);

        // Parse all dir names from the path and store in array of strings
        while (s = strtok(NULL, "/\n"))
        {
            dirNames[i] = (char *)malloc(strlen(s) + 1);
            strcpy(dirNames[i], s);
            i++;
        }

        dirNames[i] = 0;
        n = i; // n: number of dir names in the path
    }

}

// Change current working directory
int changeDir (char *pathname)
{
    MINODE *mip;
    int ino = getino(&dev, pathname);
    printf("%d\n", ino);
    mip = iget(dev, ino);

    if(!S_ISDIR(mip->INODE.i_mode))
    {
        printf("The pathname entered is not a directory\n");
        //strcpy(prevWd, wd);
        //strcpy(wd, dirname(pathTemp));
        return -1;
    }
    else
    {
        iput(running->cwd); // Dispose of the old cwd
        running->cwd = mip; // new cwd points to inode in memory

    }
}

// Display the contents of the file
int ls(char *pathname)
{
    MINODE *mip;
    DIR *dir;
	struct dirent *file;
	int ino;
	printf("path in ls: %s", pathname);
	if (!pathname[0])
	{
        ino = running->cwd->ino;
	}
	else
	{
        ino = getino(&dev, pathname);
	}
    mip = iget(dev, ino);

    if(S_ISDIR(mip->INODE.i_mode)) // if inode is a dir
    {
        printf("Ls-ing dir...\n");
        return ls_aux(&mip->INODE);
    }
    else // inode is a file
    {
        printf("inode is file...\n");
        printf("%s\n", pathname);
        return;

       ls_file(pathName);
    }
}

int ls_aux(INODE * ip) {
    char lsbuf[BLKSIZE];
    char buf[BLKSIZE], buf2[BLKSIZE];
    char lss[BLKSIZE];
    _DIR * lsdp;
    char * lscp;
    int ino, blk, offset;
    INODE *temp;

    // Look at rootblock
    get_block(dev, ip->i_block[0], lsbuf);
    lsdp = (_DIR *)lsbuf;
    lscp = lsbuf;

    while (lscp < &lsbuf[BLKSIZE]) {
        //printf("Copying...\n");
        char timestr[256];
        time_t time;
        strncpy(lss, lsdp->name, lsdp->name_len);
        lss[lsdp->name_len] = 0;

        ino = search(ip, lss);
        blk = (ino - 1) / 8 + iblock;
        offset = (ino - 1) % 8;

        get_block(dev, blk, buf2);
        temp = (INODE *)buf2 + offset;
        time = temp->i_ctime;

        strcpy(timestr, (ctime(&time)));

        if(temp->i_mode == 0xA)
        {
            readLink(sbuf, buf);
            timestr[strlen(timestr) + 1] = 0;
            printf("   %x    %4d    %4d    %4d %s->%s        %s",
                temp->i_mode, lsdp->inode, temp->i_size, lsdp->rec_len, sbuf, buf, timestr);
            lscp += lsdp->rec_len;
            lsdp = (_DIR *)lscp;
        }

        else
        {
            timestr[strlen(timestr) + 1] = 0;
        //printf("Name: '%d' Check: '%d'\n", sbuf[0], name[0]
            printf("%x    %4d    %4d    %4d  %s        %s",
                temp->i_mode, lsdp->inode, temp->i_size, lsdp->rec_len, lss, timestr);
            lscp += lsdp->rec_len;
            lsdp = (_DIR *)lscp;
        }
    }
    printf("Exiting lsAux...\n");
}


// Prints the current working dir
void pwd(MINODE *cwd, char name[], int i)
{
    MINODE *mip;
    INODE *ip;
    char buf[1024], *cp, temp[256];
    int ino, pino = 0, disp, blk, j = 0;
    int childino = cwd->ino;

    pino = search(&cwd->INODE, ".."); //get parent ino

    ip = mip = iget(dev, pino); // get the parent inode into memory

    if(cwd->ino == 2)
    {
        if (i == 0) // if i parameter is 0, we have not recursed and started at root
        {
            printf("/");
        }
        return;
    }

    // Look at rootblock
    get_block(fd, ip->i_block[0], dbuf);
    dp = (_DIR *)dbuf;
    cp = dbuf;
    // printf("name in search: %s\n", dp->name);

    while (cp < &dbuf[BLKSIZE])
    {
        strncpy(sbuf, dp->name, dp->name_len);
        sbuf[dp->name_len] = 0;

        // Store the name for recursive call
        // if the ino == child ino
        if (dp->inode == childino) {
            strcpy(temp, sbuf);
            strcpy(name, sbuf);
        }
        cp += dp->rec_len;
        dp = (_DIR *)cp;
    }

    pwd(mip, &temp[0], i + 1);
    printf("/%s", name);
}

void printMenu()
{
    char *ls = "ls", *cd = "cd", *pwd = "pwd", *open = "open", *close = "close",
    *read = "read", *write = "write", *chmod = "chmod", *lseek = "lseek", *printMenu = "menu",
    *touch = "touch", *mv = "mv", *cp = "cp", *cat = "cat", *pfd = "pfd", *link = "link",
    *unlink = "unlink", *creat = "creat", *rmdir = "rmdir", *rm = "rm", *quit = "quit",
    *mkdir = "mkdir", *symlink = "symlink";

    int i = 0;

    for(i = 0; i < 13; i++)
    {
        printf("*");
    }


    printf(" AVAILABLE COMMANDS ");

    for(i = 0; i < 13; i++)
    {
        printf("*");
    }


    printf("\n");

    printf("|");
    printf("          %s  %s  %s  %s                 |\n",
    ls, cd, pwd, open);

    printf("|");
    printf("          %s  %s  %s  %s          |\n",
    open, close, read, write);

    printf("|");
    printf("          %s  %s  %s  %s         |\n",
    chmod, lseek, printMenu, touch);

    printf("|");
    printf("          %s  %s  %s  %s                  |\n",
    mv, cp, cat, pfd);

    printf("|");
    printf("          %s  %s  %s  %s      |\n",
    link, unlink, symlink, creat);

    printf("|");
    printf("          %s  %s  %s  %s            |\n",
    rmdir, rm, quit, mkdir);


    for(i = 0; i < 46; i++)
    {
        printf("*");
    }

    printf("\n");

}




