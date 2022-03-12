#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "simulation.h"

#define radians(degrees) degrees * (M_PI / 180.0)


country *createCountry(int numberOfCities, int numberOfCitizens) {
    country *theCountry;
    if (numberOfCities <= 0 || numberOfCitizens <= 0) return NULL;

    theCountry = malloc(sizeof(country));
    if (!theCountry) return NULL;

    theCountry->cities = malloc(numberOfCities * sizeof(city *));
    if (!theCountry->cities) {
        free(theCountry);
        return NULL;
    }
    theCountry->citizens = malloc(numberOfCitizens * sizeof(citizen *));
    if (!theCountry->citizens) {
        free(theCountry->cities);
        free(theCountry);
        return NULL;
    }
    theCountry->numberOfCitizens = numberOfCitizens;
    theCountry->numberOfCities = numberOfCities;

    return theCountry;
}
city *createCity(int population, double lat, double lon) {
    city *theCity;
    if (population < 0) return NULL;
    theCity = calloc(1, sizeof(city));
    if (!theCity) return NULL;
    theCity->population = population;
    theCity->lat = lat;
    theCity->lon = lon;

    return theCity;
}
citizen *createCitizen(short cityID) {
    citizen *theCitizen;
    if (cityID < 0) return NULL;
    theCitizen = calloc(1, sizeof(citizen));
    if (!theCitizen) return NULL;
    theCitizen->cityID = cityID;

    return theCitizen;
}

void freeCountry(country **theCountry) {
    int i;
    if (!theCountry || !*theCountry) return;

    for (i = 0; i < (*theCountry)->numberOfCities; i++) {
        if ((*theCountry)->cities[i]) {
            freeCity(&(*theCountry)->cities[i]);
        }
    }
    for (i = 0; i < (*theCountry)->numberOfCitizens; i++) {
        if ((*theCountry)->citizens[i]) {
            freeCitizen(&(*theCountry)->citizens[i]);
        }
    }
    free((*theCountry)->citizens);
    free(*theCountry);
    *theCountry = NULL;
}
void freeCity(city **theCity) {
    if (!theCity || !*theCity) return;

    free(*theCity);
    *theCity = NULL;
}
void freeCitizen(citizen **theCitizen) {
    if (!theCitizen || !*theCitizen) return;

    free(*theCitizen);
    *theCitizen = NULL;
}

/* ---------------------- START OF BLOCK : CREATE STRUCT COUNTRY FROM READING CSV ------------------------------------*/

int number_of_citizens_from_csv(const char *filepath, int *number_of_cities) {
    // Ini
    FILE *fp = NULL;
    int i = 0, column_index = -1, population = 0, cities = 0;
    char buffer[255];
    char *token;

    // Opening csv file
    fp = fopen(filepath, "r");
    if (!fp) return -1;

    // Reading first line
    fgets(buffer, 255, fp);
    token = strtok(buffer, ",");
    // Determining which column has the population info
    while(token) {
        if (!strcmp(token, POPULATION_COLUMN_NAME)) column_index = i;
        token = strtok(NULL, ",");
        i++;
    }
    if (column_index == -1) return -1;

    // Reading the rest and counting the total population
    while(!feof(fp)) {
        fgets(buffer, 255, fp);
        token = strtok(buffer, ",");
        for (i = 0; i < column_index; i++) token = strtok(NULL, ",");
        population += atoi(token);
        cities++;
    }

    // Closing csv file
    if (fclose(fp) == EOF) return -1;

    // Return values
    *number_of_cities = cities;
    return population;
}

int process_csv(country **the_country, const char *filepath) {
    // Ini
    FILE *fp = NULL;
    double lon, lat;
    int i = 0, citizen_index = 0, population_index = -1, lat_index = -1, lon_index = -1, population;
    short city_index = 0;
    char buffer[255];
    char *token;

    // Opening csv file
    fp = fopen(filepath, "r");
    if (!fp) return 0;

    // Reading first line
    fgets(buffer, 255, fp);
    token = strtok(buffer, ",");
    // Determining which columns are useful
    while(token) {
        if (!strcmp(token, POPULATION_COLUMN_NAME)) population_index = i;
        if (!strcmp(token, LATITUDE_COLUMN_NAME)) lat_index = i;
        if (!strcmp(token, LONGITUDE_COLUMN_NAME)) lon_index = i;
        token = strtok(NULL, ",");
        i++;
    }
    if (population_index == -1 || lat_index == -1 || lon_index == -1) return 0;

    // Reading the rest and creating structs
    while(!feof(fp)) {
        fgets(buffer, 255, fp);
        token = strtok(buffer, ",");
        i = 0;
        while(1) {
            if (!token) break;
            if (i == population_index) population = atoi(token);
            if (i == lat_index) lat = atof(token);
            if (i == lon_index) lon = atof(token);
            token = strtok(NULL, ",");
            i++;
        }
        (*the_country)->cities[city_index] = createCity(population, lat, lon);
        for(i = 0; i < population_index; i++) {
            (*the_country)->citizens[citizen_index] = createCitizen(city_index);
            citizen_index++;
        }
        city_index++;
    }

    // Closing csv file
    if (fclose(fp) == EOF) return 0;

    return 1;
}

country* create_country_from_csv(const char *filepath) {
    int number_of_cities;
    int population = number_of_citizens_from_csv(filepath, &number_of_cities);

    if (population < 0) return NULL;

    country* temp = createCountry(number_of_cities, population);

    process_csv(&temp, filepath);

    return temp;
}

/* ---------------------- END OF BLOCK : CREATE STRUCT COUNTRY FROM READING CSV --------------------------------------*/

double randomDouble() {
    return (double) rand() / RAND_MAX * 2 - 1;
}

int randomGaussian(GaussRandom *randomPointer, double *doublePointer) {
    double v1;
    double v2;
    double s;
    double multiplier;

    if (!randomPointer || !doublePointer) {
        return EXIT_FAILURE;
    }

    if (randomPointer->hasNextValue) {
        randomPointer->hasNextValue = 0;
        *doublePointer = randomPointer->nextValue;
        return EXIT_SUCCESS;
    }

    do {
        v1 = randomDouble();
        v2 = randomDouble();
        s = (v1 * v1) + (v2 * v2);
    } while (s >= 1 || s == 0);
    multiplier = sqrt(-2 * log(s) / s);
    randomPointer->hasNextValue = 1;
    randomPointer->nextValue = v2 * multiplier;

    *doublePointer = v1 * multiplier;
    return EXIT_SUCCESS;

}

int nextNormalDistDouble(GaussRandom *randomPointer, double *doublePointer) {

    if (!randomPointer || !doublePointer) {
        return EXIT_FAILURE;
    }

    if (randomPointer->hasNextValue) {
        randomPointer->hasNextValue = 0;
        *doublePointer = randomPointer->mean + randomPointer->nextValue * randomPointer->stdDev;
        return EXIT_SUCCESS;
    }

    if (randomGaussian(randomPointer, doublePointer) == EXIT_FAILURE) return EXIT_FAILURE;
    *doublePointer = randomPointer->mean + *doublePointer * randomPointer->stdDev;
    return EXIT_SUCCESS;
}

int nextNormalDistDoubleFaster(GaussRandom *randomPointer, double *doublePointer) {
    double value1;
    double value2;
    double s;
    double multiplier;

    if (!randomPointer || !doublePointer) return EXIT_FAILURE;

    if (randomPointer->hasNextValue) {
        randomPointer->hasNextValue = 0;
        *doublePointer = randomPointer->nextValue;
        return EXIT_SUCCESS;
    }

    do {
        value1 = (double) rand() * stupidName - 1;
        value2 = (double) rand() * stupidName - 1;
        s = (value1 * value1) + (value2 * value2);
    } while (s >= 1 || s == 0);
    multiplier = sqrt(-2 * log(s) / s) * randomPointer->stdDev;
    randomPointer->hasNextValue = 1;
    randomPointer->nextValue = randomPointer->mean + (value2 * multiplier);
    *doublePointer = randomPointer->mean + (value1 * multiplier);
    return EXIT_SUCCESS;
}

GaussRandom *createRandom(double mean, double stdDev) {
    GaussRandom *newRandom;

    newRandom = malloc(sizeof(GaussRandom));
    if (newRandom) {
        newRandom->hasNextValue = 0;
        newRandom->stdDev = stdDev;
        newRandom->mean = mean;
    }

    return newRandom;
}

void freeRandom(GaussRandom **randomPointer) {
    if (!randomPointer || !*randomPointer) return;

    free(*randomPointer);
    *randomPointer = NULL;
}

double computeDistanceHaversine(double latitude1, double longitude1, double latitude2, double longitude2) {

    double sinLatitude = sin(radians((latitude1 - latitude2) * 0.5));
    double cosLatitude1 = cos(radians(latitude1));
    double cosLatitude2 = cos(radians(latitude2));
    double sinLongitude = sin(radians((longitude1 - longitude2) * 0.5));
    return 6371 * 2 * asin(sqrt(sinLatitude * sinLatitude + cosLatitude1 * cosLatitude2 * sinLongitude * sinLongitude));
}

double compute_distance(double latitude1, double longitude1, double latitude2, double longitude2) {
    double coef = 110.25;
    double x = latitude2 - latitude1;
    double y = (longitude2 - longitude1) * cos(radians(latitude1));
    return coef * sqrt(x * x + y * y);
}
