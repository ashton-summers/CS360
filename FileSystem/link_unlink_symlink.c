#include "fs.h"

int myLink(char *oldFileName, char *newFileName)
{
    int oldINO = -1, newINO = -1, pino = -1;
    MINODE *oldmip,*pip;
    char newParentName[256], newChildName[256], oldChildName[256], oldParentName[256];
    char temp[256];

    // If path is absolute
    if(oldFileName[0] == '/')
    {
        dev = root->dev;
    }
    else // Path is relative
    {
        dev = running->cwd->dev;
        //pip = running->cwd;
    }

    oldINO = getino(&dev, oldFileName);
    if(oldINO == 0)
    {
        printf("%s does not exist\n", oldFileName);
        return 0;
    }
    oldmip = iget(dev, oldINO);

    // Parse the all child names and parent names in paths
    strcpy(temp, oldFileName);
    strcpy(oldParentName, dirname(temp));
    strcpy(temp, oldFileName);
    strcpy(oldChildName, basename(temp));
    strcpy(temp, newFileName);
    strcpy(newParentName, dirname(temp));
    strcpy(temp, newFileName);
    strcpy(newChildName, basename(temp));

    pino = getino(&dev, newParentName);
    if(pino == 0)
    {
        printf("Error: New directory does not exist\n");
        return 0;
    }

    pip = iget(dev, pino);

    printf("child = %s   parent = %s\n", oldChildName, oldParentName);


    // If the node is not reg or a lnk
    if(!S_ISREG(oldmip->INODE.i_mode))
    {
        printf("Error: Cannot link to a directory\n");
        return 0;
    }
    // Make sure new file does not already exist
    if(search(pip, newChildName) != 0)
    {
        printf("Error: The file you are trying to add to path '%s' already exists\n", newParentName);
        return 0;
    }
    if(oldmip->dev != pip->dev)
    {
        printf("Error: paths '%s' and '%s' are not on the same device\n", oldFileName, newFileName);
        return 0;
    }

    //creatLink(pip, newChildName, oldINO);
    enterFileName(pip, oldINO, newChildName);
    oldmip->INODE.i_links_count++;
    oldmip->dirty = 1;
    pip->dirty = 1;
    iput(pip);
    iput(oldmip);
    printf("ref count: %d\n", oldmip->refCount);
    return 1;

}

// Unlinks a file link given a path name
// TODO: Deallocate all blocks if link count == 0;
int myUnlink(char *path)
{
    int ino = -1, pino, i;
    char parentName[256], childName[256], temp[256];
    MINODE *mip, *pip;

    if (path[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }
    // Get path inode into memory
    ino = getino(&dev, path);
    mip = iget(dev, ino);

    strcpy(temp, path);
    strcpy(parentName, dirname(temp));
    strcpy(temp, path);
    strcpy(childName, basename(temp));

    printf("MODE:%x\n", mip->INODE.i_mode);
    if(ino == 0)
    {
        printf("Error: file does not exist\n");
        return 0;
    }
    if (S_ISREG(mip->INODE.i_mode) || mip->INODE.i_mode == 0xA);
    {

        printf("Removing child\n");
        pino = getino(&dev, parentName);
        pip = iget(dev, pino);

        mip->INODE.i_links_count--;
        rmChild(pip, childName);
        if (mip->INODE.i_links_count == 0 && mip->INODE.i_mode != 0xA)
        {
            myTruncate(mip);
            idealloc(mip->dev, mip->ino);
        }
        mip->dirty = 1;
        iput(mip);
        pip->dirty = 1;
        //printf("ref count: %d\n", mip->refCount);
        iput(pip);

        return 1;
    }

    printf("Either not type LNK or REG\n");
    return 0;
}

// Creates a file that is a symbolic link between oldFile and newFile
int mySymLink(char *oldFileName, char *newFileName)
{
    int oldINO, newINO, pino;
    char temp[256], oldParentName[256], newParentName[256], newChildName[256], oldChildName[256];
    MINODE *oldmip, *pip, *newmip;

    if(oldFileName[0] == '/')
    {
        dev = root->dev;
    }
    else
    {
        dev = running->cwd->dev;
    }

    // Parse the all child names and parent names in paths
    strcpy(temp, oldFileName);
    strcpy(oldParentName, dirname(temp));
    strcpy(temp, oldFileName);
    strcpy(oldChildName, basename(temp));
    strcpy(temp, newFileName);
    strcpy(newParentName, dirname(temp));
    strcpy(temp, newFileName);
    strcpy(newChildName, basename(temp));

    // Get ino of oldFileName
    oldINO = getino(&dev, oldFileName);

    if(oldINO == 0)
    {
        printf("%s does not exist\n", oldFileName);
        return 0;
    }

    // Get oldFileName inode into memory
    oldmip = iget(dev, oldINO);

    // Get ino of parent (newFileName)
    pino = getino(&dev, newParentName);

    if(pino == 0)
    {
        printf("Dir %s does not exist\n", newParentName);
    }

    // Get parent directory inode (block of newFileName
    pip = iget(dev, pino);


     // If the node is not reg or a lnk
    if(!S_ISREG(oldmip->INODE.i_mode))
    {
        printf("Error: Cannot link to a directory\n");
        return 0;
    }

    // Make sure new file does not already exist
    if(search(pip, newChildName) != 0)
    {
        printf("Error: The file you are trying to add to path '%s' already exists\n", newParentName);
        return 0;
    }
    if(oldmip->dev != pip->dev)
    {
        printf("Error: paths '%s' and '%s' are not on the same device\n", oldFileName, newFileName);
        return 0;
    }

    // Create the file.
    creatFile(newFileName);


    // Get the new file's inode into memory.
    newINO = search(pip, newChildName);
    newmip = iget(dev, newINO);

    printf("mode before: %x\n", newmip->INODE.i_mode);

    newmip->INODE.i_mode = 0xA;
    newmip->INODE.i_size = strlen(oldChildName);

    // Copy the new child name to the block of the new file
    strcpy(temp, oldChildName);
    strcpy((char *)newmip->INODE.i_block, temp);

    newmip->dirty = 1;
    iput(newmip);
    pip->dirty = 1;
    iput(pip);

}

// Reads the contents of a symbolically linked file.
int readLink(char *file, char *buf)
{
    int ino;
    MINODE *mip;

    ino = getino(&dev, file);
    if(ino == 0)
    {
        printf("File '%s' does not exist\n", file);
        return 0;
    }

    mip = iget(dev, ino);

    if(mip->INODE.i_mode != 0xA)
    {
        printf("Error: This is not a link type\n");
        return 0;
    }
    // Copy the link contents from i_block
    strcpy(buf, (char *)mip->INODE.i_block);
    return strlen(buf);
}

