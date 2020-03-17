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

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define NORMAL_EXIT 0
#define ARGS_ERROR 1
#define SYS_CALL_FAILURE 1
#define CREATE_PERMISSIONS 0666

#define BUFFER_SIZE 4096
#define CHUNK 32768
#define DOUBLE_CHUNK 32768
//next size up: 32768

#define CONTROL_C_SIGINT 3
#define CONTROL_D_EOF 4

#define DEBUG 0

const char* usageMessage = "--port=port_number [--log=filename] [--compress]";

int compressFlag = 0;
struct termios settingsInput;
int sysCallError = 0;
int sockfd;
struct sockaddr_in serv_addr;
struct hostent* server;



void initZlibStreams(){
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
        fprintf(stderr, "Failure initiating compress stream");
        exit(1);
    }

     do{
         int defStatus = deflate(&compStrm, Z_SYNC_FLUSH);
        // zerr(defStatus);
        // int defStatus = deflate(&compStrm, Z_FINISH);
        // if (defStatus == Z_STREAM_END){ //if got to the end of the Z_STREAM_END
        //     result = (int)compStrm.total_out; //total number of output bytes
        // }
        if(defStatus == Z_STREAM_ERROR){
            fprintf(stderr, "Stream got messed up compressing\r\n");
            return defStatus;
        }
        else if(defStatus == Z_DATA_ERROR){
            fprintf(stderr, "Invalid or incomplete deflate data: %s\r\n", compStrm.msg);
            return defStatus;
        }
     } while(compStrm.avail_in > 0);
    // amtCompressed = compBuffSize - compStrm.avail_out;
    // fprintf(stderr, "decompress value 1: %ld, value 2: %d\r\n", compStrm.total_out, amtCompressed);
    deflateEnd(&compStrm);
    return compStrm.total_out;
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
        fprintf(stderr, "Failure initiating compress stream");
        exit(1);
    }

    do{
        int infStatus = inflate(&decompStrm, Z_SYNC_FLUSH);
        if(infStatus == Z_STREAM_ERROR){
            fprintf(stderr, "Stream got messed up decompressing\r\n");
            return infStatus;
        }
        if(infStatus == Z_DATA_ERROR){
            fprintf(stderr, "Invalid or incomplete deflate data: %s\r\n", decompStrm.msg);
            return infStatus;
        }
    } while(decompStrm.avail_in > 0);
    // amtDecomp = uncompBuffSize - decompStrm.avail_out;
    // fprintf(stderr, "compress value 1: %ld, value 2: %d\r\n", decompStrm.total_out, amtDecomp);
    int result = decompStrm.total_out;
    inflateEnd(&decompStrm);
    return result;
}

void exitTerminal(){
    if(compressFlag){
        freeZlibStreams();
    }

    int revertTermSettings = tcsetattr(0, TCSANOW, &settingsInput);
    if(revertTermSettings < 0){
        fprintf(stderr, "lab 1B: error restoring terminal settings: %s\r\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    shutdown(sockfd, SHUT_RDWR);
    // if(sysCallError){
    //     exit(SYS_CALL_FAILURE);
    // }
    // exit(NORMAL_EXIT);
}

void sigpipeHandler(int sig_num){
    sig_num += 0;
    fprintf(stderr, "SIGPIPE occurred\r\n");
    exit(NORMAL_EXIT);
}

void createConnection(const int portNumber){
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
        fprintf(stderr, "lab 1B: error creating socket: %s\r\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    server = gethostbyname("localhost");
    if(server == NULL){
       fprintf(stderr, "lab 1B: error getting host name: %s\r\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portNumber);
    if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0){
        fprintf(stderr, "lab 1B: error creating TCP connection: %s\r\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
}

void getAndSetTerminal(){
    //Save current terminal settings, modify and set new terminal
    int resultInput = tcgetattr(0, &settingsInput); // 0 is STDIN
    if(resultInput < 0){
        fprintf(stderr, "lab 1B: error getting terminal settings: %s\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
    struct termios inputModified = settingsInput;
    inputModified.c_iflag = ISTRIP;
    inputModified.c_oflag = 0;
    inputModified.c_lflag = 0;
    int setTerminal = tcsetattr(0, TCSANOW, &inputModified);
    if(setTerminal < 0){
        fprintf(stderr, "lab 1B: error modifying terminal settings: %s\r\n", strerror(errno));
        sysCallError = 1;
        exit(SYS_CALL_FAILURE);
    }
}

int main(int argc, char** argv){
    //lab 1B CLIENT
    //PART 1: Read in arguments
    char* logFilePath = NULL;
    int portNumber = -1;

    int shouldLog = 0;
    int c; //return value for getopt_long
    while(1){
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {"log", required_argument, 0, 'l'},
            {"compress", no_argument, 0, 'c'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; // turn off getopt's error messages
        c = getopt_long(argc, argv, "p:l:c", long_options, &option_index);
        if(c==-1){ break; }
        switch(c){
            case 'p':
                portNumber = atoi(optarg);
                // fprintf(stderr, "port option with port num %d\n", portNumber);
                break;
            case 'l':               
                logFilePath = optarg;
                shouldLog = 1;
                // fprintf(stderr, "Logging comms to %s\n", logFilePath);
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
                else if(optopt == 'l'){
                    fprintf(stderr, "lab 1B usage: --log=filename has a mandatory argument\n");
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

    getAndSetTerminal();

    atexit(exitTerminal);
    if(SIG_ERR == signal(SIGPIPE, sigpipeHandler)){
            fprintf(stderr, "lab 1A: error in setting up SIGPIPE signal handler: %s\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
    }

    createConnection(portNumber);

    int logfd;
    if(logFilePath != NULL){
        logfd = creat(logFilePath, CREATE_PERMISSIONS);
        if(logfd < 0){            
            fprintf(stderr, "lab 1B: error creating log file: %s\r\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
    }

    //establish a poll
    struct pollfd pollEvents[] = {
        {STDIN_FILENO, POLLIN | POLLHUP | POLLERR, 0}, //keyboard
        {sockfd, POLLIN | POLLHUP | POLLERR, 0} //server
    };

    char numBuffer[50];
    char kbBuffer[BUFFER_SIZE];
    char servBuffer[BUFFER_SIZE];
    while(1){
        int pollResult = poll(pollEvents, 2, 0);
        if(pollResult < 0){
            fprintf(stderr, "lab 1B: error creating poll: %s\r\n", strerror(errno));
            sysCallError = 1;
            exit(SYS_CALL_FAILURE);
        }
        if(pollEvents[0].revents & POLLIN){
            //read from keyboard
            // fprintf(stderr, "pollevent0\r\n");
            int kbInAmt = read(STDIN_FILENO, kbBuffer, BUFFER_SIZE);
            if(kbInAmt<0){
                fprintf(stderr, "lab 1B: error reading from STDIN: %s\r\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(compressFlag){
                char result[CHUNK];
                int compressedAmt = deflateDY(kbBuffer, kbInAmt, result, CHUNK);
                if(compressedAmt<0){
                    fprintf(stderr, "Compression issue\r\n");
                    sysCallError = 1;
                    exit(SYS_CALL_FAILURE);
                }
                // fprintf(stderr, "Sending over %d bytes\r\n", compressedAmt);
                if(write(sockfd, result, compressedAmt)<0){
                    fprintf(stderr, "lab 1B: error writing compressed text: %s\n", strerror(errno));
                    sysCallError = 1;
                    exit(SYS_CALL_FAILURE);
                }
                if(shouldLog){
                    write(logfd, "SENT ", 5);
                    int numLen = sprintf(numBuffer, "%d", compressedAmt);
                    write(logfd, numBuffer, numLen);
                    write(logfd, " bytes: ", 8);
                    write(logfd, result, compressedAmt);
                    write(logfd, "\n", 1);
                }
            }

            if(shouldLog && !compressFlag){ //log uncompressed output
                write(logfd, "SENT ", 5);
                int numLen = sprintf(numBuffer, "%d", kbInAmt);
                write(logfd, numBuffer, numLen);
                write(logfd, " bytes: ", 8);
                write(logfd, kbBuffer, kbInAmt);
                write(logfd, "\n", 1);
            }
            for(int i = 0; i<kbInAmt; i++){
                char currChar = kbBuffer[i];
                switch(currChar){
                    case CONTROL_C_SIGINT:
                        if(write(STDOUT_FILENO, "^C", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        if(!compressFlag && write(sockfd, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                       
                        break;
                    case CONTROL_D_EOF:
                        if(write(STDOUT_FILENO, "^D", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        if(!compressFlag && write(sockfd, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }    
                        break;
                    case '\r':
                    case '\n':
                        if(write(STDOUT_FILENO, "\r\n", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        if(!compressFlag && write(sockfd, "\n", sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                                         
                        break;
                    default:
                        if(write(STDOUT_FILENO, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        if(!compressFlag && write(sockfd, &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                     
                        break;
                }
            }               
        }

        if(pollEvents[1].revents & POLLIN){
            //read from socket
            //
            if(DEBUG){
                fprintf(stderr, "pollevent1\r\n");
            }

            int sockInAmt = read(sockfd, servBuffer, BUFFER_SIZE);

            //DEBUGGING ONLY:
            if(DEBUG){
                write(STDERR_FILENO, servBuffer, sockInAmt);
                fprintf(stderr, "Read %d amount\r\n", sockInAmt);
            }

            if(sockInAmt<0){
                fprintf(stderr, "lab 1B: error reading from socket: %s\r\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(sockInAmt == 0){
                //is this code needed?
                //received EOF from socket: no more to read
                if(DEBUG){
                    fprintf(stderr, "Read nothing from socket\r\n");
                }
                
                exit(NORMAL_EXIT);
            }
            if(shouldLog){ //doesn't matter if data is compressed or not, log it
                write(logfd, "RECEIVED ", 9);
                int numLen = sprintf(numBuffer, "%d", sockInAmt);
                write(logfd, numBuffer, numLen);
                write(logfd, " bytes: ", 8);
                write(logfd, servBuffer, sockInAmt);
                write(logfd, "\n", 1);
            }
            if(compressFlag){
                //sockfd is always outputing data but for some reason compression only reads from sockfd once
                //and then idk where the code goes
                char result[CHUNK];
                int decompressedAmt = inflateDY(servBuffer, sockInAmt, result, CHUNK);
                if(decompressedAmt<0){
                        fprintf(stderr, "Decompression issue, but continuing\r\n");
                }
                if(DEBUG){
                    fprintf(stderr, "decompressed socket input\r\n");
                }

                // if(shouldLog){
                //     write(logfd, "RECEIVED ", 9);
                //     int numLen = sprintf(numBuffer, "%d", decompressedAmt);
                //     write(logfd, numBuffer, numLen);
                //     write(logfd, " bytes: ", 8);
                //     write(logfd, result, decompressedAmt);
                //     write(logfd, "\n", 1);
                // }
                for(int i = 0; i<decompressedAmt; i++){              
                    char currChar = result[i];
                    switch(currChar){
                        case CONTROL_C_SIGINT: //prob not needed b/c shell will never send ^C
                            if(write(STDOUT_FILENO, "^C", 2*sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }              
                            break;
                        case CONTROL_D_EOF: //same as ^C but for ^D
                            if(write(STDOUT_FILENO, "^D", 2*sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }
                            break;
                        case '\r':
                        case '\n':
                            if(write(STDOUT_FILENO, "\r\n", 2*sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }                             
                            break;
                        default:
                            if(write(STDOUT_FILENO , &currChar, sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }        
                            break;
                    }
                }
                if(DEBUG){
                    fprintf(stderr, "finished printing original compressed data\r\n");
                }
 
            }
            else{ //data from server is not compressed
                if(DEBUG){
                    fprintf(stderr, " printing original uncompressed data\r\n"); 
                }
                for(int i = 0; i<sockInAmt; i++){              
                    char currChar = servBuffer[i];
                    switch(currChar){
                        case CONTROL_C_SIGINT:
                            if(write(STDOUT_FILENO, "^C", 2*sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }              
                            break;
                        case CONTROL_D_EOF:
                            if(write(STDOUT_FILENO, "^D", 2*sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }
                            break;
                        case '\r':
                        case '\n':
                            if(write(STDOUT_FILENO, "\r\n", 2*sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }                             
                            break;
                        default:
                            if(write(STDOUT_FILENO , &currChar, sizeof(char))<0){
                                fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                                sysCallError = 1;
                                exit(SYS_CALL_FAILURE);
                            }        
                            break;
                    }
                } 
            }
            if(DEBUG){
               fprintf(stderr, "done polling from socket\r\n"); 
            }
                       
        }

        if(pollEvents[0].revents & (POLLHUP | POLLERR)){
            // fprintf(stderr, "keyboard died\n");
                        // fprintf(stderr, "pollevent2\r\n");
            exit(NORMAL_EXIT);
        }

        if(pollEvents[1].revents & (POLLHUP | POLLERR)){
            //server shut down, read anything remaining and then exit
                        // fprintf(stderr, "pollevent3\r\n");
            int finalReadAmt = read(sockfd, servBuffer, BUFFER_SIZE);
            if(finalReadAmt<0){
                fprintf(stderr, "lab 1B: error reading from STDIN: %s\r\n", strerror(errno));
                sysCallError = 1;
                exit(SYS_CALL_FAILURE);
            }
            if(finalReadAmt == 0){
                // fprintf(stderr, "read nothing on shutdown\n");
                exit(NORMAL_EXIT);
            }
            //should this input also be logged? decompressed?
            if(finalReadAmt>0){
                // fprintf(stderr, "doing last bit of reading\n");
            }
            for(int i = 0; i<finalReadAmt; i++){              
                char currChar = servBuffer[i];
                switch(currChar){
                    case CONTROL_C_SIGINT:
                        if(write(STDOUT_FILENO, "^C", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }              
                        break;
                    case CONTROL_D_EOF:
                        if(write(STDOUT_FILENO, "^D", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }
                        break;
                    case '\r':
                    case '\n':
                        if(write(STDOUT_FILENO, "\r\n", 2*sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }                             
                        break;
                    default:
                        if(write(STDOUT_FILENO , &currChar, sizeof(char))<0){
                            fprintf(stderr, "lab 1B: error writing text: %s\r\n", strerror(errno));
                            sysCallError = 1;
                            exit(SYS_CALL_FAILURE);
                        }        
                        break;
                }
            } 
            //not sure if exit(1) is correct...
            // fprintf(stderr, "socket dead\n");
            exit(NORMAL_EXIT);
        }
    }
    exit(NORMAL_EXIT);
}