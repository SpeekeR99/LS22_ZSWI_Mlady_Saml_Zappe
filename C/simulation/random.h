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

int randomGaussian(anonymous struct *randomPointer, double *doublePointer);

int nextNormalDistDouble(anonymous struct *randomPointer, double *doublePointer);

int nextNormalDistDoubleFaster(anonymous struct *randomPointer, double *doublePointer);

anonymous struct *createRandom(double mean, double stdDev);

void freeRandom(anonymous struct **randomPointer);

#include "hashTable.h"

#endif //FEM_LIKE_SPREADING_MODELLING_RANDOM_H
