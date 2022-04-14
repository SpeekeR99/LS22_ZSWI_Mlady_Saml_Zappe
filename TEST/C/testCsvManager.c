#include "Unity/src/unity.h"
#include "../../C/simulation/csvManager.h"

void setUp(void) {}

void test_create_country_from_csv_should_create(void) {
    country *c = create_country_from_csv("../../DATA/initial.csv");
    TEST_ASSERT_NOT_NULL(c);
    freeCountry(&c);
}

void test_create_country_from_csv_should_not_create_1(void) {
    country *c = create_country_from_csv("non-existant.csv");
    TEST_ASSERT_NULL(c);
}

void test_create_country_from_csv_should_not_create_2(void) {
    country *c = create_country_from_csv(NULL);
    TEST_ASSERT_NULL(c);
}

void test_create_csv_from_country_should_create(void) {
    country *c = create_country_from_csv("../../DATA/initial.csv");
    create_csv_from_country(c, "test.csv", 0);
    FILE *fp = fopen("test.csv", "r");
    TEST_ASSERT_NOT_NULL(fp);
    freeCountry(&c);
    fclose(fp);
    remove("test.csv");
}

void test_create_csv_from_country_should_not_create(void) {
    create_csv_from_country(NULL, "test.csv", 0);
    FILE *fp = fopen("test.csv", "r");
    TEST_ASSERT_NULL(fp);
    fclose(fp);
}

void tearDown(void) {}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_country_from_csv_should_create);
    RUN_TEST(test_create_country_from_csv_should_not_create_1);
    RUN_TEST(test_create_country_from_csv_should_not_create_2);
    RUN_TEST(test_create_csv_from_country_should_create);
    RUN_TEST(test_create_csv_from_country_should_not_create);
    return UNITY_END();
}