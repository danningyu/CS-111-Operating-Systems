// Danning Yu
// 305087992
// CS 111 Lab 4B

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <math.h>
#include <mraa.h>

#define F_SCALE 0
#define C_SCALE 1

#define DEBUG 0

#define AIO_01_PIN_VALUE 1 //for the temperature sensor
#define GPIO_50_PIN_VALUE 60 //for the buttonSensor
#define BUFFER_SIZE 1024

#define NORMAL_EXIT 0
#define SYS_CALL_ERROR 1
#define ARGS_ERROR 1
#define OTHER_ERROR 2
#define CREATE_PERMISSIONS 0666
#define TIMEOUT 200

// Temperature constants
const int BETA = 4275;
const float R0 = 100000.0;
const float T0 = 298.15;
const float K_TO_C = 273.15;

const char* usageMessage = "[--period=#] [--scale=[CF]] [--log=file_path]";

//global context variables
int degreeScale = F_SCALE; //default is farenheit
int samplingFreq = 1; //default, 1 data point per second
char* logFilePath = NULL; //filepath for logging
int logFileFD = -1;
mraa_result_t status;
mraa_aio_context tempSensor;
mraa_gpio_context buttonSensor;
char stdinBuffer[BUFFER_SIZE] = {0};
char logFileBuffer[BUFFER_SIZE] = {0};
char completeArg[BUFFER_SIZE] = {0};
struct timeval currentTv;
struct timeval readyToReadTv;
struct tm* timeStampTime;
int shouldRead = 1;

//CHECK WHERE ALL FPRINTFS ARE PRINTING TOO!!!
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
    fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec);
    if(logFilePath != NULL){
        int amtWritten = sprintf(logFileBuffer, "%02d:%02d:%02d SHUTDOWN\n", 
          timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec);
        if(write(logFileFD, logFileBuffer, amtWritten)<0){
            fprintf(stderr, "lab 4B: Failed to write to log: %s\n", strerror(errno));
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
            fprintf(stderr, "lab 4B: Failed to write to log: %s\n", strerror(errno));
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
        fprintf(stderr, "lab 4B: sampling frequency must be at least 1\n");
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

void cleanupAndExit(){
    if(DEBUG){
        fprintf(stderr, "DEBUG: EXITING!\n");
    }
    status = mraa_gpio_close(buttonSensor);
    if(status != MRAA_SUCCESS){
        exit(OTHER_ERROR);
    }

    status = mraa_aio_close(tempSensor);
    if(status != MRAA_SUCCESS){
        exit(OTHER_ERROR);
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
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; //suppress getopt_long error messages, use my own msgs instead
        c = getopt_long(argc, argv, "p:s:l:", long_options, &option_index);
        if(c==-1){
            break;
        }
        switch(c){
            case 'p':
                samplingFreq = atoi(optarg);
                break;
            case 's':
                if(strlen(optarg)>1){
                    fprintf(stderr, "lab 4B usage: bad argument for --scale: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                if(optarg[0] == 'F'){
                    degreeScale = F_SCALE;
                }
                else if(optarg[0] == 'C'){
                    degreeScale = C_SCALE;
                }
                else{
                    fprintf(stderr, "lab 4B usage: bad argument for --scale: %s\n", usageMessage);
                    exit(ARGS_ERROR);  
                }
                break;
            case 'l':
                if(strlen(optarg) == 0){
                    fprintf(stderr, "lab 4B usage: bad argument for --log: %s\n", usageMessage);
                    exit(ARGS_ERROR);   
                }
                logFilePath = optarg;
                if(DEBUG){
                    fprintf(stderr, "DEBUG: log file path: %s\n", logFilePath);
                }
                break;
            case '?':
                if(optopt == 'p' || optopt == 's' || optopt == 'l'){
                    fprintf(stderr, "lab 4B usage: %s missing mandatory argument: %s\n", argv[optind-1], usageMessage);
                    exit(ARGS_ERROR);
                }
                fprintf(stderr, "lab 4B usage: unrecognized argument %s: usage: %s\n", argv[optind-1], usageMessage);
                exit(ARGS_ERROR);
                break;
            default:
                fprintf(stderr, "lab 4B usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
        }
    }

    if(optind<argc){
        fprintf(stderr, "lab 4B usage: extraneous arguments found, usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    if(samplingFreq <= 0){
        fprintf(stderr, "lab 4B: sampling frequency must be at least 1\n");
        exit(ARGS_ERROR);
    }

    if(logFilePath != NULL){
        logFileFD = creat(logFilePath, CREATE_PERMISSIONS);
        if(logFileFD<0){
            fprintf(stderr, "lab 4B: --log: error: %s creating/opening file at path %s\n", strerror(errno), logFilePath);
            exit(OTHER_ERROR);
        }
    }

    if(DEBUG){
        fprintf(stderr, "DEBUG: Sampling freq: %d, scale: %d\n", samplingFreq, degreeScale);
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

    //initialize buttonSensor
    buttonSensor = mraa_gpio_init(GPIO_50_PIN_VALUE);
    if(buttonSensor == NULL){
        fprintf(stderr, "Failed to initialize temperature sensor (GPIO device)\n");
        mraa_deinit();
        exit(OTHER_ERROR);
    }
    status = mraa_gpio_dir(buttonSensor, MRAA_GPIO_IN);
    if(status != MRAA_SUCCESS){
        fprintf(stderr, "Error with buttonSensor device, exiting\n");
        exit(OTHER_ERROR);
    }
    //trigger on rising edge
    status = mraa_gpio_isr(buttonSensor, MRAA_GPIO_EDGE_RISING, &shutdownHandler, NULL);
    if(status != MRAA_SUCCESS){
        fprintf(stderr, "Error with buttonSensor device, exiting\n");
        exit(OTHER_ERROR);
    }
    if(DEBUG){
        fprintf(stderr, "DEBUG: Initialized buttonSensor device.\n");
    }

    atexit(cleanupAndExit);

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
            fprintf(stdout, "%02d:%02d:%02d ", timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec);
            if(degreeScale == F_SCALE){
                temperature = temperature*(9.0/5.0)+32;
                fprintf(stdout, "%.1f\n", temperature);
                if(logFilePath != NULL){
                    int amtWritten = sprintf(logFileBuffer, "%02d:%02d:%02d %.1f\n", 
                        timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec, temperature);
                    if(write(logFileFD, logFileBuffer, amtWritten)<0){
                        fprintf(stderr, "lab 4B: Failed to write to log: %s\n", strerror(errno));
                        exit(SYS_CALL_ERROR);
                    }
                }
            }
            else{ // C scale
                fprintf(stdout, "%.1f\n", temperature);
                if(logFilePath != NULL){
                    int amtWritten = sprintf(logFileBuffer, "%02d:%02d:%02d %.1f\n", 
                        timeStampTime->tm_hour, timeStampTime->tm_min, timeStampTime->tm_sec, temperature);
                    if(write(logFileFD, logFileBuffer, amtWritten)<0){
                        fprintf(stderr, "lab 4B: Failed to write to log: %s\n", strerror(errno));
                        exit(SYS_CALL_ERROR);
                    }
                }
            }
            readyToReadTv = currentTv;
            readyToReadTv.tv_sec += samplingFreq;
        }
        int pollResult = poll(pollEvents, 1, TIMEOUT);
        if(pollResult < 0){
            fprintf(stderr, "lab 4B: error polling: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }

        if(pollEvents[0].revents & POLLIN){
            int fdInAmount = read(STDIN_FILENO, stdinBuffer, BUFFER_SIZE*sizeof(char));
            if(fdInAmount < 0){
                fprintf(stderr, "lab 4B: error reading in text: %s\n", strerror(errno));
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
        // if(pollEvents[0].revents & (POLLHUP | POLLERR)){
        //     fprintf(stderr, "Error reading in from stdin\n");
        //     exit(OTHER_ERROR);
        // }
    }
    
    exit(NORMAL_EXIT);
}
