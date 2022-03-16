/* ____________________________________________________________________________

    Module arrayList.c
    This module contains functions to work with arrayList struct. ArrayList
    stores just pointers to items which it stores. Some functions work with
    arrayList as if it was simple hashtable (That's to get rid of duplicate
    elements).
   ____________________________________________________________________________
*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "arrayList.h"

/* ____________________________________________________________________________

    arrayList *createArrayList(int listSize, int itemSize)

    Tries to create new arrayList with size listSize for items with ItemSize.
    Returns pointer to arrayList or NULL if it was not successful.
   ____________________________________________________________________________
*/
arrayList *createArrayList(int listSize, int itemSize) {
    arrayList *temp;

    if (listSize <= 0 || itemSize <= 0) return NULL;

    temp = malloc(sizeof(arrayList));
    if (!temp) return NULL;

    temp->itemSize = itemSize;
    temp->listSize = listSize;
    temp->filledItems = 0;
    temp->data = calloc(listSize, sizeof(void *));

    if (!temp->data) {
        free(temp);
        return NULL;
    }

    return temp;
}


/* ____________________________________________________________________________

    arrayListAdd(arrayList *list, void *pointer)

    Tries to add new item to the arrayList, if arrayList is full, then it
    expands it by 2 items (can be changed with INCREMENT macro in arrayList.h.
    Returns boolean value true/false if adding was/wasn't successful.
   ____________________________________________________________________________
*/
int arrayListAdd(arrayList *list, void *pointer) {
    if (!list || !pointer) return FAILURE;

    if (list->listSize == list->filledItems) arrayListExpand(list, INCREMENT);

    list->data[list->filledItems] = pointer;
    list->filledItems++;
    return SUCCESS;
}

/* ____________________________________________________________________________

    int arrayListExpand(arrayList *list, const int increment)

    Tries to increase size of arrayList by increment parameter. Then nulls
    content of the part which was added. Returns boolean value true/false if
    the operation was/wasn't successful.
   ____________________________________________________________________________
*/
int arrayListExpand(arrayList *list, const int increment) {
    void **temp;
    if (!list) return FAILURE;

    temp = realloc(list->data, (list->listSize + increment) * sizeof(void *));
    if (!temp) {
        perror("Out of memory error\n");
        return FAILURE;
    }
    list->data = temp;
    memset(&list->data[list->listSize], 0, increment * sizeof(void *));
    list->listSize += increment;
    return SUCCESS;
}


/* ____________________________________________________________________________

    void *arrayListGetPointer(arrayList *list, int index)

    Returns pointer to the item at specified index of the arrayList. If that
    position is free, returns NULL. Index must be smaller than number of items
    in the arrayList.
   ____________________________________________________________________________
*/
void *arrayListGetPointer(arrayList *list, int index) {
    if (!list || index < 0 || list->filledItems <= index) return NULL;

    return list->data[index];
}


void *arrayListRemoveElement(arrayList *list, int index) {
    void *pointer;
    int i;

    if (!list || index < 0 || list->filledItems <= index) return NULL;

    pointer = list->data[index];

    for (i = index; i < list->filledItems - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    list->data[list->filledItems - 1] = NULL;
    list->filledItems--;

    return pointer;
}


/* ____________________________________________________________________________

    void freeArrayList(arrayList **list)

    Deallocates memory used by arrayList, memory leaks can occur if elements of
    arrayList use some allocating of the memory, then you should use your own
    free function instead of this.
   ____________________________________________________________________________
*/
void freeArrayList(arrayList **list) {
    int i;
    if (!list || !*list) return;

    for (i = 0; i < (*list)->filledItems; i++) {
        if ((*list)->data[i]) free((*list)->data[i]);
    }

    free((*list)->data);
    free(*list);
    *list = NULL;
}

