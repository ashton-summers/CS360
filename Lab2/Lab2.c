#include "Lab2.h"

char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
               "quit", "help", "?", "menu", "reload", "save", 0};

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

Node *initRoot()
{
    Node *pMem;
    pMem = (Node *)malloc(sizeof(pMem));
    strcpy(pMem->name, "/");
    pMem->type = 'D';
    pMem->childPtr = 0;
    pMem->parentPtr = pMem;
    pMem->siblingPtr = 0;

    return pMem;
}

//adds a dir to the cwd
void mkdir (char *path)
{
    char *s;
    char c = '\0';
    Node *cur = root;

    //tokenize the path name
    tokenize(path);

    s = strtok(dirName, "/");     // break up "a/b/c/d" into tokens
    c = path[0];
    //check to see if the path is relative
    if (c != '/')
    {
        cur = cwd;
    }
    //iterate for every token in dirName
    while(s && s[0] != '.')
    {
        cur = search(cur->childPtr, s); //search for the current node name
        if (!cur)
        {
            printf("This directory does not exist\n");
            return;
        }
        else if (cur && cur->type != 'D')
        {
            printf("This is not a 'D' type\n");
            return;
        }
        s = strtok(NULL, "/");

    }
        Node *temp = search(cur->childPtr, baseName); //see if node already exists

        //if node exists, return and print
        if (temp)
        {
            printf("This directory already exists\n");
            return;
        }
        else //otherwise insert at the end of the list
        {
            if (cur->type == 'D')
            {
                insertAtEnd(cur, 'D');
            }

        }

}

//removes a directory given a path name
void rmdir (char *path)
{
    Node *cur = root;
    char c = path[0];
    char *s;
    tokenize(path);
    if (c == '/')
    {
        cur = cwd;
    }
    s = strtok(dirName, "/");

    while (s && s[0] != '.')
    {
        cur = search(cur->childPtr, s);
        if (!cur)
        {
            printf("This directory does not exist\n");
            return;
        }
        s = strtok(NULL, "/");
    }

    Node *temp = search(cur->childPtr, baseName);

    if (temp && temp->type == 'D' && temp->childPtr == NULL)
    {
        cur->childPtr = removeNthNode(cur->childPtr, temp);
    }
    else
    {
        printf("Directory either does not exist or is not of type 'D'");
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
        cur->siblingPtr = malloc(sizeof(Node));
        cur = cur->siblingPtr;
        strcpy(cur->name, baseName);
        cur->parentPtr = parent;
        cur->siblingPtr = NULL;
        cur->type = type;
        cur->childPtr = NULL;
    }
    else //there are no children in the cwd
    {
        cur->childPtr = malloc(sizeof(Node));
        cur = cur->childPtr;
        strcpy(cur->name, baseName);
        cur->parentPtr = parent;
        cur->siblingPtr = NULL;
        cur->type = type;
        cur->childPtr = NULL;
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
        while (s && s[0] != '.')
        {
            cur = search(cur->childPtr, s);
            s = strtok(NULL, "/");
        }

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

    s = strtok(dirName, "/");

    if (pathName[0] == 0) // no path
    {
        cwd = root;
        return;
    }
    else if (path[0] == "/") //if absolute path is given, start from root
    {
        cur = root;
    }

    while(s && s[0] != '.') //for every token in the string
    {
        cur = search(cur->childPtr, s);
        if (!cur)
        {
            printf("This directory does not exist\n");
            return;
        }

        s = strtok(NULL, "/");
    }
    cur = search(cur->childPtr, baseName);
    if (cur)
    {
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
    Node *temp = cur;

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
    Node *cur = root;

    //tokenize the path name
    tokenize(path);

    s = strtok(dirName, "/");     // break up "a/b/c/d" into tokens
    c = path[0];
    //check to see if the path is relative
    if (c != '/')
    {
        cur = cwd;
    }
    //iterate for every token in dirName
    while(s && s[0] != '.')
    {
        cur = search(cur->childPtr, s); //search for the current node name
        if (!cur)
        {
            printf("This directory does not exist\n");
            return;
        }
        else if (cur && cur->type != 'D')
        {
            printf("This is not a 'D' type\n");
            return;
        }
        s = strtok(NULL, "/");

    }
        Node *temp = search(cur->childPtr, baseName); //see if node already exists

        //if node exists, return and print
        if (temp)
        {
            printf("This directory already exists\n");
            return;
        }
        else //otherwise insert at the end of the list
        {
            if (cur->type == 'D')
            {
                insertAtEnd(cur, 'F');
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
    s = strtok(dirName, "/");

    while (s && s[0] != '.')
    {
        cur = search(cur->childPtr, s);
        if (!cur)
        {
            printf("This directory does not exist\n");
            return;
        }
        s = strtok(NULL, "/");
    }

    Node *temp = search(cur->childPtr, baseName);

    if (temp && temp->type == 'F' && temp->childPtr == NULL)
    {
        cur->childPtr = removeNthNode(cur->childPtr, temp);
    }
    else
    {
        printf("Directory either does not exist or is not of type 'F'");
    }
}

//saves the current tree structure to a file
void save()
{
    myFile = fopen("myFile", "w+");
    fprintf(myFile, "%c   %s\n", root->type, root->name);
    if (root->childPtr != 0)
    {
        saveHelper(myFile, root->childPtr);
    }
    fclose(myFile);
}
//saves contents of tree structure to a file
void saveHelper (FILE *filename, Node *cur)
{
    if (cur && filename)
    {
        fprintf(filename, "%c ", cur->type);
        rpwdf(cur, filename);
        fprintf(filename, "\n");
        saveHelper(filename, cur->childPtr);
        saveHelper(filename, cur->siblingPtr);
    }
}

void help()
{
}

void quit()
{
}


//calls the load helper function
void reload ()
{
    char filename[64];
    if (root->childPtr != 0)
    {
        deleteTree(root);
    }
    else
    {
        free(root);
        root->childPtr = 0;
        root->siblingPtr = 0;
    }
    printf("\n");
    printf("Enter a filename: ");
    scanf("%s", &filename);
    myFile = fopen (filename, "r");
    reloadHelper();
    fclose(myFile);
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

    if (myFile)
    {
        while (!feof(myFile))
        {
            fgets(line, 45, myFile);
            sscanf(line, "%s   %s", t, name);
            type = t[0];

            if(type == 'D')
            {
                mkdir(name);
            }
            else
            {
                creat(name);
            }

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
    if (cur->parentPtr == cur)
    {
        return;
    }
    rpwdf(cur->parentPtr, filename);
    fprintf(filename, "/%s", cur->name);
}
