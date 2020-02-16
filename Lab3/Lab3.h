#ifndef LAB3_H
#define LAB3_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

//GLOBALS
char *myargv[100], *headArgs[100], *tailArgs[100];
char cmd[100];
char homeDir[100];
char buf[100];
char originalBuf[100]; //used just in case there is IO redirection
char path[200];
char *allPaths[100];
char *IOStr;
char *ioToken;
char *outputFile;
char *inputFile;
int append;
//END GLOBALS


void setAllPaths();
void doCmd(char *args[], char *env[]);
void getArgs(char *buffer, char *args[]);
void resetBuffers();
void openForWrite(char *file);
void openForRead(char *file);
void openForAppend(char *file);
int isPipe();
void handlePipe();
void processLine(char *env[]);
int containsOutputRedirect(char *str);
int containsInputRedirect(char *str);

#endif
