#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "simulation.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define radians(degrees) degrees * (M_PI / 180.0)

/** This function is one step of simulation where the citizens are moving between different cities
 *
 * @param theCountry initialized country
 * @param theGaussRandom gaussRandom struct with initialized mean and standard deviation
 * @param distances array of cityDistances as big as number of cities in the coutry
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int simulationStep(country *theCountry, GaussRandom *theGaussRandom, cityDistance **distances) {
    int i;
    int j;
    int k;
    int index;
    double *doublePointer;
    city *theCity;
    arrayList *theList;
    citizen *theCitizen;
    if (!theCountry || !theGaussRandom || !distances) return EXIT_FAILURE;

    doublePointer = malloc(sizeof (double));
    if (!doublePointer) return EXIT_FAILURE;

    //go through all cities
    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];
        computeDistances(i, theCountry, distances);
        qsort(distances, theCountry->numberOfCities, sizeof(cityDistance *), cmpCitiesByDistance);

        //go through all citizens in a city
        for (j = 0; j < theCity->citizens->size; j++) {
            theList = theCity->citizens->array[j];

            //todo takes just 50% of citizens
            for (k = 0; k < theList->filledItems; k += 2) {
                theCitizen = (citizen *) arrayListGetPointer(theList, k);

                //something to stop one citizen to move more than once per step (will be somehow changed)
                if (theCitizen->timeFrame == -1) continue;
                theCitizen->timeFrame = -1;

                //maybe we could delete this, what can possibly happen :)
                if (nextNormalDistDouble(theGaussRandom, doublePointer) == EXIT_FAILURE) {
                    free(doublePointer);
                    return EXIT_FAILURE;
                }

                //finds city which is the closest (not really) to the distance which citizen should travel
                index = interpolationSearch(*doublePointer, theCountry->numberOfCities, distances);

                //move the citizen from one city to another
                hashTableRemoveElement(j, k, theCity->citizens);
                hashTableAddElement(theCitizen, theCitizen->id, theCountry->cities[index]->citizens);
            }
        }
    }

    free(doublePointer);
    return EXIT_SUCCESS;
}

/**
 * This function computes distances (not with haversine, but with the faster function)
 * to all cities in theCountry from city at cityIndex and saves them in the distances
 * array
 * @param cityIndex index of the city from which the distances are calculated
 * @param theCountry wrapper of all the cities
 * @param distances initialized array, can contain some data, they will be overwritten,
 *                  must have the same size as number of cities in the Country
 */
void computeDistances(int cityIndex, country *theCountry, cityDistance **distances) {
    int i;
    double distance;
    city *theCity;
    if (cityIndex < 0 || !theCountry || !distances) return;

    theCity = theCountry->cities[cityIndex];

    //all cities before city at cityIndex
    for (i = 0; i < cityIndex; i++) {
        distance = compute_distance(theCity, theCountry->cities[i]);
        distances[i]->distance = distance;
        distances[i]->id = i;
    }

    //all cities after city at cityIndex
    for (i = cityIndex + 1; i < theCountry->numberOfCities; i++) {
        distance = compute_distance(theCity, theCountry->cities[i]);
        distances[i]->distance = distance;
        distances[i]->id = i;
    }

    //make city at cityIndex unreachable
    distances[cityIndex]->distance = DBL_MAX;
    distances[cityIndex]->id = cityIndex;
}

country *createCountry(int numberOfCities) {
    country *theCountry;
    if (numberOfCities <= 0) return NULL;

    theCountry = malloc(sizeof(country));
    if (!theCountry) return NULL;

    theCountry->cities = malloc(numberOfCities * sizeof(city *));
    if (!theCountry->cities) {
        free(theCountry);
        return NULL;
    }
//    theCountry->citizens = malloc(numberOfCitizens * sizeof(citizen *));
//    if (!theCountry->citizens) {
//        free(theCountry->cities);
//        free(theCountry);
//        return NULL;
//    }
//    theCountry->numberOfCitizens = numberOfCitizens;
    theCountry->numberOfCities = numberOfCities;

    return theCountry;
}

city *createCity(int population, double lat, double lon) {
    city *theCity;
    if (population < 0) return NULL;
    theCity = calloc(1, sizeof(city));
    if (!theCity) return NULL;
    theCity->citizens = createHashTable(population / 10, sizeof(citizen *));
    if (!theCity->citizens) {
        free(theCity);
        return NULL;
    }

    theCity->population = population;
    theCity->lat = lat;
    theCity->lon = lon;

    return theCity;
}

citizen *createCitizen(int id) {
    citizen *theCitizen;
    if (id < 0) return NULL;
    theCitizen = calloc(1, sizeof(citizen));
    if (!theCitizen) return NULL;
    theCitizen->id = id;

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
//    for (i = 0; i < (*theCountry)->numberOfCitizens; i++) {
//        if ((*theCountry)->citizens[i]) {
//            freeCitizen(&(*theCountry)->citizens[i]);
//        }
//    }
//    free((*theCountry)->citizens);
    free(*theCountry);
    *theCountry = NULL;
}

void freeCity(city **theCity) {
    if (!theCity || !*theCity) return;

    freeHashTable(&(*theCity)->citizens);
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
    city *theCity;
    citizen *theCitizen;

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
        theCity = (*the_country)->cities[city_index];

        for (i = 0; i < theCity->population; i++) {
            theCitizen = createCitizen(citizen_index);
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

country* create_country_from_csv(const char *filepath) {
    int number_of_cities;
    int population = number_of_citizens_from_csv(filepath, &number_of_cities);

    if (population < 0) return NULL;

    country* temp = createCountry(number_of_cities);

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

int interpolationSearch(double distance, int citiesSize, cityDistance **cityDistances) {
    int left;
    int middle;
    int right;

    left = 0;
    right = citiesSize - 1;
    if (cityDistances[left]->distance > distance) return 0;
    if (cityDistances[right]->distance < distance) return citiesSize - 1;

    while (cityDistances[left]->distance < distance && cityDistances[right]->distance >= distance) {
        middle = (int) (left + ((distance - cityDistances[left]->distance) * (right - left))
                / (cityDistances[right]->distance - cityDistances[left]->distance));
        if (cityDistances[middle]->distance > distance) {
            right = middle - 1;
        } else if (cityDistances[middle]->distance < distance) {
            left = middle + 1;
        } else {
            return middle;
        }
    }

    return left;
}

double compute_distance(city *firstCity, city *secondCity) {
    double coef = 110.25;
    double x = secondCity->lat - firstCity->lat;
    double y = (secondCity->lon - firstCity->lon) * cos(radians(firstCity->lat));
    return coef * sqrt(x * x + y * y);
}


cityDistance *createCityDistance() {
    cityDistance *theCityDistance;
    theCityDistance = malloc(sizeof(cityDistance));
    if (!theCityDistance) return NULL;

    return theCityDistance;
}

void freeCityDistance(cityDistance **theCityDistance) {
    if (!theCityDistance || !*theCityDistance) return;

    free(*theCityDistance);
    *theCityDistance = NULL;
}

int cmpCitiesByDistance(const void *a, const void *b) {
    return (*(cityDistance **) a)->distance < (*(cityDistance **) b)->distance ? -1 : 1;
}

