#include "fs.h"
char line[128], parameter[256];
char *disk = "diskimage";
char name[256];
int openMode, newMode;

int main(int argc, char *argv[])
{
    int i = 0;
    strcpy(prevWd, "/");
    if (argc > 1)
        disk = argv[1];

    if ((dev = fd = open(disk, O_RDWR)) < 0)
    {
        printf("open %s failed\n", disk);
        exit(1);
    }

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    printf("Checking ext2 FS...");
    if (sp->s_magic != 0xEF53) {
        printf("Magic: %x; this is not an EXT2 fs\n", sp->s_magic);
        exit(2);
    }
    else
        printf("ext2 FS OK!\n");

    // Init file system
    init();
    mount_root();

    printf("creating P0 as running process\n");
    running = &proc[1];
    running->cwd = iget(dev, 2);

    printMenu();

    while(1)
    {       // command processing loop
       printf("input command : ");

       fgets(line, 128, stdin);
       line[strlen(line) + 1] = 0;
       if (strcmp(line, "\n") == 0) // if user entered only \r
          continue;

       //Use sscanf() to extract cmd[ ] and pathname[] from line[128]
       sscanf(line, "%s %s %64c", cmd, pathName, parameter);

       if (strcmp(pathName, ".") != 0) // If path is .., don't copy another path
       {
            strcpy(pathTemp, pathName);
       }

       printf("cmd=%s pathname=%s  parameter = %s\n", cmd, pathName, parameter);
       tokenize();
       printf("n = %d  dirName[0] = %s\n", n, dirNames[0]);

        parameter[strlen(parameter) - 1] = 0;
       // execute the cmd
       if (strcmp(cmd, "ls")==0)
       {
          ls(pathName);
       }
       else if (strcmp(cmd, "cd")==0)
          changeDir(pathName);
       else if (strcmp(cmd, "pwd")==0)
       {
           printf("************** PWD ******************\n");
          pwd(running->cwd, &name, i);
          printf("\n************** END PWD ******************\n");
       }
       else if(strcmp(cmd, "mkdir") == 0)

            makeDir(pathName);
       else if (strcmp(cmd, "quit")==0)
       {
            return 0;
       }
       else if (strcmp(cmd, "creat") == 0)
            creatFile(pathName);
       else if(strcmp(cmd, "rmdir") == 0)
            rmDir(pathName);
       else if(strcmp(cmd, "link") == 0)
       {
            myLink(pathName, parameter);
       }
       else if(strcmp(cmd, "unlink") == 0)
       {
            myUnlink(pathName);
       }
       else if(strcmp(cmd, "symlink") == 0)
       {
            mySymLink(pathName, parameter);
       }
       else if(strcmp(cmd, "open") == 0)
       {
            openFile(pathName, atoi(parameter));
       }
       else if(strcmp(cmd, "close") == 0)
       {
            int fd = atoi(pathName);
            closeFile(fd);
       }
       else if(strcmp(cmd, "pfd") == 0)
       {
            pfd();
       }
       else if(strcmp(cmd, "read") == 0)
       {
            readFile(atoi(pathName), atoi(parameter));
       }
       else if (strcmp(cmd, "write") == 0)
       {
            int fd = atoi(pathName);

            writeFile(fd, parameter);
       }
       else if (strcmp(cmd, "cat") == 0)
       {
            myCat(pathName);
       }
       else if(strcmp(cmd, "cp") == 0)
       {
            myCp(pathName, parameter);
       }
       else if(strcmp(cmd, "mv") == 0)
       {
            parameter[strlen(parameter) + 1] = 0;
            myMV(pathName, parameter);
       }
       else if (strcmp(cmd, "chmod") == 0)
       {
            int mode = atoi(parameter);
            mychmod(pathName, parameter);
       }
       else if(strcmp(cmd, "touch") == 0)
       {
            myTouch(pathName);
       }
       else if(strcmp(cmd, "menu") == 0)
       {
            printMenu();
       }
       else if(strcmp(cmd, "rm") == 0)
       {
            myUnlink(pathName);
       }
       else if(strcmp(cmd, "lseek") == 0)
       {
            mylseek(atoi(pathName), atoi(parameter));
       }
       else if(strcmp(cmd, "mount") == 0)
       {
            mount(pathName, parameter);
       }
       else if(strcmp(cmd, "umount") == 0)
       {
            unmount(pathName);
       }



       memset(pathName, 0, 256);
      }

}

