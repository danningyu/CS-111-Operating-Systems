// Danning Yu, 305087992
// CS 111 Lab 2A

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
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

#define NORMAL_EXIT 0
#define SYS_CALL_ERROR 1
#define ARGS_ERROR 1
#define OTHER_ERROR 2

#define NO_SYNC 0
#define MUTEX_PROT 1
#define SPIN_PROT 2
#define GCC_PROT 3

#define NS_TO_S 1000000000

const char* usageMessage = "[--threads=#] [--iterations=#] [--yield] [--sync=[msc]]";

static int numThreads = 1; //defaults
static int numIterations = 1; //defaults
static int opt_yield = 0; //defaults
static int syncOption = NO_SYNC; //defaults
static pthread_t* threadArray;
static long long counter = 0;
static pthread_mutex_t lock;
static int spinLockValue = 0;


void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    if (opt_yield){
        sched_yield();
    }
    *pointer = sum;
}

void* carryOutAdd(){
    for(int i = 0; i<numIterations; i++){
        switch(syncOption){
            case NO_SYNC:
                add(&counter, 1);
                break;
            case MUTEX_PROT:
                pthread_mutex_lock(&lock);
                add(&counter, 1);
                pthread_mutex_unlock(&lock);
                break;
            case SPIN_PROT:
                while(__sync_lock_test_and_set(&spinLockValue, 1)){
                    ;
                }
                add(&counter, 1);
                __sync_lock_release(&spinLockValue);
                break;
            case GCC_PROT:
                while(1){
                    long long prev = counter;
                    long long incremented = prev+1;
                    if(__sync_val_compare_and_swap(&counter, prev, incremented) == prev){
                        break;
                    }
                }
                break;
            default:
                break;
        }

    }
    for(int i = 0; i<numIterations; i++){
        switch(syncOption){
            case NO_SYNC:
                add(&counter, -1);
                break;
            case MUTEX_PROT:
                pthread_mutex_lock(&lock);
                add(&counter, -1);
                pthread_mutex_unlock(&lock);
                break;
            case SPIN_PROT:
                while(__sync_lock_test_and_set(&spinLockValue, 1)){
                    ;
                }
                add(&counter, -1);
                __sync_lock_release(&spinLockValue);
                break;
            case GCC_PROT:
                while(1){
                    long long prev = counter;
                    long long incremented = prev-1;
                    if(__sync_val_compare_and_swap(&counter, prev, incremented) == prev){
                        break;
                    }
                }
                break;
            default:
                break;
        }
    }
    return NULL;
}

void freeAndExit(){
    if(threadArray != NULL){
        free(threadArray);
    }
    if(syncOption == MUTEX_PROT){
        pthread_mutex_destroy(&lock);
    }
}

int main(int argc, char** argv){
    int c;
    while(1){
        static struct option long_options[] = {
            {"threads", required_argument, 0, 't'},
            {"iterations", required_argument, 0, 'i'},
            {"yield", no_argument, 0, 'y'},
            {"sync", required_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; //suppress getopt_long error messages
        c = getopt_long(argc, argv, "t:i:ys:", long_options, &option_index);
        if(c==-1){
            break;
        }
        switch(c){
            case 't':
                numThreads = atoi(optarg);
                break;
            case 'i':
                numIterations = atoi(optarg);
                break;
            case 'y':
                opt_yield = 1;
                break;
            case 's':
                if(strlen(optarg)>1){
                    fprintf(stderr, "lab 2A usage: bad argument for --sync: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                if(optarg[0] == 'm'){
                    syncOption = MUTEX_PROT;
                }
                else if(optarg[0] == 's'){
                    syncOption = SPIN_PROT;
                }
                else if(optarg[0] == 'c'){
                    syncOption = GCC_PROT;
                }
                else{
                    fprintf(stderr, "lab 2A usage: bad argument for --sync: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                break;
            case '?':
                if(optopt == 't' || optopt == 'i' || optopt == 's'){
                     fprintf(stderr, "lab 2A usage: %s missing mandatory argument: %s\n", argv[optind-1], usageMessage);
                    exit(ARGS_ERROR);
                }
                fprintf(stderr, "lab 2A usage: unrecognized argument %s: usage: %s\n", argv[optind-1], usageMessage);
                exit(ARGS_ERROR);
                break;
            default:
                fprintf(stderr, "lab 2A usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
        }
    }

    if(optind<argc){
        fprintf(stderr, "lab 2A usage: extraneous arguments found: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    if(numThreads <= 0){
        fprintf(stderr, "lab 2A: number of threads must be at least 1\n");
        exit(ARGS_ERROR);
    }
    if(numIterations<=0){
        fprintf(stderr, "lab 2A: number of iterations must be at least 1\n");
        exit(ARGS_ERROR); 
    }

    struct timespec startTime;
    struct timespec endTime;
    if(clock_gettime(CLOCK_MONOTONIC, &startTime)<0){
        fprintf(stderr, "lab 2A: Error getting start time\n");
        exit(SYS_CALL_ERROR);
    }

    if(syncOption == MUTEX_PROT){
        // fprintf(stderr, "Creating mutex\n");
        int mutexInitRes = pthread_mutex_init(&lock, NULL);
        if(mutexInitRes != 0){
            fprintf(stderr, "lab 2A: Mutex creation failed: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }
    }

    atexit(freeAndExit);

    threadArray = (pthread_t*)malloc(numThreads*sizeof(pthread_t));
    if(threadArray == NULL){
        fprintf(stderr, "lab 2A: Thread memory allocation failed\n");
        exit(OTHER_ERROR);
    }

    //create threads
    for(int i = 0; i<numThreads; i++){
        int createRes = pthread_create(&threadArray[i], NULL, carryOutAdd, NULL);
        if(createRes != 0){
            fprintf(stderr, "lab 2A: Error creating threads: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }
    }

    for(int i = 0; i<numThreads; i++){
        int joinRes = pthread_join(threadArray[i], NULL);
        if(joinRes != 0){
            fprintf(stderr, "lab 2A: Error joining threads: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }
    }

    if(clock_gettime(CLOCK_MONOTONIC, &endTime)<0){
        fprintf(stderr, "lab 2A: Error getting end time\n");
        exit(SYS_CALL_ERROR);
    }

    long long runTimeNS = (endTime.tv_sec - startTime.tv_sec)*NS_TO_S + (endTime.tv_nsec - startTime.tv_nsec);
    long long numOps = 2*numThreads*numIterations;
    long long timePerOp = runTimeNS/numOps;
    
    char result[100] = "add";
    if(opt_yield){
        strcat(result, "-yield");
    }
    switch(syncOption){
        case NO_SYNC:
            strcat(result, "-none");
            break;
        case MUTEX_PROT:
            strcat(result, "-m");
            break;
        case SPIN_PROT:
            strcat(result, "-s");
            break;
        case GCC_PROT:
            strcat(result, "-c");
            break;
        default:
            break;
    }
    fprintf(stdout, "%s,%d,%d,%lld,%lld,%lld,%lld\n", result, numThreads, numIterations, numOps,runTimeNS,timePerOp,counter);
    exit(NORMAL_EXIT);
}