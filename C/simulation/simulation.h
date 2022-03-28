#ifndef FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define FEM_LIKE_SPREADING_MODELLING_SIMULATION_H

#include "hashTable.h"
#include "random.h"

#define SIMULATION_INI_CSV "DATA/initial.csv"
#define CSV_NAME_FORMAT "DATA/frames/frame%04i.csv"
#define MEAN    20.0
#define STDDEV  20.0


typedef struct {
    int id;
    char status;
    char timeFrame;
}citizen;

typedef struct {
    double lat;
    double lon;
    double area;
    int city_id;
    int population;
    int infected;
    hashTable *citizens;
}city;

typedef struct {
    int id;
    double distance;
}cityDistance;

typedef struct {
    city **cities;
    cityDistance **distances;
    int numberOfCities;
}country;


double computeDistanceHaversine(double latitude1, double longitude1, double latitude2, double longitude2);
double compute_distance(city *firstCity, city *secondCity);

int cmpCitiesByDistance(const void *a, const void *b);
cityDistance *createCityDistance();
void freeCityDistance(cityDistance **theCityDistance);

int interpolationSearch(double distance, int citiesSize, cityDistance **cityDistances);
void computeDistances(int cityIndex, country *theCountry);
int simulationStep(country *theCountry, GaussRandom *theGaussRandom);


country *createCountry(int numberOfCities);
city *createCity(int city_id, double area, int population, int infected, double lat, double lon);
citizen *createCitizen(int id);
void freeCountry(country **theCountry);
void freeCity(city **theCity);
void freeCitizen(citizen **theCitizen);


void *start_and_loop(void * args);

#endif //FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
