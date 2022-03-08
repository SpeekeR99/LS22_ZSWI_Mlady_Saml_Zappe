//
// Created by Kryst on 07.03.2022.
//

#ifndef ZSWI_RANDOMGAUSSIAN_H
#define ZSWI_RANDOMGAUSSIAN_H
#define stupidName 0.000061037018951994385

typedef struct {
    char hasNextValue;
    double nextValue;
    double mean;
    double stdDev;
}GaussRandom;


GaussRandom *createRandom(double mean, double stdDev);
double randomGaussian(GaussRandom *randomPointer);
double randomDouble();
double nextNormalDistDouble(GaussRandom *randomPointer);
double nextNormalDistDouble2(GaussRandom *randomPointer);
double nextNormalDistDoubleFaster(GaussRandom *randomPointer);
void freeRandom(GaussRandom **randomPointer);
double computeDistance(double latitude1, double longitude1, double latitude2, double longitude2);

#endif //ZSWI_RANDOMGAUSSIAN_H
