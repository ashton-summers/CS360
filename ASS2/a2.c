#include "type.h"
int base;
char *table = "0123456789ABCDEF";

//rescursive function to print an unsigned integer
//base gets updated depending on format we are printing
int rpu(u32 x)
{
    char c;
    if (x)
    {
        c = table[x % base];
        rpu(x / base);
        putchar(c);
    }
}



//calls the recursive printu function to print unsigned int
int printu(u32 x)
{
    base = 10;
    if (x < 0){x *= -1;}
    else if (x == 0){ putchar('0'); }
    else
        rpu(x);
    putchar(' ');

}

//calls recursive printu function with updated base
int printo(u32 x)
{
    base = 8;
    putchar('0');
    if (x == 0){ putchar('0'); }
    else
        rpu(x);
    putchar(' ');
}

//prints an unsigned integer in hex
int printx(u32 x)
{
    base = 16;
    putchar('0');
    putchar('x');
    if (x == 0){ putchar ('0'); }
    else
        rpu(x);
    putchar(' ');
}

//calls recursive printu function with updated base to
//print a signed int
int printd(int x)
{
    base = 10;
    if (x < 0)
    {
        x *= -1;
        putchar('-');
    }
    if (x == 0){ putchar('0'); }
    else
        rpu(x);
    putchar(' ');
}

//prints a string to the screen
int prints(char *str)
{
    while (*str)
    {
        putchar(*str);
        str++;
    }
}

int myprintf(char *fmt, ...)
{
    char *p = fmt;
    u32 *ip = (u32 *) &fmt + 1;

    while(*p)
    {
        if (*p == '%')
        {
            p++;
            switch(*p)
            {
                case 'c': putchar(*ip);
                    ip++;
                    break;
                case 'd': printd(*ip);
                    ip++;
                    break;
                case 'u': printu(*ip);
                    ip++;
                    break;
                case 'x': printx(*ip);
                    ip++;
                    break;
                case 'o': printo(*ip);
                    ip++;
                    break;
                case 's': prints(*ip);
                    ip++;
                    break;
            }
        }
        else
        {
            if (*p == '\n')
            {
                putchar('\n');
                putchar('\r');
            }
            else
                putchar(*p);
        }

        p++;
    }

}

//enters NODE p into the q by priority
void enqueue (NODE **q, NODE *p)
{
    NODE *prev = NULL, *cur = NULL;
    //set the cur to head of queue
    cur = *q;

    //if queue is empty
    if (*q == NULL)
    {
        *q = p;
    }
    else
    {
        //find the right spot to insert
        while(cur != NULL && p->priority <= cur->priority)
        {
            prev = cur;
            cur = cur->next;
        }

        if (prev != NULL) //in the middle of the list somewhere
        {
            p->next = cur;
            prev->next = p;
        }
        else // inserting at front
        {
            p->next = *q;
            *q = p;
        }
    }

}

//prints the contents of the queue
void printQueue(NODE *q)
{
    while (q != NULL)
    {
        myprintf("[%s: p = %d]-->", q->name, q->priority);
        q = q->next;
    }
    myprintf("NULL");

    myprintf("\n\n");
}

//deletes first node in linked list and returns node with highest priority
NODE *dequeue(NODE **q)
{
    NODE *temp = *q;
    if (*q == NULL){ return NULL; }
    else
    {
        free(*q);
        *q =  temp->next;
    }

    return *q;
}


















