#include "Lab2.h"


/* GLOBAL VARIABLES  */
Node *root = NULL, *cwd = NULL;                             /* root and CWD pointers */
char line[128];                               /* user input line */
char command[16], pathName[64];               /* user inputs */
char dirName[64], baseName[64];
FILE *myFile;

char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
               "quit", "help", "?", "menu", "reload", "save", 0};

/* END GLOBALS */


int main()
{
    //rmdir,ls, cd, pwd,creat,rm};
    int (*fptr[ ])(char *) = {(int (*)())mkdir, rmdir, ls, cd, pwd,
                                        creat, rm, quit, help, help,
                                        help, reload, save};
    char s[64];
    int index = 0;

    //init root node
    root = initRoot();
    cwd = root;
    if (root)
    {
        printf("Root Initialized OK\n");
    }
    printf("Enter ? for help menu\n");

    while (1)
    {
        printf("Command: ");
        fgets(s, 100, stdin);
        sscanf(s, "%s %s", command, pathName);
        if (strcmp(command, "quit") == 0){ break;}
        index = findCmd(command);

        if (index < 0)
        {
            printf("invalid command\n");
        }
        else
        {
           int r = fptr[index](pathName);
        }
        memset(pathName, 0, sizeof(pathName));
        memset(command, 0, 64);
        memset(s, 0, 64);

    }

    save();
    deleteTree(root);

    return 0;
}


/*
returns the index of the input cmd from
the cmd table
*/
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

//initialize root values
initRoot()
{
    root = (Node *)malloc(sizeof(Node));
    strcpy(root->name, "/");
    root->type = 'D';
    root->childPtr = 0;
    root->parentPtr = root;
    root->siblingPtr = 0;

}

//adds a dir to the cwd
void mkdir (char *path)
{
    char *s;
    char c = path[0];
    Node *cur = root, *temp = NULL;

    //tokenize the path name
    tokenize(path);
    //check to see if the path is relative
    if (c != '/')
    {
        cur = cwd;
    }
    //iterate for every token in dirName
        cur = searchPathName(cur);
        traceMessages(cur, baseName);
        if (cur)
        {
            temp = search(cur->childPtr, baseName); //see if node already exists
        }
        //if node exists, return and print
        if (temp)
        {
            printf("This directory already exists\n");
            return;
        }
        else //otherwise insert at the end of the list
        {
            if (cur && cur->type == 'D')
            {
                insertAtEnd(cur, 'D');
                printf("insert %s into %s\n", baseName, cur->name);
                printf("------ mkdir OK ------\n");
            }
        }
}

//removes a directory given a path name
void rmdir (char *path)
{
    Node *cur = cwd;
    char c = path[0];
    tokenize(path);
    if (c == '/')
    {
        cur = root;
    }
    cur = searchPathName(cur);
    traceMessages(cur, baseName);
    Node *temp = search(cur->childPtr, baseName);
    if (temp && temp->type == 'D' && temp->childPtr == NULL)
    {
        cur->childPtr = removeNthNode(cur->childPtr, temp);
        printf("remove %s from %s\n", baseName, cur->name);
        printf("------ rmdir OK -------\n");
    }
    else
    {
        printf("Directory either is not empty or is not of type 'D'\n");
    }

}

//removes the nth node of a linked list
Node *removeNthNode(Node *start, Node *n)
{
    Node *cur = start, *prev = NULL;

    while (strcmp(cur->name, n->name) != 0)
    {
        prev = cur;
        cur = cur->siblingPtr;
    }
    if (prev != NULL)
    {
        prev->siblingPtr = cur->siblingPtr;
        free(cur);
        cur = NULL;
        return start;
    }
    else //deleting at the front of the list
    {
        Node *temp = cur->siblingPtr;
        free(cur);
        cur = NULL;
        return temp;
    }

}

//searches for a node that matches name == str
Node *search(Node *current, char *str)
{
    Node *temp = current;
    while (temp)
    {
        if (strcmp(temp->name, str) == 0)
        {
            return temp;
        }
        else
        {
            temp = temp->siblingPtr;
        }
    }

    return NULL;
}

//inserts a node at the end of a linked list
void insertAtEnd(Node *start, char type)
{
    Node *prev = NULL, *cur = start, *parent = start;

    if (cur->childPtr != NULL)
    {
        cur = cur->childPtr;
        while (cur->siblingPtr != NULL) //traverse the linked list to the end
        {
            prev = cur;
            cur = cur->siblingPtr;
        }

        //allocate memory for the new node, initialize node values
        cur->siblingPtr = makeNode(parent, type);
    }
    else //there are no children in the cwd
    {
        cur->childPtr = makeNode(parent, type);
    }

}

//lists the directory contents of pathname or cwd
void ls (char *path)
{
    Node *cur = cwd;
    tokenize(path);
    char *s = strtok(dirName, "/");

    printf("**** ls: \n");

    if (path[0] == 0) // if there is no path, list cwd contents
    {
        cur = cwd->childPtr;
        printList(cur);
    }
    else
    {
        //for every token
        cur = searchPathName(cur);
        cur = search(cur->childPtr, baseName); //set to child's child that way we are in the right spot

        if (cur)
        {
            cur = cur->childPtr;
            printList(cur);
        }
        else
        {
            printf("This directory does not exist\n");
        }
    }
}

//tokenizes a given path from the user
void tokenize(char *path)
{
     char buf[64];

    strcpy(buf, path);
    strcpy(dirName, dirname(buf));
    strcpy(buf, path);
    strcpy(baseName,basename(buf));
}

//changes to the current working directory
void cd (char *path)
{
    char *s, c = path[0];
    Node *cur = root;
    tokenize(path);

    printf("------ cd to %s ------\n", pathName);

    if (pathName[0] == 0) // no path
    {
        cwd = root;
        return;
    }
    else if (c == '/') //if absolute path is given, start from root
    {
        cur = root;
    }
    else
    {
        cur = cwd;
    }
    cur = searchPathName(cur);
    traceMessages(cur, baseName);
    cur = search(cur->childPtr, baseName);
    if (cur)
    {
        printf("------ cd OK ------ \n");
        cwd = cur;
    }
    else
    {
        printf("This path does not exist\n");
    }
}

//calls recursive print function
void pwd()
{
    printf("------ pwd ------\n");
    if (cwd == root)
        printf("/\n");
    else
        rpwd(cwd);
        printf("\n");
}

//recursively prints path of cwd
void rpwd(Node *cur)
{
    if (cur->parentPtr == cur)
    {
        return;
    }
    rpwd(cur->parentPtr);
    printf("/%s", cur->name);
}

//prints a singly list list
void printList(Node *cur)
{
    while (cur)
    {
        printf("%c   %s\n", cur->type, cur->name);
        cur = cur->siblingPtr;
    }
}

//creates a file node and adds to directory
void creat(char *path)
{
    char *s;
    char c = '\0';
    Node *cur = root, *temp;

    //tokenize the path name
    tokenize(path);

    s = strtok(dirName, "/");     // break up "a/b/c/d" into tokens
    c = path[0];
    //check to see if the path is relative
    if (c != '/')
    {
        cur = cwd;
    }
    cur = searchPathName(cur);
    traceMessages(cur, baseName);

    if (cur)
    {
        temp = search(cur->childPtr, baseName); //see if node already exists
    }

    //if node exists, return and print
    if (temp)
    {
        printf("This directory already exists\n");
        return;
    }
    else //otherwise insert at the end of the list
    {
        if (cur && cur->type == 'D')
        {
            printf("insert %s into %s\n", baseName, cur->name);
            insertAtEnd(cur, 'F');
            printf("------ creat OK ------\n");
        }
    }
}

//removes a file node given a path
void rm (char *path)
{
    Node *cur = root;
    char c = path[0];
    char *s;
    tokenize(path);
    if (c != '/')
    {
        cur = cwd;
    }
    cur = searchPathName(cur);
    traceMessages(cur, baseName);
    Node *temp = search(cur->childPtr, baseName);

    if (temp && temp->type == 'F' && temp->childPtr == NULL)
    {
        printf("remove %s from %s\n", baseName, cur->name);
        cur->childPtr = removeNthNode(cur->childPtr, temp);
    }
    else
    {
        printf("Directory either does not exist or is not of type 'F'\n");
    }
}

//saves the current tree structure to a file
void save()
{
    Node *temp = root;
    myFile = fopen("myFile", "w+");
    fprintf(myFile, "%c   %s\n", temp->type, temp->name);
    if (temp->childPtr != 0)
    {
        saveHelper(temp->childPtr);
    }
    printf("------ Save OK ------ \n");
    fclose(myFile);
}
//saves contents of tree structure to a file
void saveHelper (Node *cur)
{
    if (cur && myFile)
    {
        fprintf(myFile, "%c ", cur->type);
        rpwdf(cur, myFile);
        fprintf(myFile, "\n");
        saveHelper(cur->childPtr);
        saveHelper(cur->siblingPtr);
    }
}

//prints a help menu
void help()
{
    printf("==========================  MENU  ===============================\n");
    printf("mkdir  rmdir  ls  cd  creat  rm  help  menu  save  reload  quit\n");
    printf("================================================================\n");
}

//saves current state of the tree before exiting program
void quit()
{
}


//calls the load helper function
void reload ()
{
    char filename[64];

    deleteTree(root->childPtr);
    initRoot();
    printf("\n");
    printf("Enter a filename: ");
    scanf(" %s", filename);
    myFile = fopen (filename, "r");
    if (myFile)
    {
        reloadHelper();
        fclose(myFile);
    }
    else
    {
        printf("Failed to open file\n");
    }
}


void reloadHelper()
{
    char name[64], line[50], t[10];
    char type;
    root = initRoot();
    cwd = root;

    if (root)
    {
        printf("Root initialized OK\n");
    }

    while(fgets(line, 45, myFile))
    {
        sscanf(line, "%s   %s", t, name);
        type = t[0];
        if(type == 'D' && strcmp(name, "/") != 0)
        {
            mkdir(name);
        }
        else if (type == 'F')
        {
            creat(name);
        }
    }

}

//deletes the current tree so new data can be loaded
void deleteTree(Node *pRoot)
{
    if (pRoot)
    {
        deleteTree(pRoot->childPtr);
        deleteTree(pRoot->siblingPtr);
        free(pRoot);
    }
}

//prints current working dir to a file
//helper print function for save
void rpwdf(Node *cur, FILE *filename)
{
    if (cur && (cur->parentPtr == cur))
    {
        return;
    }
    rpwdf(cur->parentPtr, filename);
    fprintf(filename, "/%s", cur->name);
}

/*when using mkdir or any other operation that traverses the tree,
trace messages to show the current place in the operation
*/
void traceMessages(Node *cur, char *dir)
{
    printf("Searching for %s in %s\n", dir, cur->name);
}

//creates and returns a newly allocated node
Node *makeNode (Node *parent, char type)
{
    Node *pMem = NULL;
    pMem = (Node *)malloc(sizeof(Node));
    strcpy(pMem->name, baseName);
    pMem->parentPtr = parent;
    pMem->siblingPtr = 0;
    pMem->type = type;
    pMem->childPtr = 0;

    return pMem;
}

//returns where cur is in the tree after searching all tokens in dirName
Node *searchPathName(Node *cur)
{
    char *s;
    s = strtok(dirName, "/");
    while(s && s[0] != '.')
    {
        traceMessages(cur, s);
        cur = search(cur->childPtr, s); //search for the current node name
        if (!cur)
        {
            printf("This directory does not exist\n");
            return NULL;
        }
        else if (cur && cur->type != 'D')
        {
            printf("This is not a 'D' type\n");
            return NULL;
        }

        s = strtok(NULL, "/");
    }

    return cur;
}
