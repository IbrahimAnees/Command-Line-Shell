// NAME: Ibrahim Anees
// UPI: iane056
// 
// Command line C shell

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

enum status1
{
    SLEEPING,
    RUNNABLE,
    STOPPED,
    IDLE,
    ZOMBIE,
    DONE
};

struct job
{
    pid_t jobID;
    char jobCommand[100];
    enum status1 jobStatus;
    int jobNumber;
};

struct command
{
    const char **argv;
};

const int MAX_INPUT_LENGTH = 1000;

char command[100];
static char *args[512];
char homeDirectory[512];
bool terminalControl = true;
char *history[100];
static int arrayPos = 0;
int status = 0;
char *pipeCommands[10];
struct command cmd[10];
int waitBG = 1;
struct job jobs[100];
int highestJob = 0;
char pipeCommandPrint[100];
char commandPrint2[100];
int fgID = 0;

// int pipeAndBG = 0;

int searchHighestJob(struct job *jobs)
{
    int currentLargest = jobs[0].jobNumber;
    for (int i = 1; i < highestJob; i++)
    {
        if (jobs[i].jobNumber > currentLargest)
        {
            currentLargest = jobs[i].jobNumber;
        }
    }
    return currentLargest;
}

// REFERENCE: Functions pipeHelp and creatPipeFork adapted from:
// https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
int pipeHelp(int i1, int i2, struct command *cmd)
{
    pid_t pid;
    if ((pid = fork()) == 0)
    {
        if (i1 != 0)
        {
            dup2(i1, 0);
            close(i1);
        }
        if (i2 != 1)
        {
            dup2(i2, 1);
            close(i2);
        }
        return execvp(cmd->argv[0], (char *const *)cmd->argv);
    }
    return pid;
}

int creatPipeFork(int n, struct command *cmd, int pipeAndBG)
{
    int i;
    int i1, fd[2];
    i1 = 0;
    for (i = 0; i < n - 1; ++i)
    {
        pipe(fd);
        pipeHelp(i1, fd[1], cmd + i);
        close(fd[1]);
        i1 = fd[0];
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        if (i1 != 0)
            dup2(i1, 0);
        return execvp(cmd[i].argv[0], (char *const *)cmd[i].argv);
    }
    else
    {
        if (pipeAndBG == 0)
        {
            // while ((wait(&status)) > 0)
            //     ;
            waitpid(pid, NULL, 0);
        }
        else
        {
            // jobs[highestJob].jobNumber = highestJob + 1;
            // strcat(command, " &");
            // strcpy(jobs[highestJob].jobCommand, pipeCommandPrint);
            // jobs[highestJob].jobID = pid;
            // jobs[highestJob].jobStatus = RUNNABLE;

            // printf("[%d]\t%d\n", jobs[highestJob].jobNumber, jobs[highestJob].jobID);
            // highestJob++;

            jobs[highestJob].jobNumber = 0;
            strcat(command, " &");
            strcpy(jobs[highestJob].jobCommand, pipeCommandPrint);
            jobs[highestJob].jobID = pid;
            jobs[highestJob].jobStatus = RUNNABLE;
            int highestAvailableJN = searchHighestJob(jobs);
            jobs[highestJob].jobNumber = highestAvailableJN + 1;
            printf("[%d]\t%d\n", highestAvailableJN + 1, jobs[highestJob].jobID);
            highestJob++;
        }
    }
}

void catch_stop(int signum)
{
    if (signum == SIGTSTP)
    {
        printf(" Foreground process stopped \n");
        //exit(EXIT_SUCCESS);
        kill(fgID, SIGSTOP);
   }
}

char **buildArgs(char *str)
{
    int i = 1;
    char **temp = malloc(20 * sizeof(char *));
    temp[0] = strtok(str, " ");
    while ((temp[i] = strtok(NULL, " ")) != NULL)
        i++;
    temp[i] = NULL;
    return temp;
}

int pipeSplit(char *input)
{
    int pipeNumber = 1;
    pipeCommands[0] = strtok(input, "|");
    while ((pipeCommands[pipeNumber] = strtok(NULL, "|")) != NULL)
        pipeNumber++;
    pipeCommands[pipeNumber] = NULL;
    return pipeNumber - 1;
}

void spaceSplit(char *input)
{
    int i = 1;
    args[0] = strtok(input, " ");
    while ((args[i] = strtok(NULL, " ")) != NULL)
        i++;
    args[i] = NULL;
}

void printGreetingMessage()
{
    printf("---------------------------------------------- \n");
    printf("Welcome to the Ash shell! Type 'exit' to exit. \n");
    printf("---------------------------------------------- \n");
}

void getHomeDirectory()
{
    if (getcwd(homeDirectory, sizeof(homeDirectory)) != NULL)
    {
    }
    else
    {
        perror("Error getting directory");
    }
}

int runCommand(bool waitYes, bool and1)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        int status = execvp(args[0], args);
        if (status == -1)
        {
            printf("Unknown command! ...\n");
            return -1;
        }
    }
    else
    {
        if (and1)
        {
            // highestJob++;
            // jobs[highestJob-1] = pid;
            // printf("[%d]\t%d\n", highestJob, jobs[highestJob-1]);
            jobs[highestJob].jobNumber = 0;
            strcat(command, " &");
            strcpy(jobs[highestJob].jobCommand, commandPrint2);
            jobs[highestJob].jobID = pid;
            jobs[highestJob].jobStatus = RUNNABLE;
            int highestAvailableJN = searchHighestJob(jobs);
            jobs[highestJob].jobNumber = highestAvailableJN + 1;
            printf("[%d]\t%d\n", highestAvailableJN + 1, jobs[highestJob].jobID);
            highestJob++;
        }
        if (waitYes)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
        }
    }
    return pid;
}

int main()
{
    if (!isatty(STDIN_FILENO))
    {
        terminalControl = false;
    }

    if (terminalControl)
    {
        printf("\e[1;1H\e[2J");
        printGreetingMessage();
    }

    getHomeDirectory();

    signal(SIGTSTP, catch_stop);

    while (1)
    {
        
        for (int k = 0; k < highestJob; k++)
        {
            if (waitpid(jobs[k].jobID, &status, WNOHANG) != 0)
            {
                if (jobs[k].jobStatus != DONE)
                {
                    printf("[%d] <Done> %s\n", jobs[k].jobNumber, jobs[k].jobCommand);
                    jobs[k].jobStatus = DONE;
                    jobs[k].jobNumber = 0;
                }
            }
        }

        // printf("%d\n", highestAvailableJN);

        // for (int j = highestJob-1; j = 0; j--) {
        //     if (jobs[j] != 0) {
        //         highestJob = j+1;
        //         //break;
        //     }
        //     else {
        //         //highestJob = 0;
        //         printf("[%d] <Done> %s", j+1, command);
        //     }
        // }

        bool waitYes = true;
        if (terminalControl)
        {
            printf("ash> ");
            fflush(NULL);
        }

        if ((fgets(command, MAX_INPUT_LENGTH, stdin) == 0))
        {
            printf("ash> \n");
            break;
        }



        command[strcspn(command, "\r\n")] = 0;

        if (!terminalControl)
        {
            printf("ash> %s\n", command);
        }

        history[arrayPos] = (char *)malloc(strlen(command));

        if ((strncasecmp(command, "h ", 2) == 0))
        {
            bool numberOrNot = true;
            char *historyNumber = command + 2;
            for (int i = 2; i < strlen(command); i++)
            {
                if (!isdigit(command[i]))
                {
                    numberOrNot = false;
                }
            }
            int historyInt = atoi(historyNumber) - 1;
            if (numberOrNot && (historyInt < (arrayPos+1)))
            {
                char *historyCommand = history[historyInt];
                strncpy(command, historyCommand, 100);
            }
            else
            {
                // printf("Please enter a valid number...");
            }
        }
        else if ((strncasecmp(command, "history ", 8) == 0))
        {
            bool numberOrNot = true;
            char *historyNumber = command + 8;
            for (int i = 8; i < strlen(command); i++)
            {
                if (!isdigit(command[i]))
                {
                    numberOrNot = false;
                }
            }

            int historyInt = atoi(historyNumber) - 1;
            if (numberOrNot && (historyInt < (arrayPos+1)))
            {
                char *historyCommand = history[historyInt];
                strncpy(command, historyCommand, 100);
            }
            else
            {
            }
        }

        strcpy(history[arrayPos], command);
        arrayPos++;
        // if (!terminalControl)
        // {
        //     printf("ash> %s\n", command);
        // }

        int exitStatus = strcasecmp(command, "exit");
        if (exitStatus == 0)
        {
            printf("---------------\nExiting Ash...\n---------------\n");
            // break;
            _Exit(0);
        }

        else if (strcmp(command, "jobs") == 0)
        {

            FILE *fp;
            char buff[255];
            char fileLine[100];
            char State[100];
            char Name[100];
            char currentState[100];
            for (int i = 0; i < highestJob; i++)
            {
                if (jobs[i].jobStatus != DONE)
                {
                    sprintf(fileLine, "/proc/%d/status", jobs[i].jobID);
                    fp = fopen(fileLine, "r");
                    if (fp == NULL)
                    {
                    }
                    else
                    {

                        fgets(buff, 256, fp);
                        fgets(buff, 256, fp);
                        fgets(buff, 256, fp);
                        sscanf(buff, "State:\t%s", State);
                        fclose(fp);
                    }
                    if (strcmp(State, "S") == 0) {
                        strcpy(currentState, "Sleeping");
                    }
                    else if (strcmp(State, "R") == 0) {
                        strcpy(currentState, "Running");
                    }              
                                   else if (strcmp(State, "Z") == 0) {
                        strcpy(currentState, "Zombie");
                    }       
                                   else if (strcmp(State, "X") == 0) {
                        strcpy(currentState, "Dead");
                    }    
                                   else if (strcmp(State, "x") == 0) {
                        strcpy(currentState, "Dead");
                    }    
                                   else if (strcmp(State, "D") == 0) {
                        strcpy(currentState, "Waiting");
                    }    
                                   else {
                        strcpy(currentState, "Unknown");
                    }    
                    printf("[%d] <%s> %s\n", jobs[i].jobNumber, currentState, jobs[i].jobCommand);
                        // printf("%s ",Name);
                        // printf("%s\n",State);
                }
            }
        }

        else if (strcasecmp(command, "cd") == 0)
        {
            if (chdir(homeDirectory) == 0)
            {
            }
        }

        else if (strncasecmp(command, "cd ", 3) == 0)
        {
            char *newCommand = command + 3;
            if (chdir(newCommand) == 0)
            {
            }
            else
            {
                printf("cd: %s: No such file or directory\n", newCommand);
            }
        }

        else if ((strcasecmp(command, "history") == 0) || (strcasecmp(command, "h") == 0))
        {
            if (arrayPos < 10)
            {
                for (int i = 0; i < arrayPos; i++)
                {
                    printf("%d: %s\n", i + 1, history[i]);
                }
            }
            if (arrayPos >= 10)
            {
                for (int i = arrayPos - 10; i < arrayPos; i++)
                {
                    printf("%d: %s\n", i + 1, history[i]);
                }
            }
        }

        else if (strstr(command, " | ") != NULL)
        {
            int pipeAndBG;
            if ((command[strlen(command) - 1] == '&') && (command[strlen(command) - 2] == ' '))
            {
                strcpy(pipeCommandPrint, command);
                pipeAndBG = 1;
                command[strlen(command) - 2] = '\0';
            }
            else
            {
                pipeAndBG = 0;
            }
            int number_of_pipes = pipeSplit(command);
            struct command cmd2[10];
            for (int i = 0; i <= number_of_pipes; i++)
            {
                cmd2[i].argv = (const char **)(buildArgs(pipeCommands[i]));
            }
            creatPipeFork(number_of_pipes + 1, cmd2, pipeAndBG);
        }

        else if ((command[strlen(command) - 1] == '&') && (command[strlen(command) - 2] == ' '))
        {
            strcpy(commandPrint2, command);
            command[strlen(command) - 2] = '\0';
            spaceSplit(command);
            runCommand(false, true);
        }

        else if (strcmp(command, "fg") == 0) {
            if (fgID != 0) {
            kill(fgID, SIGCONT);
            }
        }

        else if (strcmp(command, "kill") == 0) {
            if (fgID != 0) {
            kill(fgID, SIGKILL);
            printf("Process killed...\n");
            }
        }

        else
        {
            spaceSplit(command);
            fgID = runCommand(true, false);
        }

        

    }
    return 0;
}