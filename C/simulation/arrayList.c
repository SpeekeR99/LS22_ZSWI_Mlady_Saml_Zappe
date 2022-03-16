/**
 * This module contains functions to work with arrayList struct. ArrayList
 * stores just pointers to items which it stores.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "arrayList.h"

/**
 * Creates new arrayList with size listSize for items with ItemSize.
 *  Returns pointer to arrayList or NULL if it was not successful.
 * @param listSize maximal number of items in the list, must be greater than 0
 * @param itemSize size of one element in the arrayList, must be greater than 0
 * @return pointer to new arrayList or NULL some of the parameters was <= 0 or
 *         it was not possible to allocate memory
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

/**
 * Adds new item to the arrayList, if arrayList is full, then it expands it by
 * INCREMENT number of items (can be changed with INCREMENT macro in arrayList.h)
 * @param list not null pointer to list
 * @param pointer not null pointer to element which should be added into the list
 * @return SUCCESS (1) or FAILURE (0), FAILURE can occur when @param pointer or @param list
 *         is null
 */
int arrayListAdd(arrayList *list, void *pointer) {
    if (!list || !pointer) return FAILURE;

    if (list->listSize == list->filledItems) arrayListExpand(list, INCREMENT);

    list->data[list->filledItems] = pointer;
    list->filledItems++;
    return SUCCESS;
}

/**
 * Increases size of arrayList by increment parameter. Then nulls content of the part
 * which was added.
 * @param list non null pointer to arrayList
 * @param increment must be greater than zero
 * @return SUCCESS (1) or FAILURE (0), failure can occur when list is NULL or it
 *         is not possible to allocate more memory
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

/**
 * Returns pointer to the item at specified index of the arrayList. If that
 *  position is free, returns NULL.
 * @param list not null pointer to arrayList
 * @param index integer between 0 and (list->filledItems - 1)
 * @return pointer to specified element if parameters are valid, else returns NULL
 */
void *arrayListGetPointer(arrayList *list, int index) {
    if (!list || index < 0 || list->filledItems <= index) return NULL;

    return list->data[index];
}


/**
 * Returns pointer to an element on @param index and removes this pointer
 * from the list.
 * @param list not null pointer to arrayList
 * @param index integer between 0 and (list->filledItems - 1)
 * @return pointer to the element which was removed or NULL if function
 *         parameters were wrong
 */
void *arrayListRemoveElement(arrayList *list, int index) {
    void *pointer;
    int i;

    if (!list || index < 0 || list->filledItems <= index) return NULL;

    pointer = list->data[index];

    //move all the elements (which were to the right from the selected one)
    // one step to the left
    for (i = index; i < list->filledItems - 1; i++) {
        list->data[i] = list->data[i + 1];
    }

    list->data[list->filledItems - 1] = NULL;
    list->filledItems--;

    return pointer;
}

/**
 * Deallocates memory used by arrayList, memory leaks can occur if elements of
 *  arrayList use some allocating of the memory, then you should use your own
 *  free function instead of this one.
 * @param list pointer to pointer to list
 */
void freeArrayList(arrayList **list) {
    int i;
    if (!list || !*list) return;

    // deallocate memory used by elements in the arrayList
    for (i = 0; i < (*list)->filledItems; i++) {
        if ((*list)->data[i]) free((*list)->data[i]);
    }

    free((*list)->data);
    free(*list);
    *list = NULL;
}

