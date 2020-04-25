#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>

static unsigned history_count = 0;
static unsigned  jobCount=0;
typedef struct job
{
    char *command;              /* command line, used for messages */
    pid_t pid;
    int  mode;
} job;
typedef struct history
{
    char *command;              /* command line, used for messages */
    pid_t pid;
    int  mode;
} history;

//struct job[100][1000]jobArray;
struct job jobArray[100][1000];
struct history historyArray[100][100];

void addJob(char *command,pid_t pid,int mode){
    (jobArray[jobCount])->command=malloc(strlen(command)+1);
    strcpy((jobArray[jobCount])->command, command);
    jobArray[jobCount]->pid=pid;
    jobArray[jobCount]->mode=mode;
    jobCount++;
}

/**
 * adds command to the history
 * @param command
 */
void add_command(const char* command,pid_t pid,int mode){
    (historyArray[history_count])->command=malloc(strlen(command)+1);
    //(historyArray[history_count])->mode=malloc(strlen(mode)+1);
    strcpy(historyArray[history_count]->command,command);
    //strcpy(historyArray[history_count]->mode,mode);
    historyArray[history_count]->pid=pid;
    historyArray[history_count]->mode=1;
    history_count++;
}
char** splitBySpace(char* line,char** args){
    char *token;
    token= strtok(line," ");
    int i=0;
    while(token!=NULL){
        args[i]=token;
        token=strtok(NULL," ");
        i++;
    }
    args[i]=NULL;

    return args;
}
/**
 * concats two strings
 * @param s1
 * @param s2
 * @return
 */
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // remember to check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


/**
 * returns whethre a string contains a certain char
 * @param command
 * @param c
 * @return
 */
int existsChar(char** command,char* s){
    int len = sizeof(command)/sizeof(command[0]);
    int i;

    for(i = 0; i < len; ++i)
    {
        if(strcmp(command[i], s)==0)
        {
            return 1;
        }
    }
    return 0;
}
int existsCharInLine(char* line,char c){
    int len = strlen(line);
    int i;

    for(i = 0; i < len; ++i)
    {
        if(line[i]==c)
        {
            return 1;
        }
    }
    return 0;
}
int findIndexOfChar(char* line,char c){
    int len=strlen(line);
    for(int i=0;i<len;i++){
        if(line[i]==c){
            return i;
        }
    }
    return -1;
}



int main() {
    //the user command
    char *args[100];
    char *currDir;
    char *toDir;
    char buffer[1000];
    //without &
    char *commandWithout[100];
    //we read the info to here
    char line[1024];
    char backupLine[1024];
    int pid;
    int background = 0;
    char *path = "/bin/";
    int numOfCommands = 0;
    size_t length;
    //jobArray
    //max is 100 commands and each command at most 100 chars

    int stat;
    //run the shell until we read the command exit
    while (1) {
        if (numOfCommands > 100) {
            break;
        }
        printf("> ");
        if (!fgets(line, 1024, stdin)) {
            break;
        }
        length = strlen(line);
        if (line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        strcpy(backupLine,line);

        //if it doesnt exists the do not run on background
        if (existsCharInLine(line, '&') == 0) {
            strcpy((char *) args, (const char *) splitBySpace(line, args));
            strcpy((char *) commandWithout, line);
            background = 0;
        } else {
            int index = findIndexOfChar(line, '&');
            line[index] = NULL;
            strcpy((char *) commandWithout, line);
            strcpy((char *) args, (const char *) splitBySpace(line, args));
            //strcpy((char *) commandWithout, (const char *) splitBySpace(line, args));
            background = 1;
        }


        if (strcmp("exit", args[0]) == 0) {
            printf("%d",getpid());
            exit(0);
        }

        if (strcmp("history", args[0]) == 0) {
            for (int i = 0; i < history_count; i++) {
                if(waitpid(historyArray[i]->pid, &stat, WNOHANG)==0){
                    historyArray[i]->mode=1;//running
                }
                else{
                    historyArray[i]->mode=0;//done
                }
            }
        }

        if (strcmp("jobs", args[0]) == 0) {
            // printf("%d\n",history_count);
            //print the history
            for (int i = 0; i < jobCount; i++) {
                //for(int j=0;j<strlen(history[i]);j++){
               //printf("%d\n",jobArray[i]->pid);
                if(waitpid(jobArray[i]->pid, &stat, WNOHANG)==0){
                   // printf("%d %s\n",jobArray[i]->pid,jobArray[i]->command);
                    jobArray[i]->mode=1;
                }
                else{
                    //printf("%d %s\n",jobArray[i]->pid,"im dead\n");
                    jobArray[i]->mode=0;
                }
            }
        }
        //finish cd
        if (strcmp("cd", args[0]) == 0) {
            add_command(commandWithout,getpid(),0);
            char cwd[1000];
            /*if(strlen(*args)>2){
                fprintf(stderr,"Error: Too many arguments");
            }*/
            if (strcmp(args[1], "..") == 0) {
                chdir("..");
                continue;
            }
            /*currDir = getcwd(buffer, sizeof(buffer));
            currDir = strcat(currDir, "/");
            printf("%s\n",args[1]);
            toDir = strcat(toDir, args[1]);
            chdir(toDir);*/
            getcwd(cwd,sizeof(cwd));
            strcat(cwd,"/");
            strcat(cwd,args[1]);
            chdir(cwd);
            continue;
        }
        numOfCommands++;
        pid = fork();
        add_command(commandWithout,pid,1);
        //if the command is cd we will implement it by our selves

        if (pid < 0) {
            //check which error to print
            fprintf(stderr, "error in child");
        }
        else {
            //child
            //printf("kkkk3 ");
            if (pid == 0) {
                //printf("kkkk2");
                 if (strcmp("jobs", args[0]) == 0) {
                     for (int i = 0; i < jobCount; i++) {
                         //for(int j=0;j<strlen(history[i]);j++){
                         // printf("%s %d","inside fork\n",jobArray[i]->mode);
                         if(jobArray[i]->mode==1){
                             printf("%d %s\n",jobArray[i]->pid,jobArray[i]->command);
                         }
                     }
                     exit(0);
                 }
                 if (strcmp("history", args[0]) == 0) {
                    for (int i = 0; i < history_count; i++) {
                        //for(int j=0;j<strlen(history[i]);j++){
                        // printf("%s %d","inside fork\n",jobArray[i]->mode);
                        if(historyArray[i]->mode==1){
                            printf("%d %s %s\n",historyArray[i]->pid,historyArray[i]->command,"RUNNING");
                        }
                        else{
                            printf("%d %s %s\n",historyArray[i]->pid,historyArray[i]->command,"DONE");
                        }
                    }
                    exit(0);
                }
                execv(concat(path, args[0]), args);
                fprintf(stderr, "\nError in system call\n");
            }
                //parent
            else {
                //check where to print pid
                if(strcmp("jobs",args[0])!=0&& strcmp("history",args[0])!=0){
                    printf("%d\n", pid);
                }
                //if theress no & then wait until the child is done
                if (background == 0) {
                    //wait(&stat);
                    if (waitpid(pid, NULL, 0) != pid){
                        printf("fork error");
                    }

                }
                else {
                    addJob(backupLine,pid,1);
                }
            }

        }
    }
    return 0;
}