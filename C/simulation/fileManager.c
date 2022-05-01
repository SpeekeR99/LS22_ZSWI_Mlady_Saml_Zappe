#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fileManager.h"

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
 * @param create_citizens 1 if citizens have to be create, 0 otherwise
 * @return 1 if everything went fine, 0 otherwise
 */
int process_csv(country **the_country, const char *filepath, int create_citizens) {
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

        if (!create_citizens) {
            city_index++;
            continue;
        }

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
 * @param create_citizens 1 if citizens have to be create, 0 otherwise
 * @return Pointer to country struct or NULL
 */
country *create_country_from_csv(const char *filepath, int create_citizens) {
    if (!filepath) return NULL;

    int number_of_cities = number_of_cities_from_csv(filepath);
    if (number_of_cities < 0) return NULL;

    country *temp = createCountry(number_of_cities);
    process_csv(&temp, filepath, create_citizens);

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

/**
 * Saves the state of the country into binary file
 * @param date current frame number
 * @return 1 if save was successful, 0 otherwise
 */
int save_state(country *the_country, int date) {
    int i, j, k;
    city *the_city;
    arrayList *the_list;
    citizen *the_citizen;
    FILE *fp = NULL;

    fp = fopen(SAVE_FILEPATH, "wb");
    if (!fp) return 0;

    fwrite(&(date), sizeof(date), 1, fp);
    for (i = 0; i < the_country->numberOfCities; i++) {
        the_city = the_country->cities[i];

        for (j = 0; j < the_city->citizens->size; j++) {
            the_list = the_city->citizens->array[j];

            for (k = 0; k < the_list->filledItems; k++) {
                the_citizen = arrayListGetPointer(the_list, k);

                fwrite((&the_citizen), sizeof(the_citizen), 1, fp);
                fwrite(&(the_city->city_id), sizeof(the_city->city_id), 1, fp);
            }
        }
    }

    if (fclose(fp) == EOF) return 0;

    return 1;
}

/**
 * Loads the state of the country from binary file
 * Using buffer loads 1000 of citizens at once, took down the time from 220 secs to 0.3 secs
 * @param the_country basic country without citizens
 * @return number of loaded frame (date)
 */
int load_state(country *the_country) {
    int i, date, citizen_id = 0, city_id, size;
    city *the_city;
    citizen *the_citizen;
    FILE *fp = NULL;
    char buffer[14*1000]; // 14 bytes needed per citizen (10 for citizen, 4 for city_id)

    for (i = 0; i < the_country->numberOfCities; i++) {
        the_city = the_country->cities[i];
        the_city->population = 0;
        the_city->infected = 0;
    }

    fp = fopen(SAVE_FILEPATH, "rb");

    fread(&date, sizeof(date), 1, fp);

    while(!feof(fp)) {
        the_citizen = calloc(1, sizeof(citizen));
        size = fread(&buffer, sizeof(the_citizen) + sizeof(city_id), 1000, fp);

        for (i = 0; i < size; i += 14) {
            the_citizen->homeTown = buffer[i + 4];
            the_citizen->status = buffer[i + 8];
            the_citizen->timeFrame = buffer[i + 9];
            city_id = buffer[i + 10];
        }

        the_citizen->id = citizen_id++;

        for (i = 0; i < the_country->numberOfCities; i++) {
            the_city = the_country->cities[i];

            if (the_city->city_id == city_id) {
                hashTableAddElement(the_citizen, the_citizen->id, the_city->citizens);
                the_city->population++;
                if (the_citizen->status == INFECTED) the_city->infected++;
                break;
            }

        }
    }

    return date;
}