#include "Unity/src/unity.h"
#include "../../C/simulation/arrayList.h"

void setUp(void) {}

void test_createArrayList_should_not_be_null(void) {
    arrayList *al = createArrayList(1, 1);
    TEST_ASSERT_NOT_NULL(al);
    freeArrayList(&al);
}

void test_createArrayList_should_be_null_1(void) {
    arrayList *al = createArrayList(-1, 1);
    TEST_ASSERT_NULL(al);
    freeArrayList(&al);
}

void test_createArrayList_should_be_null_2(void) {
    arrayList *al = createArrayList(1, -1);
    TEST_ASSERT_NULL(al);
    freeArrayList(&al);
}

void test_arrayListAdd_should_add(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    int worked = arrayListAdd(al, (void *) 5);
    TEST_ASSERT_EQUAL(worked, 1);
    TEST_ASSERT_EQUAL(5, (int *) al->data[0]);
    freeArrayList(&al);
}

void test_arrayListAdd_should_not_add_1(void) {
    int worked = arrayListAdd(NULL, (void *) 5);
    TEST_ASSERT_EQUAL(worked, 0);
}

void test_arrayListAdd_should_not_add_2(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    int worked = arrayListAdd(al, (void *) NULL);
    TEST_ASSERT_EQUAL(worked, 0);
    freeArrayList(&al);
}

void test_arrayListExpand_should_expand(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    int size_orig = al->listSize;
    arrayListExpand(al);
    int size_new = al->listSize;
    TEST_ASSERT_EQUAL(size_orig * 2, size_new);
    freeArrayList(&al);
}

void test_arrayListGetPointer_should_get_pointer(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    arrayListAdd(al, (void *) 5);
    int pointer = (int) arrayListGetPointer(al, 0);
    TEST_ASSERT_EQUAL(pointer, 5);
    freeArrayList(&al);
}

void test_arrayListGetPointer_should_not_get_pointer_1(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    arrayListAdd(al, (void *) 5);
    int *pointer = (int *) arrayListGetPointer(al, 1);
    TEST_ASSERT_NULL(pointer);
    freeArrayList(&al);
}

void test_arrayListGetPointer_should_not_get_pointer_2(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    arrayListAdd(al, (void *) 5);
    int *pointer = (int *) arrayListGetPointer(al, -1);
    TEST_ASSERT_NULL(pointer);
    freeArrayList(&al);
}

void test_arrayListGetPointer_should_not_get_pointer_3(void) {
    int *pointer = (int *) arrayListGetPointer(NULL, 0);
    TEST_ASSERT_NULL(pointer);
}

void test_arrayListRemoveElement_should_remove(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    arrayListAdd(al, (void *) 5);
    arrayListAdd(al, (void *) 10);
    int pointer = (int) arrayListRemoveElement(al, 0);
    TEST_ASSERT_EQUAL(pointer, 5);
    pointer = (int) arrayListGetPointer(al, 0);
    TEST_ASSERT_EQUAL(pointer, 10);
    freeArrayList(&al);
}

void test_arrayListRemoveElement_should_not_remove_1(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    int *pointer = (int *) arrayListRemoveElement(al, 0);
    TEST_ASSERT_NULL(pointer);
    freeArrayList(&al);
}

void test_arrayListRemoveElement_should_not_remove_2(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    int *pointer = (int *) arrayListRemoveElement(al, -5);
    TEST_ASSERT_NULL(pointer);
    freeArrayList(&al);
}

void test_arrayListRemoveElement_should_not_remove_3(void) {
    int *pointer = (int *) arrayListRemoveElement(NULL, 0);
    TEST_ASSERT_NULL(pointer);
}

void test_freeArrayList(void) {
    arrayList *al = createArrayList(10, sizeof(int));
    freeArrayList(&al);
    TEST_ASSERT_NULL(al);
}


void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_createArrayList_should_not_be_null);
    RUN_TEST(test_createArrayList_should_be_null_1);
    RUN_TEST(test_createArrayList_should_be_null_2);
    RUN_TEST(test_arrayListAdd_should_add);
    RUN_TEST(test_arrayListAdd_should_not_add_1);
    RUN_TEST(test_arrayListAdd_should_not_add_2);
    RUN_TEST(test_arrayListExpand_should_expand);
    RUN_TEST(test_arrayListGetPointer_should_get_pointer);
    RUN_TEST(test_arrayListGetPointer_should_not_get_pointer_1);
    RUN_TEST(test_arrayListGetPointer_should_not_get_pointer_2);
    RUN_TEST(test_arrayListGetPointer_should_not_get_pointer_3);
    RUN_TEST(test_arrayListRemoveElement_should_remove);
    RUN_TEST(test_arrayListRemoveElement_should_not_remove_1);
    RUN_TEST(test_arrayListRemoveElement_should_not_remove_2);
    RUN_TEST(test_arrayListRemoveElement_should_not_remove_3);
    RUN_TEST(test_freeArrayList);
    return UNITY_END();
}