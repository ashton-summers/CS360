#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef unsigned int u32;

typedef struct node{
  struct node *next;
  char name[64];
  int priority;
}NODE;


int rpu(u32 x);
int printu(u32 x);
int printo(u32 x);
int printx(u32 x);
int printd(int x);
int prints(char *str);
int myprintf(char *fmt, ...);
void enqueue (NODE **q, NODE *p);
void printQueue(NODE *q);
NODE *dequeue(NODE **q);
NODE *searchHighest(NODE *q);

