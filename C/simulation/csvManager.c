#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "csvManager.h"

/**
 * Basically just counts the number of lines in the csv file (minus one - heading)
 * @param filepath Path to the csv file
 * @return Number of cities in the csv file
 */
int number_of_cities_from_csv(const char *filepath) {
    // Ini
    FILE *fp = NULL;
    int cities = 0;
    char buffer[255];

    // Opening csv file
    fp = fopen(filepath, "r");
    if (!fp) return -1;

    // Skipping first line
    fgets(buffer, 255, fp);

    // Reading the rest and counting the total population
    while (!feof(fp)) {
        fgets(buffer, 255, fp);
        cities++;
    }

    // Closing csv file
    if (fclose(fp) == EOF) return -1;

    return cities;
}

/**
 * Reads the CSV file input and fills the struct country based on that
 * @param the_country Country struct being filled
 * @param filepath Path to the input CSV file
 * @return 1 if everything went fine, 0 otherwise
 */
int process_csv(country **the_country, const char *filepath) {
    // Ini
    FILE *fp = NULL;
    double lon, lat, area;
    int i = 0, citizen_index = 0, population_index = -1, lat_index = -1, lon_index = -1, city_id_index = -1,
            infected_index = -1, area_index = -1, population, city_id, infected;
    short city_index = 0;
    char buffer[255];
    char *token;
    city *theCity;
    citizen *theCitizen;

    // Opening csv file
    fp = fopen(filepath, "r");
    if (!fp) return 0;

    // Reading first line
    fgets(buffer, 255, fp);
    token = strtok(buffer, ",");
    // Determining which columns are useful
    while (token) {
        if (!strcmp(token, POPULATION_COLUMN_NAME)) population_index = i;
        if (!strcmp(token, LATITUDE_COLUMN_NAME)) lat_index = i;
        if (!strcmp(token, LONGITUDE_COLUMN_NAME)) lon_index = i;
        if (!strcmp(token, CITY_ID_COLUMN_NAME)) city_id_index = i;
        if (!strcmp(token, CITY_AREA_COLUMN_NAME)) area_index = i;
        if (!strcmp(token, INFECTED_COLUMN_NAME)) infected_index = i;
        token = strtok(NULL, ",");
        i++;
    }
    if (population_index == -1 || lat_index == -1 || lon_index == -1 || city_id_index == -1 ||
        area_index == -1 || infected_index == -1)
        return 0;

    // Reading the rest and creating structs
    while (!feof(fp)) {
        fgets(buffer, 255, fp);
        token = strtok(buffer, ",");
        i = 0;
        while (1) {
            if (!token) break;
            if (i == population_index) population = atoi(token);
            if (i == lat_index) lat = atof(token);
            if (i == lon_index) lon = atof(token);
            if (i == city_id_index) city_id = atoi(token);
            if (i == area_index) area = atof(token);
            if (i == infected_index) infected = atoi(token);
            token = strtok(NULL, ",");
            i++;
        }
        (*the_country)->cities[city_index] = createCity(city_id, area, population, infected, lat, lon);
        theCity = (*the_country)->cities[city_index];

        //set up infected citizens first
        for (i = 0; i < theCity->infected; i++) {
            theCitizen = createCitizen(citizen_index, city_index);
            if (!theCitizen) return 0;
            theCitizen->status = INFECTED;
            hashTableAddElement(theCitizen, citizen_index, theCity->citizens);
            citizen_index++;
        }

        for (i = 0; i < theCity->population - theCity->infected; i++) {
            theCitizen = createCitizen(citizen_index, city_index);
            if (!theCitizen) return 0;
            hashTableAddElement(theCitizen, citizen_index, theCity->citizens);
            citizen_index++;
        }

        city_index++;
    }

    // Closing csv file
    if (fclose(fp) == EOF) return 0;

    return 1;
}

/**
 * Creates country struct and fills it in based on the input CSV file
 * @param filepath Path to the input CSV file
 * @return Pointer to country struct or NULL
 */
country *create_country_from_csv(const char *filepath) {
    if (!filepath) return NULL;

    int number_of_cities = number_of_cities_from_csv(filepath);
    if (number_of_cities < 0) return NULL;

    country *temp = createCountry(number_of_cities);
    process_csv(&temp, filepath);

    return temp;
}

/**
 * Creates CSV file based on country struct
 * @param the_country Input country struct
 * @param filepath Path to the output CSV file
 * @param date Date is basically the animation frame
 * @return 1 if everything went fine, 0 otherwise
 */
int create_csv_from_country(country *the_country, const char *filepath, int date) {
    // Ini
    FILE *fp = NULL;
    city *curr_city = NULL;
    int i;

    // Sanity check
    if (!the_country || !filepath) return 0;

    // Create file
    fp = fopen(filepath, "w");
    if (!fp) return 0;
    fprintf(fp, "kod_obce,pocet_obyvatel,pocet_nakazenych,datum\n");

    for (i = 0; i < the_country->numberOfCities; i++) {
        curr_city = the_country->cities[i];
        fprintf(fp, "%d,", curr_city->city_id);
        fprintf(fp, "%d,", curr_city->population);
        fprintf(fp, "%d,", curr_city->infected);
        fprintf(fp, "%d\n", date);
    }

    // Closing csv file
    if (fclose(fp) == EOF) return 0;

    return 1;
}