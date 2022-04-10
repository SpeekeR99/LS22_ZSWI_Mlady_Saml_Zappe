#include "Unity/src/unity.h"
#include "../../C/simulation/random.h"

void setUp(void) {}

void test_createRandom(void) {
    GaussRandom *rand = createRandom(1, 1);
    TEST_ASSERT_NOT_NULL(rand);
    freeRandom(&rand);
}

void test_randomDouble(void) {
    int i;
    for (i = 0; i < 1000; i++) {
        double rand = randomDouble();
        if (rand < -1 || rand > 1) TEST_ASSERT_NOT_NULL(NULL);
    }
    TEST_ASSERT_NULL(NULL);
}

void test_randomGaussian_should_work(void) {
    GaussRandom *rand = createRandom(1, 1);
    double num = 2.;
    int worked = randomGaussian(rand, &num);
    TEST_ASSERT_EQUAL(0, worked);
    freeRandom(&rand);
}

void test_randomGaussian_should_not_work_1(void) {
    GaussRandom *rand = createRandom(1, 1);
    int worked = randomGaussian(rand, NULL);
    TEST_ASSERT_EQUAL(1, worked);
    freeRandom(&rand);
}

void test_randomGaussian_should_not_work_2(void) {
    double num = 2.;
    int worked = randomGaussian(NULL, &num);
    TEST_ASSERT_EQUAL(1, worked);
}

void test_nextNormalDistDouble_should_work(void) {
    GaussRandom *rand = createRandom(1, 1);
    double num = 2.;
    int worked = nextNormalDistDouble(rand, &num);
    TEST_ASSERT_EQUAL(0, worked);
    freeRandom(&rand);
}

void test_nextNormalDistDouble_should_not_work_1(void) {
    GaussRandom *rand = createRandom(1, 1);
    int worked = nextNormalDistDouble(rand, NULL);
    TEST_ASSERT_EQUAL(1, worked);
    freeRandom(&rand);
}

void test_nextNormalDistDouble_should_not_work_2(void) {
    double num = 2.;
    int worked = nextNormalDistDouble(NULL, &num);
    TEST_ASSERT_EQUAL(1, worked);
}

void test_nextNormalDistDoubleFaster_should_work(void) {
    GaussRandom *rand = createRandom(1, 1);
    double num = 2.;
    int worked = nextNormalDistDoubleFaster(rand, &num);
    TEST_ASSERT_EQUAL(0, worked);
    freeRandom(&rand);
}

void test_nextNormalDistDoubleFaster_should_not_work_1(void) {
    GaussRandom *rand = createRandom(1, 1);
    int worked = nextNormalDistDoubleFaster(rand, NULL);
    TEST_ASSERT_EQUAL(1, worked);
    freeRandom(&rand);
}

void test_nextNormalDistDoubleFaster_should_not_work_2(void) {
    double num = 2.;
    int worked = nextNormalDistDoubleFaster(NULL, &num);
    TEST_ASSERT_EQUAL(1, worked);
}

void test_freeRandom(void) {
    GaussRandom *rand = createRandom(1, 1);
    freeRandom(&rand);
    TEST_ASSERT_NULL(rand);
}

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_createRandom);
    RUN_TEST(test_randomDouble);
    RUN_TEST(test_randomGaussian_should_work);
    RUN_TEST(test_randomGaussian_should_not_work_1);
    RUN_TEST(test_randomGaussian_should_not_work_2);
    RUN_TEST(test_nextNormalDistDouble_should_work);
    RUN_TEST(test_nextNormalDistDouble_should_not_work_1);
    RUN_TEST(test_nextNormalDistDouble_should_not_work_2);
    RUN_TEST(test_nextNormalDistDoubleFaster_should_work);
    RUN_TEST(test_nextNormalDistDoubleFaster_should_not_work_1);
    RUN_TEST(test_nextNormalDistDoubleFaster_should_not_work_2);
    RUN_TEST(test_freeRandom);
    return UNITY_END();
}