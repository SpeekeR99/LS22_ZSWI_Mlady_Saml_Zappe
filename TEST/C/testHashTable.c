#include "Unity/src/unity.h"
#include "../../C/simulation/hashTable.h"

void setUp(void) {}

void test_createHashTable_should_not_be_null_1(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    TEST_ASSERT_NOT_NULL(ht);
    freeHashTable(&ht);
}

void test_createHashTable_should_not_be_null_2(void) {
    hashTable *ht = createHashTable(0, sizeof(int));
    TEST_ASSERT_NOT_NULL(ht);
    freeHashTable(&ht);
}

void test_createHashTable_should_be_null_1(void) {
    hashTable *ht = createHashTable(-1, sizeof(int));
    TEST_ASSERT_NULL(ht);
    freeHashTable(&ht);
}

void test_createHashTable_should_be_null_2(void) {
    hashTable *ht = createHashTable(10, -1);
    TEST_ASSERT_NULL(ht);
    freeHashTable(&ht);
}

void test_hashTableAddElement_should_add_1(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    int worked = hashTableAddElement((void *) 5, 5, ht);
    TEST_ASSERT_EQUAL(1, worked);
    TEST_ASSERT_EQUAL(5, arrayListGetPointer(ht->array[5], 0));
    freeHashTable(&ht);
}

void test_hashTableAddElement_should_add_2(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    int worked = hashTableAddElement((void *) 5, -5, ht);
    TEST_ASSERT_EQUAL(1, worked);
    TEST_ASSERT_EQUAL(5, arrayListGetPointer(ht->array[5], 0));
    freeHashTable(&ht);
}

void test_hashTableAddElement_should_not_add_1(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    int worked = hashTableAddElement(NULL, 5, ht);
    TEST_ASSERT_EQUAL(0, worked);
    freeHashTable(&ht);
}

void test_hashTableAddElement_should_not_add_2(void) {
    int worked = hashTableAddElement((void *) 5, 5, NULL);
    TEST_ASSERT_EQUAL(0, worked);
}

void test_hashTableRemoveElement_should_remove(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    hashTableAddElement((void *) 5, 5, ht);
    int removed = (int) hashTableRemoveElement(5, 0, ht);
    TEST_ASSERT_EQUAL(5, removed);
    freeHashTable(&ht);
}

void test_hashTableRemoveElement_should_not_remove_1(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    hashTableAddElement((void *) 5, 5, ht);
    int *removed = hashTableRemoveElement(0, 5, ht);
    TEST_ASSERT_NULL(removed);
    freeHashTable(&ht);
}

void test_hashTableRemoveElement_should_not_remove_2(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    int *removed = hashTableRemoveElement(-5, 5, ht);
    TEST_ASSERT_NULL(removed);
    freeHashTable(&ht);
}

void test_hashTableRemoveElement_should_not_remove_3(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    int *removed = hashTableRemoveElement(0, -5, ht);
    TEST_ASSERT_NULL(removed);
    freeHashTable(&ht);
}

void test_hashTableRemoveElement_should_not_remove_4(void) {
    int *removed = hashTableRemoveElement(0, 5, NULL);
    TEST_ASSERT_NULL(removed);
}

void test_expandArray_should_expand(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    int worked = expandArray(ht);
    TEST_ASSERT_EQUAL(0, worked);
    TEST_ASSERT_EQUAL(40, ht->size);
    freeHashTable(&ht);
}

void test_expandArray_should_not_expand(void) {
    int worked = expandArray(NULL);
    TEST_ASSERT_EQUAL(1, worked);
}

void test_freeHashTable(void) {
    hashTable *ht = createHashTable(10, sizeof(int));
    freeHashTable(&ht);
    TEST_ASSERT_NULL(ht);
}

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_createHashTable_should_not_be_null_1);
    RUN_TEST(test_createHashTable_should_not_be_null_2);
    RUN_TEST(test_createHashTable_should_be_null_1);
    RUN_TEST(test_createHashTable_should_be_null_2);
    RUN_TEST(test_hashTableAddElement_should_add_1);
    RUN_TEST(test_hashTableAddElement_should_add_2);
    RUN_TEST(test_hashTableAddElement_should_not_add_1);
    RUN_TEST(test_hashTableAddElement_should_not_add_2);
    RUN_TEST(test_hashTableRemoveElement_should_remove);
    RUN_TEST(test_hashTableRemoveElement_should_not_remove_1);
    RUN_TEST(test_hashTableRemoveElement_should_not_remove_2);
    RUN_TEST(test_hashTableRemoveElement_should_not_remove_3);
    RUN_TEST(test_hashTableRemoveElement_should_not_remove_4);
    RUN_TEST(test_expandArray_should_expand);
    RUN_TEST(test_expandArray_should_not_expand);
    RUN_TEST(test_freeHashTable);
    return UNITY_END();
}