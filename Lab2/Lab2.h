#ifndef LAB2_H
#define LAB2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
typedef struct Node Node;

struct Node
{
    char name[64];
    char type;
    Node *childPtr;
    Node *siblingPtr;
    Node *parentPtr;
};



int findCmd(char *command);
initRoot();
void mkdir (char *path);
void rmdir (char *path);
void cd (char *path);
void ls (char *path);
void pwd();
void rpwd(Node *cur);
void creat(char *path);
void rm (char *path);
void save ();
void saveHelper(Node *cur);
void reload ();
void quit();
void insertAtEnd(Node *start, char type);
Node *removeNthNode(Node *start, Node *n);
Node *search(Node *current, char *str);
void tokenize(char *path);
void printList(Node *cur);
void help();
void rpwdf(Node *cur, FILE *filename);
void reloadHelper();
void deleteTree(Node *pRoot);
void traceMessages(Node *cur, char *dir);
Node *searchPathName(Node *cur);
Node *makeNode (Node *parent, char type);


#endif
