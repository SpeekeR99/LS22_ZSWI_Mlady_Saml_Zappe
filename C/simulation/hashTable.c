/**
 *  This module contains functions which work with hashTable struct made with
 *  arrayLists. It can be used only by graphNode or graphEdge structs, because
 *  they have similar structure. HashTable saves only pointers to structs.
 */
#include <malloc.h>
#include <stdio.h>
#include "hashTable.h"

/**
 * Creates new hashTable with @param size for items with @param itemSize.
 *  Returns pointer to arrayList or NULL if it was not successful.
 * @param size number of arrayLists in the hashTable, must be greater than 0
 * @param itemSize size of one element in the hashTable, must be greater than 0
 * @return pointer to new hashTable or NULL some of the parameters was <= 0 or
 *         it was not possible to allocate memory
 */
hashTable *createHashTable(int size, int itemSize) {
    int i;
    hashTable *table = malloc(sizeof(hashTable));
    table->size = size;
    table->filledItems = 0;
    table->array = malloc(sizeof(arrayList *) * size);
    for (i = 0; i < table->size; i++) {
        table->array[i] = createArrayList(5, itemSize);
    }

    return table;
}

/**
 * Adds new element to the end of arrayList
 * @param element pointer to be added into hashTable
 * @param id index of element
 * @param table not null pointer to hashTable
 * @return 0 in case of failure (invalid parameters) or 1
 */
int hashTableAddElement(void *element, int id, hashTable *table) {
    if (!element || !table) return 0;
    int updatedIndex = ABS(id % table->size);
    arrayListAdd(table->array[updatedIndex], element);
    table->filledItems++;
    return 1;
}

/**
 * Returns pointer to chosen element and removes it from hashTable
 * @param arrayIndex must be in interval <0, table->size)
 * @param elementIndex must be in interval <0, table->array[arrayIndex]->filledItems)
 * @param table not null hashtable
 * @return pointer to removed element or NULL if pointer is null, or parameters are invalid
 */
void *hashTableRemoveElement(int arrayIndex, int elementIndex, hashTable *table) {
    if (!table || arrayIndex < 0 || elementIndex < 0 ||
    arrayIndex > table->size || table->array[arrayIndex]->filledItems < elementIndex) return NULL;

    void *pointer = arrayListRemoveElement(table->array[arrayIndex],elementIndex);
    if (!pointer) return NULL;

    table->filledItems--;
    return pointer;
}

/**
 *
 *  Deallocates memory used by hashTable, arrayLists which it uses and elements
 *  saved in arrayLists.
 */
void freeHashTable(hashTable **table) {
    int i;

    if (!table || !*table) return;

    for (i = 0; i < (*table)->size; i++) {
        if ((*table)->array[i]) freeArrayList(&(*table)->array[i]);
    }

    free((*table)->array);
    free(*table);
    *table = NULL;
}