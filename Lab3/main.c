#include "Lab3.h"


int main(int argc, char *argv[], char *env[])
{
	int i = 0, status, pid;
	char temp[100];
	char *str = "hello";
	strcpy(path, "/usr/bin:/usr:/usr/local/sbin:/usr/local/bin:/usr/sbin:/sbin:/bin:/usr/games:/usr/local/games:.");
	setAllPaths();
	printf("******* Ashton's SH Simulator *******\n");

	while (1)
	{
		printf("ash %: ");
		fgets(temp, 100, stdin); //get line from stdin
		strcpy(buf, temp);
		strcpy(originalBuf, buf);
		if (strcmp(buf, "exit\n") == 0) { exit(1); }
		if (strcmp(buf, "\n") != 0)
		{
            pid = fork();
            printf("ash forks a child process\n");
            if (pid)
            {
                pid = wait(&status);
            }
            else
            {
                processLine(env);
            }
        }

		resetBuffers();
	}

	return 0;
}



//parses the args entered, puts them in char *myargs
void getArgs(char *buffer, char *args[])
{
	int i = 1;
	char *temp = strtok(buffer, " \n");
	args[0] = malloc(100);
	strcpy(args[0], temp);

	while (temp = strtok(NULL, " \n"))
	{
		if (strcmp(temp, "<") == 0) //if there is an IO symbol, open
		{
			inputFile = strtok(NULL, " \n"); //return the string after IO token
			ioToken = "<";
			IOStr = inputFile;
		}
		else if (strcmp(temp, ">") == 0)
		{
			outputFile = strtok(NULL, " \n");
			ioToken = ">";
			IOStr = outputFile;
		}
		else if (strcmp(temp, ">>") == 0)
		{
			outputFile = strtok(NULL, " \n");
			ioToken = ">";
			IOStr = outputFile;
			append = 1;
		}
		else
		{
			args[i] = (char *)malloc(strlen(temp) + 1);
			strcpy(args[i], temp);
			i++;
		}
	}

	args[i] = (char *)NULL;
}

//resets command line buffer variables
void resetBuffers()
{
	memset(buf, 0, 100);
	memset(cmd, 0, 100);
	memset(myargv, 0, 100);
	memset(headArgs, 0, 100);
	memset(tailArgs, 0, 100);
	ioToken = "";
}

//sets array of pathnames to search for cmd to execute
void setAllPaths()
{
	char *temp = strtok(path, ":");
	int i = 0;
	while (temp = strtok(NULL, ":"))
	{
		allPaths[i] = (char *)malloc(strlen(temp) + 1);
		strcpy(allPaths[i], temp);
		i++;
	}

	allPaths[i] = NULL;
}

//process command entered by the user
void doCmd(char *args[], char *env[])
{
	char tempPath[100];
	int i = 0, j = -1, ret = -1, index = -1;
	char *token;

	if (strcmp(args[0], "cd") == 0) //if command is trivial
	{
		if (myargv[1])
		{
			ret = chdir(args[1]);
			if (chdir == 0)
				printf("cd successful!\n");
			else
				printf("cd failed!\n");
		}
		else //no path, so cd to home
		{
			ret = chdir(homeDir);
			if (chdir == 0)
				printf("cd to home successful!\n");
			else
				printf("cd to home unsuccessful!\n");
		}
	}
	else // not a trivial command
	{

        while (j == -1 && i < 9) //try the command on all paths. 9 paths total hence i < 9
        {
            strcpy(tempPath, allPaths[i]); //copy path to temp
            strcat(tempPath, "/"); //concatenate "/" and the cmd to temp
            strcat(tempPath, args[0]);
            //printf("try %s on path %s\n", args[0], tempPath);
            //printf("arg = %s\n", args[0]);
            j = execvp(args[0], args); //attempt to execute the path
            i++;
        }
		printf("No such command\n");
	}

}

//closes stdout file descriptor and writes all input to *file instead
void openForWrite(char *file)
{
    printf("opening file '%s' for writing\n", file);
	close(1);
	open(file, O_WRONLY | O_CREAT, 0644);
}

//closes stdout file descriptor and writes all data from to screen file instead
void openForRead(char *file)
{
    printf("opening file '%s' for reading\n", file);
	close(0);    // system call to close file descriptor 0
	open(file, O_RDONLY);
}

//opens a file for appending
void openForAppend(char *file)
{
    close (1);
    open(file, O_APPEND | O_WRONLY | O_CREAT, 0644);
}


int isPipe()
{
	int ret = strchr(buf, '|');

	if (ret)
	{
		return 1;
	}
	return 0;
}

//gets the proper arguments from the line inputted by user
//parses all arguments to be passed to exec() function
void processLine(char *env[])
{
    char *head, tail[100], tempPath[100], tempHead[100], tempTail[100];
    int pd[2], pid, status, j, i;
    if (isPipe(buf))
    {
        pipe(pd);
        head = strtok(buf, "|");
        strcpy(tempHead, head);
        strcpy(tail,strtok(NULL, "\n"));
        strcpy(tempTail, tail);
        getArgs(head, headArgs);
        getArgs(tail, tailArgs);

        pid = fork();
        if (pid)
        {
            printf("Parent %d pipe writer waits for child %d to die\n", getpid(), pid);
            close(pd[0]); //writer closes pd[0]
            close(1); //close write (stdout)
            dup(pd[1]); //replace write with pd[1]
            if (containsInputRedirect(tempHead))
            {
                openForRead(inputFile);
            }
            else if (containsOutputRedirect(tempHead))
            {
                openForWrite(outputFile);
            }
            else if (append == 1)
            {
                openForAppend(outputFile);
            }
            //pid = wait(&status);
            doCmd(headArgs, env);
        }
        else
        {
            printf("Child %d pipe reader executing...\n", getpid());
            close(pd[1]); //close write
            close(0); //close read (stdin)
            dup(pd[0]); //replace read with pd[0]
            if (containsInputRedirect(tempHead))
            {
                openForRead(inputFile);
            }
            else if (containsOutputRedirect(tempHead))
            {
                openForWrite(outputFile);
            }
            else if (append == 1)
            {
                openForAppend(outputFile);
            }
            doCmd(tailArgs, env);
        }
       // exit(EXIT_FAILURE);
    }
    else
    {

        getArgs(buf, myargv);
        pid = fork();
        if (pid)
        {
            printf("PARENT %d WAITS FOR CHILD %d TO DIE\n", getpid(), pid);
            pid = wait(&status);
            printf("DEADCHILD = %d, HOW = %04x\n", getpid(), status);
        }
        else
        {
            if (containsInputRedirect(originalBuf))
            {
                openForRead(inputFile);
            }
            else if (containsOutputRedirect(originalBuf))
            {
                openForWrite(outputFile);
            }
            else if (append == 1)
            {
                openForAppend(outputFile);
            }
            printf("\n");
            doCmd(myargv, env);
        }
    }
}


int containsOutputRedirect(char *str)
{
    int ret = strchr(str, '>');

	if (ret)
	{
		return 1;
	}
	return 0;
}

int containsInputRedirect(char *str)
{
    int ret = strchr(str, '<');

	if (ret)
	{
		return 1;
	}
	return 0;
}
