//Danning Yu
//danningyu@ucla.edu
//305087992

#include <sys/wait.h>
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
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"

#define NORMAL_EXIT 0
#define ARGS_ERROR 1
#define SYS_CALL_FAILURE 1

#define CREATE_PERMISSIONS 0666
#define BUFFER_SIZE 4096
#define CHUNK 32768

#define CONTROL_C_SIGINT 3
#define CONTROL_D_EOF 4

#define MAX_LISTEN 5

//debug switch
#define DEBUG 0


#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

const char* usageMessage = "--port=port_number [--compress]";
const char* bashPath = "/bin/bash";

int compressFlag = 0;
pid_t processID;
int sysCallError = 0;
int control_c_kill = 0;
int sockfd, newsockfd;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
int alreadyClosedfd = 0;

void initZlibStreams(){
    //initialize the z-lib streams
    return;   
}

void freeZlibStreams(){
    return;
}


int deflateDY(char* origBuffer, int origBuffSize, char* compBuffer, int compBuffSize){
    //returns the size of the compressed buffer upon completion

    z_stream compStrm;
    compStrm.zalloc = Z_NULL;
    compStrm.zfree = Z_NULL;
    compStrm.opaque = Z_NULL;
    compStrm.avail_in = (uInt)origBuffSize;
    compStrm.next_in = (Bytef*)origBuffer;
    compStrm.avail_out = (uInt)compBuffSize;
    compStrm.next_out = (Bytef*)compBuffer;
    if(deflateInit(&compStrm, Z_DEFAULT_COMPRESSION)<0){
        fprintf(stderr, "Failure initiating compress stream\n");
        exit(1);
    }

    do{
        int defStatus = deflate(&compStrm, Z_SYNC_FLUSH);
        if(defStatus == Z_STREAM_ERROR){
            fprintf(stderr, "stream got messed up compressing\n");
            return defStatus;
        }
        if(defStatus == Z_DATA_ERROR){
            fprintf(stderr, "D invalid or incomplete deflate data: %s\n", compStrm.msg);
            return defStatus;
        }
    } while(compStrm.avail_in > 0);

    int result = compStrm.total_out;
    deflateEnd(&compStrm);
    return result;
}

int inflateDY(char* origBuffer, int origBuffSize, char* uncompBuffer, int uncompBuffSize){
    z_stream decompStrm;
    decompStrm.zalloc = Z_NULL;
    decompStrm.zfree = Z_NULL;
    decompStrm.opaque = Z_NULL;
        decompStrm.avail_in = (uInt)origBuffSize;
    decompStrm.next_in = (Bytef*)origBuffer;
    decompStrm.avail_out = (uInt)uncompBuffSize;
    decompStrm.next_out = (Bytef*) uncompBuffer;
    if(inflateInit(&decompStrm)<0){
        fprintf(stderr, "Failure initiating decompress stream\n");
        exit(1);
    }

    do{
        int infStatus = inflate(&decompStrm, Z_SYNC_FLUSH);
        if(infStatus == Z_STREAM_ERROR){
            fprintf(stderr, "stream got messed up decompressing\n");
            return infStatus;
        }
        if(infStatus == Z_DATA_ERROR){
            fprintf(stderr, "B invalid or incomplete deflate data: %s\n", decompStrm.msg);
            return infStatus;
        }
      } while(decompStrm.avail_in > 0);
    
    int result = decompStrm.total_out;
    inflateEnd(&decompStrm);
    return result;
}

void createServerConnection(const int portNumber){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "lab 1B: error creating socket: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portNumber);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0){
        fprintf(stderr, "lab 1B: error binding to socket: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    listen(sockfd, MAX_LISTEN);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
    if(newsockfd < 0){
        fprintf(stderr, "lab 1B: error accepting connection: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
}

void sigPipeIntHandler(int sig_num){
    if(sig_num == SIGPIPE){
        // fprintf(stderr, "Got a sigpipe!\n");
        exit(NORMAL_EXIT);
    }
    if(sig_num == SIGINT){
        if(DEBUG){
            fprintf(stderr, "got a ^C\n");
        }

        if(kill(processID, SIGINT)<0){
            fprintf(stderr, "lab 1B: error killing shell: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
    }
}

void exitServer(){
    if(compressFlag){
        freeZlibStreams();
    }
    int shellStatus;
    if(DEBUG){
        fprintf(stderr, "Right before waitpid\n");
    }

    int waitStatus = waitpid(processID, &shellStatus, 0);
    if(waitStatus < 0){
        fprintf(stderr, "lab 1B: error waiting for shell to exit: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    // if(WIFEXITED(shellStatus)){
    //     // fprintf(stderr, "Immediate exit\n");
    //     close(sockfd);
    //     close(newsockfd);
    //     fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(shellStatus), WEXITSTATUS(shellStatus));
    //     return;
    // }
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(shellStatus), WEXITSTATUS(shellStatus));
    close(sockfd);
    close(newsockfd);
    // exit(0);
}

int main(int argc, char** argv){
    //lab 1B SERVER

    //PART 1: Read in arguments
    int portNumber = -1;

    int c; //return value for getopt_long
    while(1){
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"compress", no_argument, 0, 'c'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; // turn off getopt's error messages
        c = getopt_long(argc, argv, "p:c", long_options, &option_index);
        if(c==-1){ break; }
        switch(c){
            case 'p':
                portNumber = atoi(optarg);
                // fprintf(stderr, "port option with port num %d\n", portNumber);
                break;
            case 'c':
                compressFlag = 1;
                initZlibStreams();
                // fprintf(stderr, "enabling compression\n");
                break;
            case '?':
                if(optopt == 'p'){
                    fprintf(stderr, "lab 1B usage: --port=port_number has a mandatory argument\n");
                    exit(ARGS_ERROR);
                }
                fprintf(stderr, "lab 1B: unrecognized argument; usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
                break;
            default:
                fprintf(stderr, "lab 1B: unrecognized argument; usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
                break;
        }
    }
    if(optind<argc){
        fprintf(stderr, "lab 1B: extraneous arguments found: usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    if(portNumber == -1){
        fprintf(stderr, "lab 1B: missing port number argument: usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
   
    createServerConnection(portNumber);
    
    atexit(exitServer);

    int fd1[2]; //parent reads from shell, shell writes to this
    int fd2[2]; //shell reads from parent, parent writes to this
    int pipeResultParent = pipe(fd1);
    if(pipeResultParent < 0){
        fprintf(stderr, "lab 1B: error in creating parent to child pipe: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    int pipeResultChild = pipe(fd2);
    if(pipeResultChild < 0){
        fprintf(stderr, "lab 1B: error in creating child to parent pipe: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    if(SIG_ERR == signal(SIGPIPE, sigPipeIntHandler)){
        fprintf(stderr, "lab 1B: error in setting up SIGPIPE signal handler: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }

    if(SIG_ERR==signal(SIGINT, sigPipeIntHandler)){
        fprintf(stderr, "lab 1B: error in setting up SIGPIPE signal handler: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }

    processID = fork();
    if(processID < 0){
        fprintf(stderr, "lab 1B: error in forking: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    else if(processID == 0){
        //child code
        if(close(fd1[0])<0){ //close parent's read
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(close(fd2[1])<0){//close child's write
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }


        //now read from parent and write using child
        if(close(0)<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE); 
        }

        if(dup(fd2[0])<0){
            fprintf(stderr, "lab 1B: error in duplicating file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }

        if(close(fd2[0])<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE); 
        }


        if(close(1)<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(dup(fd1[1])<0){ //capture stdout
            fprintf(stderr, "lab 1B: error in duplicating file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(close(2)<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(dup(fd1[1])<0){ //capture stderr
            fprintf(stderr, "lab 1B: error in duplicating file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(close(fd1[1])<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno)); 
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }

        if(execl(bashPath, bashPath, (char*)NULL)<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno)); 
        }
        //this segment shouldn't execute unless execl fails
        fprintf(stderr, "lab 1B: error in opening shell process: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    else{
        //parent code
        if(close(fd1[1])<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE); 
        }

        if(close(fd2[0])<0){
            fprintf(stderr, "lab 1B: error in closing file descriptor: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE); 
        }
        char cliBuffer[BUFFER_SIZE];
        char shBuffer[BUFFER_SIZE];
        int fdIn = newsockfd;
        int fdOut = fd1[0];
        struct pollfd pollEvents[] = {
            {fdIn, POLLIN | POLLHUP | POLLERR, 0},
            {fdOut, POLLIN | POLLHUP | POLLERR, 0}
        };
        while(1){
            int pollResult = poll(pollEvents, 2, 0);
            if(pollResult < 0){
                fprintf(stderr, "lab 1B: error creating poll: %s\r\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(pollEvents[0].revents & POLLIN){
                //socket sent something over
                int sockInAmt = read(fdIn, cliBuffer, BUFFER_SIZE*sizeof(char));
                if(sockInAmt < 0){
                    fprintf(stderr, "lab 1B: error reading in text: %s\n", strerror(errno));
                    sysCallError = 1;
                    exit(SYS_CALL_FAILURE);
                }
                if(sockInAmt == 0){
                    if(DEBUG){
                        fprintf(stderr, "no bytes read\n");
                    }
                    
                    if(!alreadyClosedfd){
                        if(close(fd2[1])<0){
                            //close server's end of pipe
                            fprintf(stderr, "2 lab 1B: error in closing file descriptor: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        alreadyClosedfd = 1;
                    }  
                }
                if(compressFlag){
                    if(DEBUG){
                     fprintf(stderr, "Decompressing client input\n");
                    }
                    char result[CHUNK];
                    int decompressedAmt = inflateDY(cliBuffer, sockInAmt, result, CHUNK);
                    if(decompressedAmt < 0 && DEBUG){
                        fprintf(stderr, "Decompression issue\n");
                    }
                    // fprintf(stderr, "Got %d bytes\n", decompressedAmt);
                    // write(1, result, decompressedAmt);
                    for(int i = 0; i<decompressedAmt; i++){
                        char currChar = result[i];
                        switch(currChar){
                            case CONTROL_C_SIGINT:
                                control_c_kill = 1;
                                if(kill(processID, SIGINT)<0){
                                    fprintf(stderr, "lab 1B: error killing process %d: %s\n", processID, strerror(errno));
                                    sysCallError = 1;
                                    exit(SYS_CALL_FAILURE);
                                }
                                break;
                            case CONTROL_D_EOF:
                                if(DEBUG){
                                    fprintf(stderr, "^D location 1");
                                }
                                
                                if(!alreadyClosedfd){
                                    if(close(fd2[1])<0){
                                        //close server's end of pipe
                                        fprintf(stderr, "2 lab 1B: error in closing file descriptor: %s\n", strerror(errno));
                                        sysCallError = 1;
                                        exit(SYS_CALL_FAILURE);
                                    }
                                    alreadyClosedfd = 1;
                                }          
                                
                                break;
                            case '\r':
                            case '\n':
                                if(write(fd2[1], "\n", sizeof(char))<0){
                                    fprintf(stderr, "lab 1B: 1 error writing text: %s\n", strerror(errno));
                                    sysCallError = 1;
                                    exit(SYS_CALL_FAILURE);
                                }                                         
                                break;
                            default:
                                if(write(fd2[1], &currChar, sizeof(char))<0){
                                    fprintf(stderr, "lab 1B: 2 error writing text: %s\n", strerror(errno));
                                    sysCallError = 1;
                                    exit(SYS_CALL_FAILURE);
                                }                     
                                break;
                        } 
                    }
                }
                else{ //no compression
                    for(int i = 0; i<sockInAmt; i++){
                        char currChar = cliBuffer[i];
                        switch(currChar){
                            case CONTROL_C_SIGINT:
                                control_c_kill = 1;
                                if(kill(processID, SIGINT)<0){
                                    fprintf(stderr, "lab 1B: error killing process %d: %s\n", processID, strerror(errno));
                                    sysCallError = 1;
                                    exit(SYS_CALL_FAILURE);
                                }
                                break;
                            case CONTROL_D_EOF:
                                if(DEBUG){
                                     fprintf(stderr, "^D location 2");  
                                }  
          
                                if(!alreadyClosedfd){
                                    if(close(fd2[1])<0){
                                        //close server's end of pipe
                                        fprintf(stderr, "2 lab 1B: error in closing file descriptor: %s\n", strerror(errno));
                                        sysCallError = 1;
                                        exit(SYS_CALL_FAILURE);
                                    }
                                    alreadyClosedfd = 1;
                                }          
                                
                                break;
                            case '\r':
                            case '\n':
                                if(write(fd2[1], "\n", sizeof(char))<0){
                                    fprintf(stderr, "lab 1B: 3 error writing text: %s\n", strerror(errno));
                                    sysCallError = 1;
                                    exit(SYS_CALL_FAILURE);
                                }                                         
                                break;
                            default:
                                if(write(fd2[1], &currChar, sizeof(char))<0){
                                    fprintf(stderr, "lab 1B: 4 error writing text: %s\n", strerror(errno));
                                    sysCallError = 1;
                                    exit(SYS_CALL_FAILURE);
                                }                     
                                break;
                        } 
                    }
                }       
            }
            if(pollEvents[1].revents & POLLIN){
                //shell sent something over
                int fdOutAmount = read(fdOut, shBuffer, BUFFER_SIZE*sizeof(char));
                
                if(fdOutAmount < 0){
                    fprintf(stderr, "1 lab 1B: error reading in text: %s\n", strerror(errno));
                    sysCallError = 1;
                    exit(SYS_CALL_FAILURE);
                }
                //for debugging purposes ONLY
                if(DEBUG){
                    write(1, shBuffer, fdOutAmount);
                }

                
                if(fdOutAmount == 0){
                    //shell has died
                    // fprintf(stderr, "Read nothing from shell\n");
                    exit(NORMAL_EXIT);
                }
                if(compressFlag){
                    //compress data and then send
                    char result[CHUNK];
                    int compressedAmt = deflateDY(shBuffer, fdOutAmount, result, CHUNK);
                    if(compressedAmt<0){
                        fprintf(stderr, "Decompression issue\n");
                        sysCallError = 1;
                        exit(SYS_CALL_FAILURE);
                    }
                    if(DEBUG){
                        fprintf(stderr, "writing %d bytes\n", compressedAmt);
                    }
                    if(write(newsockfd, result, compressedAmt)<0){
                        fprintf(stderr, "lab 1B: error writing compressed text: %s\n", strerror(errno));
                        sysCallError = 1;
                        exit(SYS_CALL_FAILURE);
                    }
                    if(DEBUG){
                        write(1, result, compressedAmt);
                    }

                }
                else{
                    if(write(newsockfd, shBuffer, fdOutAmount)<0){
                        fprintf(stderr, "lab 1B: 5 error writing text: %s\n", strerror(errno));
                        sysCallError = 1;
                        exit(SYS_CALL_FAILURE);
                    }
                }
                
            }
            if(pollEvents[0].revents & (POLLHUP | POLLERR)){
                if(DEBUG){
                    fprintf(stderr, "^D location 3"); 
                }
           
                if(!alreadyClosedfd){
                    if(close(fd2[1])<0){
                        //close server's end of pipe
                        fprintf(stderr, "2 lab 1B: error in closing file descriptor: %s\n", strerror(errno));
                        sysCallError = 1;
                        exit(SYS_CALL_FAILURE);
                    }
                    alreadyClosedfd = 1;
                }          
                                
                exit(NORMAL_EXIT);
            }

            if(pollEvents[1].revents & (POLLHUP | POLLERR)){
                // int finalReadAmt = read(sockfd, shBuffer, BUFFER_SIZE);
                // if(finalReadAmt<0){
                //     fprintf(stderr, "2 lab 1B: error reading from shell: %s\n", strerror(errno));
                //     sysCallError = 1;
                //     exit(SYS_CALL_FAILURE);
                // }
                //should this input also be logged? decompressed?
                // if(write(newsockfd, shBuffer, finalReadAmt)<0){
                //     fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                //     sysCallError = 1;
                //     exit(SYS_CALL_FAILURE);
                // }
                exit(NORMAL_EXIT);
            }
        }
    }
    exit(NORMAL_EXIT);
}