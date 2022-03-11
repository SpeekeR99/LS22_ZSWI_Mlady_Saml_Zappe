#ifndef FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define FEM_LIKE_SPREADING_MODELLING_SIMULATION_H

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
citizen *createCitizen();

void freeCountry(country **theCountry);
void freeCitizen(citizen **theCitizen);

#endif //FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
