#include "Unity/src/unity.h"
#include "../../C/simulation/simulation.h"

void setUp(void) {}

void test_createCitizen_should_not_be_null(void) {
    citizen *prsn = createCitizen(0, 0);
    TEST_ASSERT_NOT_NULL(prsn);
    freeCitizen(&prsn);
}

void test_createCitizen_should_be_null(void) {
    citizen *prsn = createCitizen(-1, 0);
    TEST_ASSERT_NULL(prsn);
}

void test_freeCitizen(void) {
    citizen *prsn = createCitizen(0, 0);
    freeCitizen(&prsn);
    TEST_ASSERT_NULL(prsn);
}

void test_createCity_should_not_be_null(void) {
    city *city1 = createCity(0, 0, 1, 0, 0, 0);
    TEST_ASSERT_NOT_NULL(city1);
    freeCity(&city1);
}

void test_createCity_should_be_null(void) {
    city *city1 = createCity(0, 0, 0, 0, 0, 0);
    TEST_ASSERT_NULL(city1);
}

void test_freeCity(void) {
    city *city1 = createCity(0, 0, 1, 0, 0, 0);
    freeCity(&city1);
    TEST_ASSERT_NULL(city1);
}

void test_createCountry_should_not_be_null(void) {
    country *ctry = createCountry(1);
    TEST_ASSERT_NOT_NULL(ctry);
    freeCountry(&ctry);
}

void test_createCountry_should_be_null(void) {
    country *ctry = createCountry(0);
    TEST_ASSERT_NULL(ctry);
}

void test_freeCountry(void) {
    country *ctry = createCountry(1);
    freeCountry(&ctry);
    TEST_ASSERT_NULL(ctry);
}

void test_createCityDistance(void) {
    cityDistance *ctdst = createCityDistance();
    TEST_ASSERT_NOT_NULL(ctdst);
    freeCityDistance(&ctdst);
}

void test_freeCityDistance(void) {
    cityDistance *ctdst = createCityDistance();
    freeCityDistance(&ctdst);
    TEST_ASSERT_NULL(ctdst);
}

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_createCitizen_should_not_be_null);
    RUN_TEST(test_createCitizen_should_be_null);
    RUN_TEST(test_freeCitizen);
    RUN_TEST(test_createCity_should_not_be_null);
    RUN_TEST(test_createCity_should_be_null);
    RUN_TEST(test_freeCity);
    RUN_TEST(test_createCountry_should_not_be_null);
    RUN_TEST(test_createCountry_should_be_null);
    RUN_TEST(test_freeCountry);
    RUN_TEST(test_createCityDistance);
    RUN_TEST(test_freeCityDistance);
    return UNITY_END();
}