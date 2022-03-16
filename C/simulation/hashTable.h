#ifndef GRAPH_C_HASHTABLE_H
#define GRAPH_C_HASHTABLE_H
#define ABS(x) (((x) >= (0)) ? (x) : (-(x)))

#include "arrayList.h"

typedef struct {
    int size;
    int itemSize;
    int filledItems;
    arrayList **array;
} hashTable;

hashTable *createHashTable(int size, int itemSize);
int hashTableAddElement(void *element, int id, hashTable *table);
void *hashTableRemoveElement(int arrayIndex, int elementIndex, hashTable *table);
void freeHashTable(hashTable **table);

#endif
