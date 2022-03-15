#ifndef SEMESTRALKA_ARRAYLIST_H
#define SEMESTRALKA_ARRAYLIST_H
#define INCREMENT 10
#define FAILURE 0
#define SUCCESS 1

/* ____________________________________________________________________________

    Structures and Datatypes
   ____________________________________________________________________________
*/
typedef unsigned int uint;
typedef struct {
    uint listSize;
    int itemSize;
    void **data;
    uint filledItems;
} arrayList;

/* ____________________________________________________________________________

    Function Prototypes
   ____________________________________________________________________________
*/
arrayList *createArrayList(int listSize, int itemSize);
int arrayListAdd(arrayList *list, void *pointer);
int arrayListExpand(arrayList *list, int increment);
void *arrayListGetPointer(arrayList *list, int index);
void *arrayListRemoveElement(arrayList *list, int index);
void freeArrayList(arrayList **list);


#endif
