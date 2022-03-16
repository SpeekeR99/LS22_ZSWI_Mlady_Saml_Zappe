#include <math.h>
#include <stdlib.h>
#include "random.h"

/**
 * Returns random double from interval <-1, 1>
 * @return double
 */
double randomDouble() {
    return (double) rand() / RAND_MAX * 2 - 1;
}

/**
 * Returns normally distributed value with mean 0.0 and standard deviation
 * 1.0, this uses polar method described in The Art of Computer Programming.
 * Generates two values #efficiency
 * @param randomPointer not null pointer to GaussRandom
 * @param doublePointer where the value will be stored
 * @return EXIT_SUCCESS or EXIT_FAILURE in case of invalid parameters
 */
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

/**
 * Returns normally distributed value with mean and standard deviation specified
 * by attributes of @param randomPointer
 * @param randomPointer not null randomPointer which specifies mean and standard deviation
 * @param doublePointer where return value will be stored
 * @return EXIT_SUCCESS or EXIT_FAILURE when parameters are invalid or it is not possible
 *         to call randomGaussian() successfully
 */
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


/**
 * A bit faster function which returns normally distributed value with mean and standard
 * deviation specified by attributes of @param randomPointer
 * @param randomPointer not null randomPointer which specifies mean and standard deviation
 * @param doublePointer where return value will be stored
 * @return EXIT_SUCCESS or EXIT_FAILURE when parameters are invalid or it is not possible
 *         to call randomGaussian() successfully
 */
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

/**
 * Dummy contructor to GaussRandom struct
 * @param mean  of normally distributed values
 * @param stdDev must be positive, standard deviation
 * @return pointer to struct GaussRandom or NULL if it is not possible
 *         to allocate memory
 */
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

/**
 * Dummy function to deallocate memory used by struct
 * @param randomPointer pointer to pointer to struct
 */
void freeRandom(GaussRandom **randomPointer) {
    if (!randomPointer || !*randomPointer) return;

    free(*randomPointer);
    *randomPointer = NULL;
}