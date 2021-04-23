/*
 * CT30A3370 Käyttöjärjestelmät ja systeemiohjelmointi
 * Project 3: Unix Shell
 * Creator: Ville Felin
 *
 * With a little bit of help from the course material (mainly child processes)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define True 1
#define DEFARGS 8

//pointer are 64 bits in 64-bit systems
#define ptrSize 8


int main(int argc, char *argv[]) {

    FILE *input;
    char error_message[50] = "An error has occurred\n";
    size_t line;
    char *tempVariable;

    //delimiters for strtok when parsing commands
    char delimiters[]=" \t\r\n\v\f";
    char *token;

    //dynamic pointer system for paths
    int pathCount = 1;
    char **paths = malloc(pathCount * ptrSize);
    if (paths == NULL) {
        strcpy(error_message, "malloc: error\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    tempVariable = malloc(sizeof("/bin"));
    if (tempVariable == NULL) {
        strcpy(error_message, "malloc: error\n");
        exit(1);
    }
    strcpy(tempVariable, "/bin");
    paths[0] = tempVariable;
    char *currentPath;

    //for child processes
    pid_t pid = getpid();
    int status;
    int i;


    //Dynamic pointer system for arguments
    int argumentCount = 0;
    int argumentMax = DEFARGS;
    char **arguments = malloc(argumentMax * ptrSize);
    if (arguments == NULL) {
        strcpy(error_message, "malloc: error\n");
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    //no batch file, using stdin
    if (argc == 1) {
        input = stdin;
    }

    //executing batch-file
    else if (argc == 2) {
        if ((input = fopen(argv[1], "r")) == NULL) {
            strcpy(error_message, "wish: cannot open file\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
    }

    else {
        printf("usage: wish [file]");
        exit(1);
    }


    while(True) {

        if (argc == 1) printf("wish> ");

        //buffer for the getline
        size_t bufferSize = 0;
        char *buffer = malloc(bufferSize * sizeof(char));
        if (buffer == NULL) {
            strcpy(error_message, "malloc: error\n");
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        //Using getline() to get the whole line
        line = getline(&buffer, &bufferSize, input);

        //EOF
        if (line == -1) {

            for (i = 0; i < pathCount; i++) free(paths[i]);
            for (i = 0; i < argumentCount; i++) free(arguments[i]);
            exit(0);
            free(buffer);
            break;

        }

        //removing newline
        buffer[strcspn(buffer, "\n")] = 0;

        //Emptying arguments array
        for (i = 0; i < argumentCount; i++) free(arguments[i]);
        for (i = 0; i < argumentMax; i++) arguments[i] = 0;

        //resetting the size of array for arguments
        if (argumentMax > DEFARGS) {
            arguments = realloc(arguments, DEFARGS * ptrSize);
            if (arguments == NULL) {
                strcpy(error_message, "realloc: error\n");
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            argumentMax = DEFARGS;
        }

        if (*buffer){
            argumentCount = 0;

            for (i = 0; (token = strtok_r(buffer, delimiters, &buffer)) != 0; i++) {

                //growing the argument array if it gets too small
                if (i == argumentMax) {

                    argumentMax = argumentMax * 2;

                    arguments = realloc(arguments, (argumentMax * ptrSize));
                    if (arguments == NULL) {
                        strcpy(error_message, "realloc: error\n");
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(1);
                    }


                }


                if (strcmp(token, "&") == 0) {
                    //TODO: parallel executions
                }

                //copying the token to dynamically allocated slot in the arguments array
                tempVariable = malloc(sizeof(token));
                if (tempVariable == NULL) {
                    strcpy(error_message, "malloc: error\n");
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
                strcpy(tempVariable, token);
                arguments[i] = tempVariable;
                argumentCount++;

            }


            //"path"-built-in command
            if (strcmp(arguments[0], "path") == 0) {

                //emptying the paths array
                for (i = 0; i < pathCount; i++) free(paths[i]);

                paths = realloc(paths, (argumentCount - 1) * ptrSize);
                if (paths == NULL) {
                    strcpy(error_message, "realloc: error\n");
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
                pathCount = argumentCount - 1;

                //copying the arguments to the paths array
                for (i = 1; i < argumentCount; i++) {

                    tempVariable = malloc(sizeof(arguments[i]));
                    if (tempVariable == NULL) {
                        strcpy(error_message, "malloc: error\n");
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(1);
                    }

                    if (arguments[i] != 0) strcpy(tempVariable, arguments[i]);

                    paths[i-1] = tempVariable;

                }
            }

                //"exit"-built-in command
            else if (strcmp(arguments[0], "exit") == 0) {

                for (i = 0; i < pathCount; i++) free(paths[i]);
                for (i = 0; i < argumentCount; i++) free(arguments[i]);
                exit(0);

            }

                //"cd"-built-in command
            else if (strcmp(arguments[0], "cd") == 0) {

                //error messages
                if (argumentCount > 2) strcpy(error_message, "cd: Too many arguments\n");
                if (argumentCount < 2) strcpy(error_message, "usage: cd 'directory'\n");

                //wrong argument count
                if (argumentCount != 2) {

                    write(STDERR_FILENO, error_message, strlen(error_message));

                }


                else if (chdir(arguments[1]) == -1) {

                    //if chdir did not work
                    strcpy(error_message, "cd: error\n");
                    write(STDERR_FILENO, error_message, strlen(error_message));

                }

            }

            else {

                //if the program is in the same directory
                if (arguments[0][0] == '.') {


                    currentPath = malloc(sizeof(arguments[0]));
                    strcpy(currentPath, "");
                    strcat(currentPath, arguments[0]);

                }

                    //Testing access to program
                else for (i = 0; i < pathCount; i++) {

                        currentPath = malloc(sizeof(paths[i]) + sizeof(arguments[0]) + 1*sizeof(char));

                        strcpy(currentPath, paths[i]);

                        strcat(currentPath, "/");
                        strcat(currentPath, arguments[0]);

                        //if an accessible file is found, break
                        if (!access(currentPath, X_OK)) break;

                    }

                //executing the commands using child process
                switch (pid = fork()) {

                    case -1:              /* error in fork */
                        strcpy(error_message, "Something went wrong when forking\n");
                        write(STDERR_FILENO, error_message, strlen(error_message));

                    case 0:               /* child process */
                        if ((execv(currentPath, arguments)) == -1) {
                            strcpy(error_message, "execv: Something went wrong\n");
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(1);
                        }
                        break;

                    default:              /* parent process */
                        if (wait(&status) == -1) {
                            strcpy(error_message, "Something went wrong when waiting\n");
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(1);
                        }
                        break;

                }

                free(currentPath);
                
            }
        }
        else free(buffer);
    }
}

