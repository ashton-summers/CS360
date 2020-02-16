
#include "Lab6.h"
char *disk = "mydisk";
char diskName[256];
char path[256];
char *s;

int main(int argc, char *argv[ ])
{
    int i = 1;

    printf("Enter a disk image name and a path name\n");
    fgets(buf, 128, stdin);

    // Parse the diskName from input
    strcpy(diskName,strtok(buf, " "));
    fd = open(diskName, O_RDONLY);
    if (fd < 0){
        printf("open failed\n");
        exit(1);
    }

    // Parse the path from input
    strcpy(path, strtok(NULL, " \n"));
    if (strcmp(path, "/") == 0)
    {
        findDir();
        return;
    }
    s = strtok(path, "/ \n");
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
    findDir();

}
