#include <stdlib.h>
#include <math.h>
#include "simulation.h"
#define radians(degrees) degrees * (M_PI / 180.0)


country *createCountry(int numberOfCities, int numberOfCitizens) {
    country *theCountry;
    if (numberOfCities <= 0 || numberOfCitizens <= 0) return NULL;

    theCountry = malloc(sizeof(country));
    if (!theCountry) return NULL;

    theCountry->citiesPopulation = malloc(numberOfCities * sizeof(int));
    if (!theCountry->citiesPopulation) {
        free(theCountry);
        return NULL;
    }
    theCountry->citizens = malloc(numberOfCitizens * sizeof(citizen *));
    if (!theCountry->citizens) {
        free(theCountry->citiesPopulation);
        free(theCountry);
        return NULL;
    }
    theCountry->numberOfCitizens = numberOfCitizens;
    theCountry->numberOfCities = numberOfCities;

    return theCountry;
}
citizen *createCitizen(int cityID) {
    citizen *theCitizen;
    if (cityID < 0) return NULL;
    theCitizen = calloc(1, sizeof(citizen));
    if (!theCitizen) return NULL;

    return theCitizen;
}

void freeCountry(country **theCountry) {
    int i;
    if (!theCountry || !*theCountry) return;

    free((*theCountry)->citiesPopulation);
    for (i = 0; i < (*theCountry)->numberOfCitizens; i++) {
        if ((*theCountry)->citizens[i]) {
            freeCitizen(&(*theCountry)->citizens[i]);
        }
    }
    free((*theCountry)->citizens);
    free(*theCountry);
    *theCountry = NULL;
}
void freeCitizen(citizen **theCitizen) {
    if (!theCitizen || !*theCitizen) return;

    free(*theCitizen);
    *theCitizen = NULL;
}


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


