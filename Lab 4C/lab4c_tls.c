// Danning Yu
// 305087992
// CS 111 Lab 4C TLS (Part 2)

// Need to test the code!!

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// compile using -lm, -lmraa
#include <math.h>
#include <mraa.h>

// compile using -lssl, -lcrypto
#include <openssl/ssl.h>
#include <openssl/err.h>

#define F_SCALE 0
#define C_SCALE 1

#define DEBUG 0

#define AIO_01_PIN_VALUE 1 //for the temperature sensor
#define BUFFER_SIZE 1024

#define NORMAL_EXIT 0
#define SYS_CALL_ERROR 1
#define ARGS_ERROR 1
#define OTHER_ERROR 2
#define CREATE_PERMISSIONS 0666
#define TIMEOUT 100

// Temperature constants
const int BETA = 4275;
const float R0 = 100000.0;
const float T0 = 298.15;
const float K_TO_C = 273.15;

const char* usageMessage = "port_number --id=9_digit_num --host=hostname --log=filename [--period=#] [--scale=[CF]]";

//global context variables
int degreeScale = F_SCALE; //default is farenheit
int samplingFreq = 1; //default, 1 data point per second
char* logFilePath = NULL; //filepath for logging
int logFileFD = -1;
char* clientID = NULL;
char* hostName = NULL;
int portNumber = -1;

// TCP testing server: port = 18000, hostname = lever.cs.ucla.edu
// TLS testing server: port = 19000, hostname = lever.cs.ucla.edu

mraa_result_t status;
mraa_aio_context tempSensor = NULL;
// mraa_gpio_context buttonSensor;
char stdinBuffer[BUFFER_SIZE] = {0};
char logFileBuffer[BUFFER_SIZE] = {0};
char completeArg[BUFFER_SIZE] = {0};
struct timeval currentTv;
struct timeval readyToReadTv;
struct tm* timeStampTime;
int shouldRead = 1;
int sockfd;

struct sockaddr_in serv_addr;
struct hostent* server;

//SSL connection variables
SSL_CTX* sslCTX;
int sslServer;
SSL* sslObject;
const SSL_METHOD* sslMethod;

int isNineDigitNum(char* input){
    if(strlen(input) != 9){
        return 0;
    }
    int i;
    for(i = 0; i<9; i++){
        if(!isdigit(input[i])){
            return 0;
        }
    }
    return 1;
}

void createConnection(const int portNumber){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0){
        fprintf(stderr, "lab 4C: error creating socket: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    server = gethostbyname(hostName);
    if(server == NULL){
        fprintf(stderr, "lab 4C: error getting host name: %s\n", strerror(errno));
        exit(OTHER_ERROR);
    }
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portNumber);
    if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0){
        fprintf(stderr, "lab 4C: error creating TCP connection: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
}

//CHECK WHERE ALL FPRINTFS ARE PRINTING TO!!!
void shutdownHandler(){
    if(DEBUG){
        fprintf(stderr, "DEBUG: Button got pressed or OFF cmd sent\n");        
    }

    if(gettimeofday(&currentTv, NULL) < 0){
        fprintf(stderr, "Failed to get current time: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    timeStampTime = localtime(&currentTv.tv_sec);
    if(timeStampTime == NULL){
        fprintf(stderr, "Failed to get current date and time\n");
        exit(OTHER_ERROR);
    }
    char shutdownBuffer[BUFFER_SIZE] = {0};
    int amtWritten = sprintf(shutdownBuffer, "%02d:%02d:%02d SHUTDOWN\n", timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec);
    if(SSL_write(sslObject, shutdownBuffer, amtWritten)<0){
        fprintf(stderr, "lab 4C: Failed to write to stdout\n");
        exit(SYS_CALL_ERROR);
    }
    if(logFilePath != NULL){
        int amtWritten = sprintf(logFileBuffer, "%02d:%02d:%02d SHUTDOWN\n", 
          timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec);
        if(write(logFileFD, logFileBuffer, amtWritten)<0){
            fprintf(stderr, "lab 4C: 1 Failed to write to log: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }        
    }
    
    exit(NORMAL_EXIT);
}

void processArgument(char* argument){
    if(DEBUG){
        fprintf(stderr, "Processing arg: %s\n", argument);
    }
    if(logFilePath != NULL){
        int amtWritten = sprintf(logFileBuffer, "%s\n", argument);
        if(write(logFileFD, logFileBuffer, amtWritten)<0){
            fprintf(stderr, "lab 4C: 2 Failed to write to log: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }
    }
    // param guaranteed to contain \0, so can consider as C string
    if(strcmp(argument, "SCALE=F") == 0){
        degreeScale = F_SCALE;
    }
    else if(strcmp(argument, "SCALE=C") == 0){
        degreeScale = C_SCALE;
    }
    else if(strstr(argument, "PERIOD=") != NULL){
        int newSamplingFreq = atoi(argument+7*sizeof(char));
        if(newSamplingFreq <= 0){
        fprintf(stderr, "lab 4C: sampling frequency must be at least 1\n");
        exit(ARGS_ERROR);
        }
        samplingFreq = newSamplingFreq;
    }
    else if(strcmp(argument, "STOP") == 0){
        shouldRead = 0;
    }
    else if(strcmp(argument, "START") == 0){
        shouldRead = 1;
    }
    else if(strncmp(argument, "LOG", 3) == 0){
        ;
    }
    else if(strcmp(argument, "OFF") == 0){
        shutdownHandler();
    }
    //otherwise, do nothing...
}

void createSSLConnection(){
    if(SSL_library_init()<0){
        fprintf(stderr, "lab 4C: Error initializing SSL library\n");
        exit(OTHER_ERROR);
    }
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    sslMethod = SSLv23_client_method();
    sslCTX = SSL_CTX_new(sslMethod);
    if(sslCTX == NULL){
        fprintf(stderr, "lab 4C: error creating SSL context\n");
        exit(OTHER_ERROR);
    }
    sslObject = SSL_new(sslCTX);
    if(sslObject == NULL){
        fprintf(stderr, "lab 4C: error creating SSL object\n");
        exit(OTHER_ERROR);
    }
    if(SSL_set_fd(sslObject, sockfd) == 0){
        fprintf(stderr, "lab 4C: error binding SSL to socket\n");
        exit(OTHER_ERROR);
    }
    if(SSL_connect(sslObject)<=0){
        SSL_shutdown(sslObject);
        SSL_free(sslObject);
        SSL_CTX_free(sslCTX);
        fprintf(stderr, "lab 4C: error starting SSL connection\n");
        exit(OTHER_ERROR);
    }
}

void cleanupAndExit(){
    if(DEBUG){
        fprintf(stderr, "DEBUG: EXITING!\n");
    }
    SSL_shutdown(sslObject);
    SSL_free(sslObject);
    SSL_CTX_free(sslCTX);
    if(tempSensor != NULL){
        status = mraa_aio_close(tempSensor);
        if(status != MRAA_SUCCESS){
            exit(OTHER_ERROR);
        }  
    }

    if(logFileFD != -1){
    	if(close(logFileFD)<0){
    		fprintf(stderr, "Error closing file descriptor: %s\n", strerror(errno));
    		exit(SYS_CALL_ERROR);
    	}    	
    }
}

int main(int argc, char** argv){
    int c;
    while(1){
        static struct option long_options[] = {
            {"period", required_argument, 0, 'p'},
            {"scale", required_argument, 0, 's'},
            {"log", required_argument, 0, 'l'},
            {"id", required_argument, 0, 'i'},
            {"host", required_argument, 0, 'h'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; //suppress getopt_long error messages, use my own msgs instead
        c = getopt_long(argc, argv, "p:s:l:i:h:", long_options, &option_index);
        if(c==-1){
            break;
        }
        switch(c){
            case 'p':
                samplingFreq = atoi(optarg);
                break;
            case 's':
                if(strlen(optarg)>1){
                    fprintf(stderr, "lab 4C usage: bad argument for --scale: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                if(optarg[0] == 'F'){
                    degreeScale = F_SCALE;
                }
                else if(optarg[0] == 'C'){
                    degreeScale = C_SCALE;
                }
                else{
                    fprintf(stderr, "lab 4C usage: bad argument for --scale: %s\n", usageMessage);
                    exit(ARGS_ERROR);  
                }
                break;
            case 'l':
                if(strlen(optarg) == 0){
                    fprintf(stderr, "lab 4C usage: bad argument for --log: %s\n", usageMessage);
                    exit(ARGS_ERROR);   
                }
                logFilePath = optarg;
                if(DEBUG){
                    fprintf(stderr, "DEBUG: log file path: %s\n", logFilePath);
                }
                break;
            case 'i':
                if(!isNineDigitNum(optarg)){
                    fprintf(stderr, "lab 4C usage: id must be 9 digit number: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                clientID = optarg;
                if(DEBUG){
                    fprintf(stderr, "DEBUG: client ID: %s\n", clientID);
                }
                break;
            case 'h':
                if(strlen(optarg) == 0){
                    fprintf(stderr, "lab 4C usage: bad argument for --host: %s\n", usageMessage);
                    exit(ARGS_ERROR);   
                }
                hostName = optarg;
                if(DEBUG){
                    fprintf(stderr, "DEBUG: host name: %s\n", hostName);
                }
                break;
            case '?':
                if(optopt == 'p' || optopt == 's' || optopt == 'l'){
                    fprintf(stderr, "lab 4C usage: %s missing mandatory argument: %s\n", argv[optind-1], usageMessage);
                    exit(ARGS_ERROR);
                }
                fprintf(stderr, "lab 4C usage: unrecognized argument %s: usage: %s\n", argv[optind-1], usageMessage);
                exit(ARGS_ERROR);
                break;
            default:
                fprintf(stderr, "lab 4C usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
        }
    }
    if(optind >= argc){
        fprintf(stderr, "lab 4C usage: missing port argument, usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    portNumber = atoi(argv[optind]);
    optind++;

    if(optind<argc){
        fprintf(stderr, "lab 4C usage: extraneous arguments found, usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    if(logFilePath == NULL || clientID == NULL || hostName == NULL){
        fprintf(stderr, "1 lab 4C usage: port, --log, --id, and --host parameters are mandatory: usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    if(samplingFreq <= 0){
        fprintf(stderr, "lab 4C: sampling frequency must be at least 1\n");
        exit(ARGS_ERROR);
    }

    logFileFD = creat(logFilePath, CREATE_PERMISSIONS);
    if(logFileFD<0){
        fprintf(stderr, "lab 4C: --log: error: %s creating/opening file at path %s\n", strerror(errno), logFilePath);
        exit(OTHER_ERROR);
    }

    if(DEBUG){
        fprintf(stderr, "DEBUG: Sampling freq: %d, scale: %d\n", samplingFreq, degreeScale);
        fprintf(stderr, "Client ID: %s; log file path: %s; host name: %s\n", clientID, logFilePath, hostName);
    }

    //create TCP connection
    createConnection(portNumber);

    //now establish SSL connection   
    createSSLConnection();

    atexit(cleanupAndExit);


    if(close(STDIN_FILENO)<0){
        fprintf(stderr, "lab 4C: error in closing file descriptor: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR); 
    }
    // if(close(STDOUT_FILENO)<0){
    //     fprintf(stderr, "lab 4C: error in closing file descriptor: %s\n", strerror(errno));
    //     exit(SYS_CALL_ERROR); 
    // }

    if(dup(sockfd)<0){ //duplicate sockfd to 0 for STDIN
        fprintf(stderr, "lab 4C: error in duplicating file descriptor: %s\n", strerror(errno)); 
        exit(SYS_CALL_ERROR);
    }

    // if(dup(sockfd)<0){ //duplicate sockfd to 1 for STDOUT
    //     fprintf(stderr, "lab 4C: error in duplicating file descriptor: %s\n", strerror(errno)); 
    //     exit(SYS_CALL_ERROR);
    // }

    // if(close(sockfd)<0){ //close the original one
    //     fprintf(stderr, "lab 4C: error in closing file descriptor: %s\n", strerror(errno));
    //     exit(SYS_CALL_ERROR); 
    // }

    char intialMessage[16] = {};
    int amtWritten = sprintf(intialMessage, "ID=%s\n", clientID);
    if(amtWritten < 0){
        fprintf(stderr, "lab 4C: Failed to send ID to host server: %s\n", strerror(errno));
        exit(OTHER_ERROR);
    }
    if(SSL_write(sslObject, intialMessage, amtWritten)<0){
        fprintf(stderr, "lab 4C: 3 Failed to write ID messeage to server\n");
        exit(SYS_CALL_ERROR);
    }

    //initialize the temperature sensor
    tempSensor = mraa_aio_init(AIO_01_PIN_VALUE);
    if(tempSensor == NULL){
        fprintf(stderr, "Failed to initialize temperature sensor (AIO device)\n");
        mraa_deinit();
        exit(OTHER_ERROR);
    }
    if(DEBUG){
        fprintf(stderr, "DEBUG: Initialized temperature sensor.\n");
    }    

    struct pollfd pollEvents[] = {
        {STDIN_FILENO, POLLIN | POLLHUP | POLLERR, 0},
    };

    if(gettimeofday(&currentTv, NULL) < 0){
        fprintf(stderr, "Failed to get current time: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    readyToReadTv.tv_sec = 0; //so that we always get first reading

    int tempSensorReading;
    float R;
    float temperature;
    int index = 0;
    //main event loop
    while(1){
        if(gettimeofday(&currentTv, NULL)<0){
            fprintf(stderr, "Failed to get current time: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }
        timeStampTime = localtime(&currentTv.tv_sec);
            if(timeStampTime == NULL){
                fprintf(stderr, "Failed to get current date and time\n");
                exit(OTHER_ERROR);
        }

        if(shouldRead && currentTv.tv_sec >= readyToReadTv.tv_sec){
            //the appropriate period has passed
            tempSensorReading = mraa_aio_read(tempSensor);
            if(tempSensorReading<0){
            	fprintf(stderr, "Error reading the temperature\n");
            	exit(OTHER_ERROR);
            }
            R = R0*(1023.0/((float)tempSensorReading) - 1.0);
            temperature = 1.0/(log(R/R0)/BETA + 1/T0) - K_TO_C;
            if(degreeScale == F_SCALE){
                temperature = temperature*(9.0/5.0)+32;
            }
            char outputBuffer[BUFFER_SIZE]= {0};
            int amtWritten = sprintf(outputBuffer, "%02d:%02d:%02d %.1f\n", 
                timeStampTime->tm_hour, timeStampTime->tm_min, 
                timeStampTime->tm_sec, temperature);
            if(SSL_write(sslObject, outputBuffer, amtWritten)<0){
                fprintf(stderr, "lab 4C: 4 Failed to TLS write to stdout\n");
                exit(SYS_CALL_ERROR);
            }
            if(logFilePath != NULL){
                if(write(logFileFD, outputBuffer, amtWritten)<0){
                    fprintf(stderr, "lab 4C: 5 Failed to write to log: %s\n", strerror(errno));
                    exit(SYS_CALL_ERROR);
                }
            }
            
            readyToReadTv = currentTv;
            readyToReadTv.tv_sec += samplingFreq;
        }
        int pollResult = poll(pollEvents, 1, TIMEOUT);
        if(pollResult < 0){
            fprintf(stderr, "lab 4C: error polling: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }

        if(pollEvents[0].revents & POLLIN){
            int fdInAmount = SSL_read(sslObject, stdinBuffer, BUFFER_SIZE*sizeof(char));
            if(fdInAmount < 0){
                fprintf(stderr, "lab 4C: error reading in TLS text\n");
                exit(SYS_CALL_ERROR);
            }
            // newline is terminator here

            int i = 0;
            for(; i<fdInAmount; i++){
                if(stdinBuffer[i] == '\n'){
                    completeArg[index] = '\0'; //map \n to \0
                    processArgument(completeArg);
                    index = 0;
                }
                else{
                    completeArg[index] = stdinBuffer[i];
                    index++;
                }
            }
        }
    }
    
    exit(NORMAL_EXIT);
}
