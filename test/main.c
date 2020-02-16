#include <stdio.h>
int g = 56;
int *FP; // a global pointer
int main(int argc, char *argv[], char *env[])
{
	int a,b,c,f;
    printf("enter main\n");
    printf("&a = %x  &b = %x  &c = %x\n", &a, &b, &c);
    a=1; b=2; c=3;
    f = A(a,b);
    printf("exit main\n\n");
}


int A(int x, int y)
{
	int d,e,f;
    printf("enter A\n\n");
    d=4; e=5; f=6;
    B(d,e);
    printf("&x = %x  &y = %x\n", &x, &y);
    printf("&d = %x  &e = %x  &f = %x\n", &d, &e, &f);
    printf("exit A\n\n");
    return 7;
}

int B(int x, int y)
{
	int u,v,w, *fp;
	unsigned int *temp = FP;
    printf("enter B\n");
    int r, t;

    u=7; v=8; w=9;
    asm("movl %ebp, FP"); // set FP=CPU’s %ebp register

    printf("&x = %x  &y = %x  &u = %x  &v = %x  &w = %x\n", &x, &y, &u, &v, &w);

    printf("------------ Problem 3.1 ------------ \n");
    fp = FP;
    printf("FP = %x\n", FP);

    // Write C code to DO (1)-(3) AS SPECIFIED BELOW
    while (fp != 0)
    {
  		printf("%x --> ", fp);
  		temp = fp;
    	fp = (int *)*fp;
    }

    printf("NULL\n\n");
    printf("------------ Problem 3.2 ------------ \n");
    asm("movl %ebp, FP"); //set FP to ebp address
    fp = FP;

    printf("*temp = %x\n", *temp);
    while (fp != temp)
    {

        printf("%x      %x\n", (unsigned int)fp, (unsigned int)*fp);
        fp++;
    }



    printf("exit B\n\n");
}
