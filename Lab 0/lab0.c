#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define ARGS_ERROR 1
#define INPUT_ERROR 2
#define OUTPUT_ERROR 3
#define SEG_FAULT_ERROR 4
#define CREATE_PERMISSIONS 0666

void segFaultFound(int sig_num){
    fprintf(stderr, "Segmentation fault initiated and caught; signal number: %d\n", sig_num);
    exit(SEG_FAULT_ERROR);
}

void causeSegFault(){
    int* badPointer = NULL;
    *badPointer = 2020;
}

int main(int argc, char** argv){
    // printf("Number of arguments: %d\n", argc);
    //arguments: --input=filename, --output=filename, --segfault, --catch
    int c;
    int segFaultTrue = 0;
    int shouldCatch = 0;
    char* inputFilePath = NULL;
    char* outputFilePath = NULL;
    int ifd = STDIN_FILENO;
    int ofd = STDOUT_FILENO;
    char* usageMessage = "[--input=filename] [--output=filename] [--segfault] [--catch]";
    while(1){
        static struct option long_options[] = {
            {"input", required_argument, 0, 'i'},
            {"output", required_argument, 0, 'o'},
            {"segfault", no_argument, 0, 's'},
            {"catch", no_argument, 0, 'c'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; //suppress getopt_long error messages
        c = getopt_long(argc, argv, "i:o:sc", long_options, &option_index);
        if(c==-1){
            break;
        }
        switch(c){
            case 'i':
// printf("--input argument with arg %s\n", optarg);
                inputFilePath = optarg;
                break;
            case 'o':
// printf("--output with arg %s\n", optarg);
                outputFilePath = optarg;
                break;
            case 's':
                // printf("--segfault, will cause segfault\n");
                segFaultTrue = 1;
                break;
            case 'c':
                // printf("--catch: will catch segfault\n");
                shouldCatch = 1;
                break;
            case '?':
                if(optopt == 'i' || optopt == 'o'){
 					fprintf(stderr, "lab0 usage: --input has a mandatory file path argument\n");
                	exit(ARGS_ERROR);
                }
                else if(optopt == 'o'){
                	fprintf(stderr, "lab0 usage: --output has a mandatory file path argument\n");
                	exit(ARGS_ERROR);
                }

                // printf("Error found earlier\n");
                fprintf(stderr, "lab0: unrecognized argument %s: usage: %s\n", argv[optind-1], usageMessage);
                exit(ARGS_ERROR);
            case ':':
// printf("Missing argument!\n");
            	fprintf(stderr, "lab0 usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
            default:
                fprintf(stderr, "lab0 usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
        }
    }
    if(optind<argc){
        fprintf(stderr, "lab0: extraneous arguments found: usage: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }

    // File redirection
    if(inputFilePath != NULL){
        ifd = open(inputFilePath, O_RDONLY);
        // printf("ifd: %d\n", ifd);
        if(ifd >=0){
            close(0);
            dup(ifd);
            close(ifd);
        }
        else{
            fprintf(stderr, "lab 0: --input: error: %s involving file at path %s\n", strerror(errno), inputFilePath);
            exit(INPUT_ERROR);
        }
        // printf("input file path: %s\n", inputFilePath);
    }
    if(outputFilePath != NULL){
        ofd = creat(outputFilePath, CREATE_PERMISSIONS);
// printf("ofd: %d\n", ofd);
        if(ofd >= 0){
            close(1);
            dup(ofd);
            close(ofd);
        }
        else{
            fprintf(stderr, "lab 0: --output: error: %s involving file at path %s\n", strerror(errno), outputFilePath);
            exit(OUTPUT_ERROR);
        }
        // printf("output file path: %s\n", outputFilePath);
    }

    // Register signal handler
    if(shouldCatch){
        signal(SIGSEGV, segFaultFound); 
    }

    // Cause segfault if specified
    if(segFaultTrue){
        causeSegFault();
    }

    //Copy from fd0 to fd1
    unsigned char buffer[1];
    int errorNumber;
    ssize_t readStatus;
    ssize_t writeStatus;
    while(1){
        readStatus = read(0, buffer, sizeof(unsigned char));
        if(readStatus < 0){
            errorNumber = errno;
            fprintf(stderr, "lab0: Error occurred while reading in: %s\n", strerror(errno));
            exit(errorNumber);
        }
        else if(readStatus == 0){
            // printf("Done reading\n");
            exit(0);
        }
        else{
            writeStatus = write(1, buffer, sizeof(unsigned char));
            if(writeStatus < 0){
                errorNumber = errno;
                fprintf(stderr, "lab0: Error occurred while writing out: %s\n", strerror(errno));
                exit(errorNumber);
            }
        }
    }
    // printf("PROGRAM RUN SUCCESSFULLY");
    exit(0);
}
