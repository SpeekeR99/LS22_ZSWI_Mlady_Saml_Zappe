#ifndef FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define stupidName 0.000061037018951994385

typedef struct {
    char hasNextValue;
    double nextValue;
    double mean;
    double stdDev;
}GaussRandom;


GaussRandom *createRandom(double mean, double stdDev);
int randomGaussian(GaussRandom *randomPointer, double *doublePointer);
double randomDouble();
int nextNormalDistDouble(GaussRandom *randomPointer, double *doublePointer);
int nextNormalDistDoubleFaster(GaussRandom *randomPointer, double *doublePointer);
void freeRandom(GaussRandom **randomPointer);
double computeDistanceHaversine(double latitude1, double longitude1, double latitude2, double longitude2);

typedef struct {
    short cityID;
    char status;
    char timeFrame;
}citizen;


typedef struct {
    int *citiesPopulation;
    int numberOfCities;
    citizen **citizens;
    int numberOfCitizens;
}country;


country *createCountry(int numberOfCities, int numberOfCitizens);
citizen *createCitizen(int cityID);

void freeCountry(country **theCountry);
void freeCitizen(citizen **theCitizen);

#endif //FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
