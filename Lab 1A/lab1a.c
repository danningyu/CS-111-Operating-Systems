#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <poll.h>

// preprocessor macros
#define BUFFER_SIZE 256

#define CONTROL_C_SIGINT 3
#define CONTROL_D_EOF 4

#define NORMAL_EXIT 0
#define ARGS_ERROR 1
#define SYS_CALL_FAILURE 1

// global constants
const char* usageMessage = "[--shell] [--debug]";
const char* bashPath = "/bin/bash";

//global vars...can't avoid them
pid_t processID;
struct termios settingsInput;
static int createShell = 0;
static int debugMode = 0;
int control_c_kill = 0;
int sysCallError = 0;

void runSubshell(int* fd1, int* fd2){
    //executed by parent process    
    int fdIn = 0;
    int fdOut = fd1[0];
    char buffer[BUFFER_SIZE];
    struct pollfd pollEvents[] = {
        {fdIn, POLLIN | POLLHUP | POLLERR, 0},
        {fdOut, POLLIN | POLLHUP | POLLERR, 0}
    };
    while(1){
        int pollResult = poll(pollEvents, 2, 0);
        if(pollResult < 0){
            fprintf(stderr, "lab 1A: error creating poll: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(pollEvents[0].revents & POLLIN){
            //read from keyboard
            // fprintf(stderr, "%sReading from keyboard%s", crlf, crlf);
            int fdInAmount = read(fdIn, buffer, BUFFER_SIZE*sizeof(char));
            if(fdInAmount < 0){
                fprintf(stderr, "lab 1A: error reading in text: %s\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            //check for error
            for(int i = 0; i<fdInAmount; i++){
                char currChar = buffer[i];
                switch(currChar){
                    case CONTROL_C_SIGINT:
                        if(write(1, "^C", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            exit(SYS_CALL_FAILURE);
                        }
                        control_c_kill = 1;
                        if(kill(processID, SIGINT)<0){
                            fprintf(stderr, "lab 1A: error killing process %d: %s\n", processID, strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        break;
                    case CONTROL_D_EOF:
                        if(write(1, "^D", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        if(close(fd2[1])<0){
                            fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        break;
                    case '\r':
                    case '\n':
                        if(write(1, "\r\n", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        if(write(fd2[1], "\n", sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                                         
                        break;
                    default:
                        if(write(1, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        if(write(fd2[1], &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                     
                        break;
                }              
            }            
        }
        if(pollEvents[1].revents & POLLIN){
            //the shell output something, send it to stdout
            int fdOutAmount = read(fdOut, buffer, BUFFER_SIZE*sizeof(char));
            if(fdOutAmount < 0){
                fprintf(stderr, "lab 1A: error reading in text: %s\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            for(int i = 0; i<fdOutAmount; i++){
                char currChar = buffer[i];
                switch(currChar){
                    case CONTROL_C_SIGINT:
                        if(write(1, "^C", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        break;
                    case CONTROL_D_EOF:
                        if(write(1, "^D", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        break;
                    case '\r':
                    case '\n':
                        if(write(1, "\r\n", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        break;
                    default:
                        if(write(1, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                      
                        break;
                }              
            }      
        }
        if(pollEvents[1].revents & (POLLHUP | POLLERR)){
            // take in the events using revent and then process them
            //pipes closed, or error when polling
            //after POLLIN!!
            exit(NORMAL_EXIT); //use exit() to invoke exitTerminal()         
        }
        if(pollEvents[0].revents & (POLLHUP | POLLERR)){
            exit(NORMAL_EXIT); //use exit() to invoke exitTerminal()
        }
    }
}

void exitTerminal(){
    if(debugMode){
        fprintf(stderr, "Restoring settings and exiting\r\n");
    }
    int revertTermSettings = tcsetattr(0, TCSANOW, &settingsInput);
    if(revertTermSettings < 0){
        fprintf(stderr, "lab 1A: error restoring terminal settings: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    } 
    if(createShell){
        int shellStatus;
        int waitStatus = waitpid(processID, &shellStatus, 0);
        if(waitStatus < 0){
            fprintf(stderr, "lab 1A: error waiting for shell to exit: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        // int wterm = shellStatus&0x7f;
        // fprintf(stderr, "MODIFIED SHELL EXIT SIGNAL=%d STATUS=%d\n", wterm, WEXITSTATUS(shellStatus));
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(shellStatus), WEXITSTATUS(shellStatus));
        // if(control_c_kill || sysCallError){
        //     exit(1);
        // }
        // else{
        //     exit(NORMAL_EXIT);
        // }
    }
    //for both ^C and ^D, normal exit
    sysCallError += 0;
    exit(NORMAL_EXIT);
}

void sigpipeHandler(int sig_num){
    sig_num += 0;
    exit(NORMAL_EXIT);
}

int main(int argc, char** argv){
    //PART 1: Read in arguments
    int c; //return value for getopt_long
    while(1){
        static struct option long_options[] = {
            {"shell", no_argument, &createShell, 1},
            {"debug", no_argument, &debugMode, 1},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0;
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if(c==-1){
            break;
        }
        if(c=='?'){
            fprintf(stderr, "lab 1A: unrecognized argument %s: usage: %s\n", argv[optind-1], usageMessage);
            exit(ARGS_ERROR);
        }
    }

    if(optind<argc){
        fprintf(stderr, "lab 1A: extraneous arguments found: usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }

    if(createShell && debugMode){
        fprintf(stderr, "Will spawn subshell\r\n");
    }

    //Save current terminal settings, modify and set new terminal
    int resultInput = tcgetattr(0, &settingsInput); // 0 is STDIN
    if(resultInput < 0){
        fprintf(stderr, "lab 1A: error: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    struct termios inputModified = settingsInput;
    inputModified.c_iflag = ISTRIP;
    inputModified.c_oflag = 0;
    inputModified.c_lflag = 0;
    int setTerminal = tcsetattr(0, TCSANOW, &inputModified);
    if(setTerminal < 0){
        fprintf(stderr, "lab 1A: error modifying terminal settings: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }

    // invoke exitTerminal() when exit() is called
    atexit(exitTerminal);

    if(!createShell){ //do not create a shell
        char buffer[BUFFER_SIZE];
        while(1){
            int amountRead = read(0, buffer, BUFFER_SIZE*sizeof(char));
            if(amountRead < 0){
                fprintf(stderr, "lab 1A: error reading in text: %s\n", strerror(errno));
            }
            for(int i = 0; i<amountRead; i++){
                char currChar = buffer[i];
                switch(currChar){
                    // case CONTROL_C_SIGINT:
                    //     if(write(1, "^C", 2*sizeof(char))<0){
                    //         fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                    //         sysCallError = 1;
                    //         exit(SYS_CALL_FAILURE);
                    //     }                       
                    //     //kill signal, ^C - do not kill if --shell not specified
                    //     // kill(processID, SIGINT);
                    //     break;
                    case CONTROL_D_EOF:
                        if(write(1, "^D", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        exit(NORMAL_EXIT);
                        break;
                    case '\r':
                    case '\n':
                        if(write(1, "\r\n", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        break;
                    default:
                        if(write(1, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1A: error writing text: %s\n", strerror(errno)); 
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);                      
                        }                       
                        break;
                }               
            }
        }
    }
    else{
        int fd1[2];
        int fd2[2];
        int pipeResultParent = pipe(fd1);
        if(pipeResultParent < 0){
            fprintf(stderr, "lab 1A: error in creating parent to child pipe: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        int pipeResultChild = pipe(fd2);
        if(pipeResultChild < 0){
            fprintf(stderr, "lab 1A: error in creating child to parent pipe: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(SIG_ERR == signal(SIGPIPE, sigpipeHandler)){
            fprintf(stderr, "lab 1A: error in setting up SIGPIPE signal handler: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        processID = fork();
        if(processID < 0){
            fprintf(stderr, "lab 1A: error in forking: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }   
        else if(processID == 0){
            //child code
            if(close(fd1[0])<0){ //close parent's read
               fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(close(fd2[1])<0){//close child's write
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }


            //now read from parent and write using child
            if(close(0)<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE); 
            }

            if(dup(fd2[0])<0){
                fprintf(stderr, "lab 1A: error in duplicating file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }

            if(close(fd2[0])<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE); 
            }


            if(close(1)<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(dup(fd1[1])<0){ //capture stdout
                fprintf(stderr, "lab 1A: error in duplicating file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(close(2)<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(dup(fd1[1])<0){ //capture stderr
                fprintf(stderr, "lab 1A: error in duplicating file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(close(fd1[1])<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno)); 
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }

            if(execl(bashPath, bashPath, (char*)NULL)<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno)); 
            }
            //this segment shouldn't execute unless execl fails
            fprintf(stderr, "lab 1A: error in opening shell process: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        else{
            //parent code
            if(close(fd1[1])<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE); 
            }

            if(close(fd2[0])<0){
                fprintf(stderr, "lab 1A: error in closing file descriptor: %s\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE); 
            }
            runSubshell(fd1, fd2);
        }
    }
}
