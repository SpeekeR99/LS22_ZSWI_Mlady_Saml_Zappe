//
// Created by Kryst on 16.03.2022.
//

#ifndef FEM_LIKE_SPREADING_MODELLING_RANDOM_H
#define FEM_LIKE_SPREADING_MODELLING_RANDOM_H

#define stupidName 0.000061037018951994385

typedef struct {
    char hasNextValue;
    double nextValue;
    double mean;
    double stdDev;
}GaussRandom;

double randomDouble();

int randomGaussian(GaussRandom *randomPointer, double *doublePointer);

int nextNormalDistDouble(GaussRandom *randomPointer, double *doublePointer);

int nextNormalDistDoubleFaster(GaussRandom *randomPointer, double *doublePointer);

GaussRandom *createRandom(double mean, double stdDev);

void freeRandom(GaussRandom **randomPointer);

#include "hashTable.h"

#endif //FEM_LIKE_SPREADING_MODELLING_RANDOM_H
