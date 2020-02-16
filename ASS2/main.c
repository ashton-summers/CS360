#include "type.h"
NODE *readyQueue = 0;
NODE *p;

int main(int argc, char *argv[], char *env[])
{
    int i = 0, priority;
    char num[5] = "Node";
    char buf[5];
    srand(time(NULL));
    myprintf("%d %d %d\n", 400, 222, 333);
    printf("%d\n", -10000000000);
    //putchar('0');

    myprintf("argc: %d\n", argc);
    myprintf("Enter any key to print argv args: ");
    getchar();
    while(*argv)
    {
        myprintf("%s\n", *argv);
        argv++;
    }
    myprintf("\n");
    myprintf("Enter any key to print env args: ");
    getchar();
    while(*env)
    {
        myprintf("%s\n", *env);
        env++;
    }


    while(1)
    {
        p = (NODE *)malloc(sizeof(NODE));
        sprintf(buf, "%d", i); // put int i into a buffer
        strcat(num, buf); // append i to num char array
        strcpy(p->name, num); //copy name to NODE p
        myprintf("Press any key to enter node into ready queue: \n");
        p->priority = rand() % 10;
        getchar();
        enqueue(&readyQueue, p);
        i++;
        printQueue(readyQueue);
        memset(num, 0, 5);
        strcpy(num, "Node");
    }



    return 0;
}




