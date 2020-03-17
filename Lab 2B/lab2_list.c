// Danning Yu, 305087992
// CS 111 Lab 2B - Add --lists option

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
#include "SortedList.h"

#define NORMAL_EXIT 0
#define SYS_CALL_ERROR 1
#define ARGS_ERROR 1
#define OTHER_ERROR 2

#define NO_SYNC 0
#define MUTEX_PROT 1
#define SPIN_PROT 2

#define KEY_LENGTH 8

#define DEBUG 0 //toggle switch for debugging mode

#define NS_TO_S 1000000000

int opt_yield; //extern from SortedList.h
const char* lettersAndNumbers = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijlkmnopqrstuvwxyz";
const char* usageMessage = "[--threads=#] [--iterations=#] [--yield=[idl]] [--sync=[msc]] [--lists=#]";

struct subListHead {
    SortedList_t m_head;
    int m_syncOption;
    pthread_mutex_t m_lock;
    int m_spinLockValue;
};

typedef struct subListHead subListHead_t;

// Global option switches
static int numThreads = 1; //defaults
static int numIterations = 1;
static int syncOption = NO_SYNC;
static int numLists = 1;

//Pointers to dynamically allocated arrays
static subListHead_t* subListHeadArray = NULL;
static SortedListElement_t* slElemArray = NULL;
static long long* mutexWaitTimeArray = NULL;
static int* offsets = NULL;
static pthread_t* threadArray = NULL;
// static pthread_mutex_t lock;
// static int spinLockValue = 0;
// static SortedList_t sortList; //the "head"

//Other global vars
static int numKeysGenerated = 0;
static int alreadyJoined = 0;

void SortedList_print(SortedList_t* list){
    if(list==NULL){
        return;
    }
    SortedListElement_t* curr = list->next;
    while(curr != list){
        if(curr->key != NULL){
            fprintf(stderr, "%s->", curr->key);  
        }
        else{
            fprintf(stderr, "ERROR: null key!\n");
            return;
        }

        curr = curr->next;
    }
    fprintf(stderr, "\n");
    return;
}

void segFaultFound(int sig_num){
    fprintf(stderr, "lab 2B: corrupted list caused segmentation fault with signal number: %d\n", sig_num);
    exit(OTHER_ERROR);
}

void deleteFromLL(int baseOffset, int i, int listIndex){
    // i is the offset from the baseOffset
    // int arrayIndex = baseOffset/numIterations;
    SortedListElement_t* desiredElem = SortedList_lookup(&(subListHeadArray[listIndex].m_head), slElemArray[baseOffset+i].key);
    //we should be able to find all elements...?
    if(desiredElem == NULL){
        fprintf(stderr, "lab 2B: List got corrupted while searching\n");
        exit(OTHER_ERROR);
    }
    // if(DEBUG){
    //     fprintf(stderr, "Addresses: Element: %x, Key: %x\n", desiredElem, desiredElem->key);
    // }
    if(SortedList_delete(desiredElem) != 0){
        fprintf(stderr, "lab 2B: List got corrupted while deleting\n");
        exit(OTHER_ERROR);
    }
    // free(desiredElem->key);
}

int getListNumber(const char* key){
    int result = 0;
    for(int i = 0; i<KEY_LENGTH; i++){
        result += key[i];
    }
    result = result%numLists;
    if(DEBUG){
        printf("List number: %d\n", result);
    }
    return result;
}

void* carryOutLLOp(void* threadNumber){
    //each thread executes this function
    struct timespec lockStartTime;
    struct timespec lockEndTime;

    int baseOffset = *((int*)threadNumber);
    int arrayIndex = baseOffset/numIterations;

    for(int i = 0; i<numIterations; i++){
        int listIndex = getListNumber(slElemArray[baseOffset+i].key);
        SortedList_t* sortList = &(subListHeadArray[listIndex].m_head);
        int* spinLockValue = &(subListHeadArray[listIndex].m_spinLockValue);
        pthread_mutex_t* lock = &(subListHeadArray[listIndex].m_lock);
        switch(syncOption){
            case NO_SYNC: ;
                SortedList_insert(sortList, &slElemArray[baseOffset+i]);
                break;
            case MUTEX_PROT: ;          
                clock_gettime(CLOCK_MONOTONIC, &lockStartTime);
                pthread_mutex_lock(lock); //lock
                clock_gettime(CLOCK_MONOTONIC, &lockEndTime);
                mutexWaitTimeArray[arrayIndex] += (lockEndTime.tv_sec - lockStartTime.tv_sec)*NS_TO_S + (lockEndTime.tv_nsec - lockStartTime.tv_nsec);
                SortedList_insert(sortList, &slElemArray[baseOffset+i]);
                pthread_mutex_unlock(lock); //and unlock
                break;
            case SPIN_PROT: ;
                while(__sync_lock_test_and_set(spinLockValue, 1)){
                    ;
                }
                SortedList_insert(sortList, &slElemArray[baseOffset+i]);
                __sync_lock_release(spinLockValue);
                break;
            default:
                break;             
        }
    }

    // get length
    int length = 0;
    for(int i = 0; i<numLists; i++){
        SortedList_t* sortList = &(subListHeadArray[i].m_head);
        int* spinLockValue = &(subListHeadArray[i].m_spinLockValue);
        pthread_mutex_t* lock = &(subListHeadArray[i].m_lock);   
        switch(syncOption){        
        case NO_SYNC: ;
            length = SortedList_length(sortList);
            if(length<0){
                fprintf(stderr, "lab 2B: List got corrupted while getting its length\n");
                exit(OTHER_ERROR);
            }          
            break;
        case MUTEX_PROT: ;
            clock_gettime(CLOCK_MONOTONIC, &lockStartTime);
            pthread_mutex_lock(lock); //lock...
            clock_gettime(CLOCK_MONOTONIC, &lockEndTime);
            mutexWaitTimeArray[arrayIndex] += (lockEndTime.tv_sec - lockStartTime.tv_sec)*NS_TO_S + (lockEndTime.tv_nsec - lockStartTime.tv_nsec);
            length = SortedList_length(sortList);
            if(length<0){
                fprintf(stderr, "lab 2B: List got corrupted while getting its length\n");
                exit(OTHER_ERROR);
            }
            pthread_mutex_unlock(lock); //...and unlock    
            break;
        case SPIN_PROT: ;
            while(__sync_lock_test_and_set(spinLockValue, 1)){
                    ;
            }
            length = SortedList_length(sortList);
            if(length<0){
                fprintf(stderr, "lab 2B: List got corrupted while getting its length\n");
                exit(OTHER_ERROR);
            }
            __sync_lock_release(spinLockValue);
            break;
        default:
            break; 
        }
        if(DEBUG){
            printf("List length: %d\n", length);
        }
    }

    //delete
    for(int i = 0; i<numIterations; i++){
        int listIndex = getListNumber(slElemArray[baseOffset+i].key);
        // SortedList_t* sortList = &(subListHeadArray[listIndex].m_head);
        int* spinLockValue = &(subListHeadArray[listIndex].m_spinLockValue);
        pthread_mutex_t* lock = &(subListHeadArray[listIndex].m_lock);
        switch(syncOption){
            case NO_SYNC: ;
                deleteFromLL(baseOffset, i, listIndex);
                break;
            case MUTEX_PROT: ;
                clock_gettime(CLOCK_MONOTONIC, &lockStartTime);
                pthread_mutex_lock(lock); //lock
                clock_gettime(CLOCK_MONOTONIC, &lockEndTime);
                mutexWaitTimeArray[arrayIndex] += (lockEndTime.tv_sec - lockStartTime.tv_sec)*NS_TO_S + (lockEndTime.tv_nsec - lockStartTime.tv_nsec);
                deleteFromLL(baseOffset, i, listIndex);
                pthread_mutex_unlock(lock); //and unlock
                break;
            case SPIN_PROT: ;
                while(__sync_lock_test_and_set(spinLockValue, 1)){
                    ;
                }               
                deleteFromLL(baseOffset, i, listIndex);
                __sync_lock_release(spinLockValue);
                break;
            default:
                break;
        }
        
    }
    if(DEBUG){
        printf("Reached end of LLOp\n");
    }


    return NULL;
}

void freeAndExit(){
    // if(DEBUG){
    //     fprintf(stderr, "Cleaning up\n");
    //     SortedList_print(sortList);   
    // }

    for(int i = 0; i<numKeysGenerated; i++){
        free((char*)(slElemArray[i].key));
    }

    for(int i = 0; syncOption == MUTEX_PROT && i<numLists; i++){
        pthread_mutex_destroy(&(subListHeadArray[i].m_lock));
    }

    free(slElemArray);
    free(offsets);
    free(threadArray);
    free(mutexWaitTimeArray);
    free(subListHeadArray);
}

int main(int argc, char** argv){
    opt_yield = 0;
    int c;
    while(1){
        static struct option long_options[] = {
            {"threads", required_argument, 0, 't'},
            {"iterations", required_argument, 0, 'i'},
            {"yield", required_argument, 0, 'y'},
            {"sync", required_argument, 0, 's'},
            {"lists", required_argument, 0, 'l'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        opterr = 0; //suppress getopt_long error messages
        c = getopt_long(argc, argv, "t:i:y:s:", long_options, &option_index);
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
            case 'l':
                numLists = atoi(optarg);
                break;
            case 'y':
                if(strlen(optarg)>3){
                    fprintf(stderr, "lab 2B usage: bad argument for --yield: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                for(size_t i = 0; i<strlen(optarg); i++){
                    if(optarg[i] == 'i'){
                       opt_yield |= INSERT_YIELD; 
                    }
                    else if(optarg[i] == 'd'){
                        opt_yield |= DELETE_YIELD;
                    }
                    else if(optarg[i]=='l'){
                        opt_yield |= LOOKUP_YIELD;
                    }
                    else{
                        fprintf(stderr, "lab 2B usage: bad argument for --yield: %s\n", usageMessage);
                        exit(ARGS_ERROR);
                    }
                }
                break;
            case 's':
                if(optarg[0] == 'm'){
                    syncOption = MUTEX_PROT;
                }
                else if(optarg[0] == 's'){
                    if(DEBUG){
                        fprintf(stderr, "Spin protection\n");
                    }
                    syncOption = SPIN_PROT;
                }
                else{
                    fprintf(stderr, "lab 2B usage: bad argument for --sync: %s\n", usageMessage);
                    exit(ARGS_ERROR);
                }
                break;
            case '?':
                if(optopt == 't' || optopt == 'i' || optopt == 's'){
                     fprintf(stderr, "lab 2B usage: %s missing mandatory argument: %s\n", argv[optind-1], usageMessage);
                    exit(ARGS_ERROR);
                }
                fprintf(stderr, "lab 2B usage: unrecognized argument %s: usage: %s\n", argv[optind-1], usageMessage);
                exit(ARGS_ERROR);
                break;
            default:
                fprintf(stderr, "lab 2B usage: %s\n", usageMessage);
                exit(ARGS_ERROR);
        }
    }

    if(optind<argc){
        fprintf(stderr, "lab 2B usage: extraneous arguments found: %s\n", usageMessage);
        exit(ARGS_ERROR);
    }
    if(numThreads <= 0){
        fprintf(stderr, "lab 2B: number of threads must be at least 1\n");
        exit(ARGS_ERROR);
    }
    if(numIterations<=0){
        fprintf(stderr, "lab 2B: number of iterations must be at least 1\n");
        exit(ARGS_ERROR); 
    }
    if(numLists<=0){
        fprintf(stderr, "lab 2B: number of lists must be at least 1\n");
        exit(ARGS_ERROR); 
    }

    signal(SIGSEGV, segFaultFound);

    subListHeadArray = (subListHead_t*)malloc(numLists*sizeof(subListHead_t));
    if(subListHeadArray == NULL){
        fprintf(stderr, "lab 2B: List element memory allocation failed\n");
        exit(OTHER_ERROR);
    }

    for(int i = 0; i<numLists; i++){
        SortedList_t* headNode = &(subListHeadArray[i].m_head);
        headNode->key = NULL;
        headNode->next = headNode;
        headNode->prev = headNode;
        subListHeadArray[i].m_syncOption = syncOption;
        if(syncOption == MUTEX_PROT){
            int mutexInitRes = pthread_mutex_init(&(subListHeadArray[i].m_lock), NULL);
            if(mutexInitRes != 0){
                fprintf(stderr, "lab 2B: Mutex creation failed: %s\n", strerror(errno));
                exit(SYS_CALL_ERROR);
            }
        }
        else if(syncOption == SPIN_PROT){
            subListHeadArray[i].m_spinLockValue = 0;	
        }
    }

    atexit(freeAndExit);

    if(syncOption == MUTEX_PROT){
        mutexWaitTimeArray = (long long*)malloc(numThreads*sizeof(long long));
        if(mutexWaitTimeArray == NULL){
            fprintf(stderr, "lab 2B: Mutex wait time array memory allocation failed\n");
            exit(OTHER_ERROR);
        }
    }

    slElemArray = (SortedListElement_t*)malloc(numIterations*numThreads*sizeof(SortedListElement_t));
    if(slElemArray == NULL){
        fprintf(stderr, "lab 2B: List element memory allocation failed\n");
        exit(OTHER_ERROR);
    }

    srand(time(0));
    for(int i = 0; i<numIterations*numThreads; i++){
        char* randomString = (char*)malloc((KEY_LENGTH+1)*sizeof(char));
        // printf("%x\n", randomString);
        if(randomString == NULL){
            fprintf(stderr, "lab 2B: Random key memory allocation failed\n");
            exit(OTHER_ERROR);            
        }
        numKeysGenerated++;
        for(int i = 0; i<KEY_LENGTH; i++){
            randomString[i] = lettersAndNumbers[rand()%62];

        }
        randomString[KEY_LENGTH] = '\0'; //terminate with nullbyte
        slElemArray[i].key = randomString;
    }

    offsets = (int*)malloc(numThreads*sizeof(int));
    for(int i = 0, val = 0; i<numThreads; i++, val += numIterations){
        offsets[i] = val;
    }

    struct timespec startTime;
    struct timespec endTime;

    if(clock_gettime(CLOCK_MONOTONIC, &startTime)<0){
        fprintf(stderr, "lab 2B: Error getting start time\n");
        exit(SYS_CALL_ERROR);
    }

    threadArray = (pthread_t*)malloc(numThreads*sizeof(pthread_t));
    if(threadArray == NULL){
        fprintf(stderr, "lab 2B: Thread memory allocation failed\n");
        exit(OTHER_ERROR);
    }

    // SortedList_print(sortList);
    for(int i = 0; i<numThreads; i++){
        int createRes = pthread_create(&threadArray[i], NULL, carryOutLLOp, (void*)(&offsets[i]));
        if(createRes != 0){
            fprintf(stderr, "lab 2B: Error creating threads: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }
    }

    for(int i = 0; i<numThreads; i++){
        int joinRes = pthread_join(threadArray[i], NULL);
        if(joinRes != 0){
            fprintf(stderr, "lab 2B: Error joining threads: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }

    }
    alreadyJoined = 1;

    if(clock_gettime(CLOCK_MONOTONIC, &endTime)<0){
        fprintf(stderr, "lab 2B: Error getting end time\n");
        exit(SYS_CALL_ERROR);
    }
    for(int i = 0; i<numLists; i++){
        if(SortedList_length(&(subListHeadArray[i].m_head)) != 0){
            fprintf(stderr, "lab 2B: List is not empty!\n");
            exit(OTHER_ERROR);
        }
    }

    long long runTimeNS = (endTime.tv_sec - startTime.tv_sec)*NS_TO_S + (endTime.tv_nsec - startTime.tv_nsec);
    char result[100] = "list-";
    long long numOps = 3*numThreads*numIterations;
    char* selectedYieldOpt = "none";
    switch(opt_yield){
        case INSERT_YIELD:
            selectedYieldOpt = "i";
            break;
        case DELETE_YIELD:
            selectedYieldOpt = "d";
            break;
        case LOOKUP_YIELD:
            selectedYieldOpt = "l";
            break;
        case INSERT_YIELD|DELETE_YIELD:
            selectedYieldOpt = "id";
            break;
        case INSERT_YIELD|LOOKUP_YIELD:
            selectedYieldOpt = "il";
            break;
        case DELETE_YIELD|LOOKUP_YIELD:
            selectedYieldOpt = "dl";
            break;
        case INSERT_YIELD|DELETE_YIELD|LOOKUP_YIELD:
            selectedYieldOpt = "idl";
            break;
        default:
            break;
    }
    strcat(result, selectedYieldOpt);
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
        default:
            break;
    }
    long long timePerOp = runTimeNS/numOps;
    long long totalMutexTime = 0;
    for(int i = 0; mutexWaitTimeArray != NULL && i<numThreads; i++){
        totalMutexTime += mutexWaitTimeArray[i];
    }
    fprintf(stdout, "%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", result, numThreads, numIterations, numLists, numOps,runTimeNS,timePerOp,totalMutexTime/numOps);
    exit(NORMAL_EXIT);
}
