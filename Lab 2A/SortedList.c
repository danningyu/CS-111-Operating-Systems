// Danning Yu, 305087992
// CS 111 Lab 2A
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "SortedList.h"

/**
 *  * SortedList_insert ... insert an element into a sorted list
 *   *
 *    *    The specified element will be inserted in to
 *     *    the specified list, which will be kept sorted
 *      *    in ascending order based on associated keys
 *       *
 *        * @param SortedList_t *list ... header for the list
 *         * @param SortedListElement_t *element ... element to be added to the list
 *          */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    if(list == NULL){
        return;
    }
    if(element == NULL){
        return;
    }
    if(list->key != NULL){ //you didn't provide the head
        return;
    }
    SortedListElement_t* previous = list;
    SortedListElement_t* curr = list->next;
    while(curr != list){
        if(strcmp(curr->key, element->key)>0){
            break; //found the place to insert at
        }
        // if(strcmp(curr->key, element->key) == 0){
        //     fprintf(stderr, "Duplicate element found\n");
        //     return; //no duplicate keys allowed
        // }
        previous = curr;
        curr = curr->next;
    }
    if(opt_yield & INSERT_YIELD){
        sched_yield();
    }

    element->next = previous->next;
    element->prev = previous;
    previous->next = element;
    element->next->prev = element;
}

/**
 *  * SortedList_delete ... remove an element from a sorted list
 *   *
 *    *    The specified element will be removed from whatever
 *     *    list it is currently in.
 *      *
 *       *    Before doing the deletion, we check to make sure that
 *        *    next->prev and prev->next both point to this node
 *         *
 *          * @param SortedListElement_t *element ... element to be removed
 *           *
 *            * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *             *
 *              */
int SortedList_delete( SortedListElement_t *element){
    if(element == NULL){
        return 1;
    }
    if(element->key == NULL){
        return 1;
    }
    if(element->next->prev != element || element->prev->next != element){
        return 1; //somehow got corrupted
    }

    if(opt_yield & DELETE_YIELD){
        sched_yield();
    }
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
}

/**
 *  * SortedList_lookup ... search sorted list for a key
 *   *
 *    *    The specified list will be searched for an
 *     *    element with the specified key.
 *      *
 *       * @param SortedList_t *list ... header for the list
 *        * @param const char * key ... the desired key
 *         *
 *          * @return pointer to matching element, or NULL if none is found
 *           */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
    if(key == NULL){ // check piazza about this
        return NULL;
    }
    if(list == NULL || list->key != NULL){
        return NULL;
    }
    SortedListElement_t* result = list->next;
    while(result != list){
        if(strcmp(result->key, key) == 0){
            //found it!
            return result;
        }
        if(opt_yield & LOOKUP_YIELD){
            sched_yield();
        }
        result = result->next;
    }
    return NULL;
}

/**
 *  * SortedList_length ... count elements in a sorted list
 *   *    While enumeratign list, it checks all prev/next pointers
 *    *
 *     * @param SortedList_t *list ... header for the list
 *      *
 *       * @return int number of elements in list (excluding head)
 *        *       -1 if the list is corrupted
 *         */
int SortedList_length(SortedList_t *list){
    if(list == NULL || list->key != NULL){
        return -1; //bad parameter
    }
    int count = 0;
    SortedListElement_t* curr = list->next;
    while(curr != list){
        if(curr->prev->next != curr || curr->next->prev != curr){
            return -1; //corruption detected
        }
        count++;
        if(opt_yield & LOOKUP_YIELD){
            sched_yield();
        }
        curr = curr->next;
    }
    return count;
}