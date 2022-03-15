/* ____________________________________________________________________________

    Module hashTable.c
    This module contains functions which work with hashTable struct made with
    arrayLists. It can be used only by graphNode or graphEdge structs, because
    they have similar structure. HashTable saves only pointers to structs.
   ____________________________________________________________________________
*/
#include <malloc.h>
#include <stdio.h>
#include "hashTable.h"

/* ____________________________________________________________________________

    hashTable *createHashtable(int size, int itemSize)

    Creates new hashTable with which has size same as parameter size and
    itemSize. HashTable is made with usage of arrayLists.
   ____________________________________________________________________________
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

/* ____________________________________________________________________________

    int hashTableAddElement(void *element, int index, hashTable *table)

    Adds the element without checking if the index is unique in the hashTable,
    this function is used when new edges are added into graph on index which
    means the source node of the edge
   ____________________________________________________________________________
*/
int hashTableAddElement(void *element, int id, hashTable *table) {
    if (!element || !table) return 0;
    int updatedIndex = ABS(id % table->size);
    arrayListAdd(table->array[updatedIndex], element);
    table->filledItems++;
    return 1;
}

void *hashTableRemoveElement(int arrayIndex, int elementIndex, hashTable *table) {
    if (!table || arrayIndex < 0 || elementIndex < 0 ||
    arrayIndex > table->size || table->array[arrayIndex]->filledItems < elementIndex) return NULL;

    void *pointer = arrayListRemoveElement(table->array[arrayIndex],elementIndex);
    if (!pointer) return NULL;

    table->filledItems--;
    return pointer;
}

/* ____________________________________________________________________________

    void freeHashTable(hashTable **table)

    Deallocates memory used by hashTable, arrayLists which it uses and elements
    saved in arrayLists.
   ____________________________________________________________________________
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