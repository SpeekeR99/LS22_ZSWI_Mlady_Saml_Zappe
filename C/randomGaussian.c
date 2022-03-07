#include <stdlib.h>
#include "randomGaussian.h"
#include "math.h"
#define radians(degrees) degrees * (M_PI / 180.0)
#define stupidName 0.000061037018951994385

typedef struct {
    char hasNextValue;
    double nextValue;
    double mean;
    double stdDev;
}random;

double randomDouble() {
    return (double) rand() / RAND_MAX * 2 - 1;
}

double randomGaussian(random *randomPointer) {
    //todo sanity check, is there nextValue?
    if (!randomPointer) {
        //nevim
    }

    if (randomPointer->hasNextValue) {
        randomPointer->hasNextValue = 0;
        return randomPointer->nextValue;
    }
    double v1;
    double v2;
    double s;
    double multiplier;
    do {
        v1 = randomDouble();
        v2 = randomDouble();
//        v1 = (double) rand() * stupidName - 1;
//        v2 = (double) rand() * stupidName - 1;
        s = (v1 * v1) + (v2 * v2);
    } while (s >= 1 || s == 0);
    multiplier = sqrt(-2 * log(s) / s);
    randomPointer->hasNextValue = 1;
    randomPointer->nextValue = v2 * multiplier;
    return v1 * multiplier;

}

double nextNormalDistDouble(random *randomPointer) {
    //todo sanity check
    if (!randomPointer) {
        //nevim
    }

    if (randomPointer->hasNextValue) {
        randomPointer->hasNextValue = 0;
        return randomPointer->mean + randomPointer->nextValue * randomPointer->stdDev;
    }

    return randomPointer->mean + randomGaussian(randomPointer) * randomPointer->stdDev;
}

double nextNormalDistDoubleFaster(random *randomPointer) {
    //todo sanity check
    if (!randomPointer) {
        //nevim
    }

    if (randomPointer->hasNextValue) {
        randomPointer->hasNextValue = 0;
        return randomPointer->nextValue;
    }
    double v1;
    double v2;
    double s;
    double multiplier;
    do {
        v1 = (double) rand() * stupidName - 1;
        v2 = (double) rand() * stupidName - 1;
        s = (v1 * v1) + (v2 * v2);
    } while (s >= 1 || s == 0);
    multiplier = sqrt(-2 * log(s) / s) * randomPointer->stdDev;
    randomPointer->hasNextValue = 1;
    randomPointer->nextValue = v2 * multiplier + randomPointer->mean;
    return randomPointer->mean + (v1 * multiplier);
}

random *createRandom(double mean, double stdDev) {
    random *newRandom;

    newRandom = malloc(sizeof(random));
    if (newRandom) {
        newRandom->hasNextValue = 0;
        newRandom->stdDev = stdDev;
        newRandom->mean = mean;
    }

    return newRandom;
}

void freeRandom(random **randomPointer) {
    if (!randomPointer || !*randomPointer) return;

    free(*randomPointer);
    *randomPointer = NULL;
}

double computeDistance(double latitude1, double longitude1, double latitude2, double longitude2) {

    double sinLatitude = sin(radians((latitude1 - latitude2) * 0.5));
    double cosLatitude1 = cos(radians(latitude1));
    double cosLatitude2 = cos(radians(latitude2));
    double sinLongitude = sin(radians((longitude1 - longitude2) * 0.5));
    return 6371 * 2 * asin(sqrt(sinLatitude * sinLatitude + cosLatitude1 * cosLatitude2 * sinLongitude * sinLongitude));
}



