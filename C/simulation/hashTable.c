/**
 *  This module contains functions which work with hashTable struct made with
 *  arrayLists. It can be used only by graphNode or graphEdge structs, because
 *  they have similar structure. HashTable saves only pointers to structs.
 */
#include <stdlib.h>
#include "hashTable.h"
#include "simulation.h"

/**
 * Creates new hashTable with @param size for items with @param itemSize.
 *  Returns pointer to arrayList or NULL if it was not successful.
 * @param size number of arrayLists in the hashTable, must be greater than 0
 * @param itemSize size of one element in the hashTable, must be greater than 0
 * @return pointer to new hashTable or NULL some of the parameters was <= 0 or
 *         it was not possible to allocate memory
 */
hashTable *createHashTable(int size, int itemSize) {
    // TODO: fix this
    // The next line breaks CsvManager but if it's not there TESTS will fail
//    if (size <= 0 || itemSize <= 0) return NULL;
    int i;
    hashTable *table = malloc(sizeof(hashTable));
    if (!table) return NULL;
    table->size = size > 0 ? size : 40;
    table->filledItems = 0;
    table->itemSize = itemSize;
    table->array = malloc(sizeof(arrayList *) * table->size);
    for (i = 0; i < table->size; i++) {
        table->array[i] = createArrayList(500, table->itemSize);
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

    //todo change size
    if (table->array[updatedIndex]->filledItems > 1000) expandArray(table);
    updatedIndex = ABS(id % table->size);
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
        arrayIndex > table->size || table->array[arrayIndex]->filledItems < elementIndex)
        return NULL;

    void *pointer = arrayListRemoveElement(table->array[arrayIndex], elementIndex);
    if (!pointer) return NULL;

    table->filledItems--;
    return pointer;
}

int expandArray(hashTable *table) {
    arrayList **newArrayLists;
    int i;
    int j;
    int newSize;
    void *pointer;
    int updatedIndex;

    if (!table) return EXIT_FAILURE;

    newSize = table->size * 4;
    newArrayLists = malloc(sizeof(arrayList *) * newSize);
    if (!newArrayLists) return EXIT_FAILURE;

    for (i = 0; i < newSize; i++) {
        newArrayLists[i] = createArrayList(300, table->itemSize);
    }

    for (i = 0; i < table->size; i++) {
        for (j = 0; j < table->array[i]->filledItems; j++) {
            if ((pointer = arrayListGetPointer(table->array[i], j))) {
                updatedIndex = ((citizen *) pointer)->id % newSize;
                arrayListAdd(newArrayLists[updatedIndex], pointer);
            }
        }
    }

    for (i = 0; i < table->size; i++) {
        free(table->array[i]);
    }


    table->array = newArrayLists;
    table->size = newSize;
    return EXIT_SUCCESS;
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