#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_LINE 80

void copyfromto(char *array1[], char *array2[]){
    int x=0;
    for (int i = 0; array1[i] != NULL; ++i) {
        array2[i]=strdup(array1[i]);
        x=i;
    }
//    printf("%s", array2[x]);
    array2[x+1]=NULL;
}
int main() {

    char *argv[MAX_LINE/2 +1];
   int should_run = 1;
    int hashistory = 0;
    char *history[MAX_LINE / 2 + 1];

    while (should_run){



//        printf("%s",history[0]);
        printf("mysh:~$ " );
        fflush(stdout);
        char input[MAX_LINE/2 +1]; //store input here
        int argnum=0;

        fgets( input , MAX_LINE/2+1 , stdin );
        char *token = strtok(input, " \n");
        while (token != NULL){
            argv[argnum] = token;
            token = strtok(NULL, " \n");
            argnum++;
        }
        argv[argnum]=NULL;

        if(argv[0]==NULL){
            //do nothing, reprompt
        }

        else if ( strcmp ( argv[0],"!!") == 0 ){
            if (hashistory == 0){
                printf("No commands in history.\n");
            }
            else {
                for (int i = 0; history[i] != NULL ; ++i) {
//                    printf("%i", i);
                    printf("%s ", history[i]);
                }

                printf("\n");
                //list options


                if(strcmp(history[argnum-1], "&")==0){
                    char *temmpargv[MAX_LINE/2 + 1];
                    for (int i = 0; i < argnum ; ++i) {
                        temmpargv[i]=strdup(history[i]);
                    }
                    temmpargv[argnum]=NULL;

                    pid_t pid2= fork();
                    if (pid2 == 0)
                    {
                        FILE *temp= fopen("/dev/null", "w+");
                        dup2( fileno(temp) , STDOUT_FILENO);
                        dup2( fileno(temp), STDERR_FILENO);
                        fclose(temp); //do i really need this??
                        execvp(temmpargv[0],temmpargv);
                    }

                    else if( pid2 > 0){
                        // do not wait for child.

                    }
                }


                else if (strcmp ( history[0],"cd")==0){
                    if (history[1]==NULL){
                        chdir(getenv("HOME"));
                    }
                    else if(chdir(history[1]) < 0){
                        printf("cd: %s: No such file or directory\n", history[1]);
                    }
                }

                else if ( (strcmp("|", history[1]) == 0 || strcmp( history[2], "|") == 0 ) ){


                    int fd[2];
                    pipe(fd);
                    if (fork() == 0) {
//                    dup2(fd[1],1);
                        close(STDOUT_FILENO);
                        dup(fd[1]);
                        close(fd[0]);
                        close(fd[1]);
                        char *part1[] = {history[0], NULL}; //only works for single command pipes
                        execvp(part1[0], part1);
                    }

                    if(fork() == 0){
                        close(STDIN_FILENO);
                        dup(fd[0]);
                        close(fd[1]);
                        close(fd[0]);

                        char *part2[] = {history [2] , NULL};
                        execvp(part2[0], part2);

                    }
                    close(fd[0]);
                    close(fd[1]);
                    wait(0);
                    wait(0);
                }

                else if( strcmp( "<" , history[1]) == 0){
//            FILE *tempf2 =  fopen(argv[2] , "r");
//
                    char *temparray[1]={history[0]};

//            fclose(tempf2);

                    pid_t inputredir = fork();
                    if (inputredir == 0 ) {
                        dup2(fileno(fopen(history[2], "r")), STDIN_FILENO);
                        execvp(temparray[0], temparray);
//                fclose(argv[2]);
                        exit(0);
                    }
                    else
                        wait(NULL);
                }

                else if(  strcmp( ">" , history[1] ) == 0){
//           FILE *tempf =  fopen(argv[2] , "w");
                    char *temparray[1]={history[0]};
                    pid_t outputredir = fork();
                    if ( outputredir == 0 ){
                        dup2( fileno(fopen(history[2] , "w")) , STDOUT_FILENO );
                        execvp(temparray[0], temparray);
//                fclose(argv[2]); //dont even need that
                        exit(0);
                    }
                    else {
                        wait(NULL);

                    }

                }


//
                else {
                    dup2( STDOUT_FILENO , STDERR_FILENO );

                    pid_t pid3 = fork();

                    if (pid3 == 0) {

                        if(execvp(history[0], history)<0){
                            printf("%s: command not found \n" , history[0]);
                        }

                    }

                    else if(pid3 < 0){
                        // fork errors here
                    }
                    else {
                        wait(NULL);
                    }
                }


            }
        }
        //output redirect
        else if( argnum > 1 && strcmp( ">" , argv[1] ) == 0){
//           FILE *tempf =  fopen(argv[2] , "w");
            hashistory = 1;
            copyfromto(argv , history);
            char *temparray[1]={argv[0]};
            pid_t outputredir = fork();
            if ( outputredir == 0 ){
                dup2( fileno(fopen(argv[2] , "w")) , STDOUT_FILENO );
                execvp(temparray[0], temparray);
//                fclose(argv[2]); //dont even need that
                exit(0);
            }
            else {
                wait(NULL);

            }

        }


        //input redir
        else if( argnum > 1 && strcmp( "<" , argv[1]) == 0){
//            FILE *tempf2 =  fopen(argv[2] , "r");
//
            hashistory = 1;
            copyfromto(argv , history);

            char *temparray[1]={argv[0]};

//            fclose(tempf2);

            pid_t inputredir = fork();
            if (inputredir == 0 ) {
                dup2(fileno(fopen(argv[2], "r")), STDIN_FILENO);
                execvp(temparray[0], temparray);
//                fclose(argv[2]);
                exit(0);
            }
            else
                wait(NULL);
        }

        else if ( argnum > 2 && (strcmp("|", argv[1]) == 0 || strcmp( argv[2], "|") == 0 ) ){

            hashistory = 1;
            copyfromto(argv , history);

                int fd[2];
                pipe(fd);
                if (fork() == 0) {
//                    dup2(fd[1],1);
                    close(STDOUT_FILENO);
                    dup(fd[1]);
                    close(fd[0]);
                    close(fd[1]);
                    char *part1[] = {argv[0], NULL}; //only works for single command pipes
                    execvp(part1[0], part1);
                }

                if(fork() == 0){
                    close(STDIN_FILENO);
                    dup(fd[0]);
                    close(fd[1]);
                    close(fd[0]);

                    char *part2[] = {argv [2] , NULL};
                    execvp(part2[0], part2);

                }
                close(fd[0]);
                close(fd[1]);
                wait(0);
                wait(0);
            }




        else if(strcmp(argv[argnum-1], "&")==0){
            hashistory = 1;
            copyfromto(argv, history);
            char *temmpargv[MAX_LINE/2 + 1];
            for (int i = 0; i < argnum - 1 ; ++i) {
                temmpargv[i]=strdup(argv[i]);
            }
            temmpargv[argnum - 1 ]=NULL;

            pid_t pid2= fork();
            if (pid2 == 0)
            {
                FILE *temp= fopen("/dev/null", "w+");
                dup2( fileno(temp) , STDOUT_FILENO);
                dup2( fileno(temp), STDERR_FILENO);
                fclose(temp); //do i really need this??
                execvp(temmpargv[0],temmpargv);
            }

            else if( pid2 > 0){
                // do not wait for child.

            }
        }

        else if(strcmp(argv[0], "exit")==0){
            should_run = 0;
        }
        else if (strcmp ( argv[0],"cd")==0){
            hashistory = 1;
            copyfromto(argv, history);

            if (argv[1]==NULL){
                chdir(getenv("HOME"));
            }
            else if(chdir(argv[1]) < 0){
                printf("cd: %s: No such file or directory\n", argv[1]);
            }
        }





        else {
            hashistory = 1;
            copyfromto(argv , history);
            pid_t pid = fork();
            dup2( STDOUT_FILENO , STDERR_FILENO );

            if (pid == 0) {
                 if(execvp(argv[0], argv)<0){
                    printf("%s: command not found \n" , argv[0]);
                    //                printf("%s \n" , strerror(errno));fd
                }

            }
            else if(pid < 0){
                    // fork errors here
            }
            else {
                wait(pid);
            }
        }


    }
}
