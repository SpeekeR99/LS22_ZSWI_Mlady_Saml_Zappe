#ifndef FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define FEM_LIKE_SPREADING_MODELLING_SIMULATION_H

#include "hashTable.h"
#include "random.h"


#define NORMAL 1
#define INFECTED 2
#define RECOVERED 3
#define SIMULATION_INI_CSV "./DATA/initial.csv"
#define CSV_NAME_FORMAT "./DATA/sim_frames/frame%04d.csv"

typedef struct {
    int id;
    int homeTown;
    char status;
    char timeFrame;
}citizen;

typedef struct {
    double lat;
    double lon;
    int city_id;
    int population;
    int infected;
    double area;
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
    int movedCitizensLength;
    char *movedCitizens;
}country;


double computeDistanceHaversine(double latitude1, double longitude1, double latitude2, double longitude2);
double computeDistance(city *firstCity, city *secondCity);

int cmpCitiesByDistance(const void *a, const void *b);
cityDistance *createCityDistance();
void freeCityDistance(cityDistance **theCityDistance);

int interpolationSearch(double distance, int citiesSize, cityDistance **cityDistances);
void computeDistances(int cityIndex, country *theCountry);
void simulateDay(country *theCountry, GaussRandom *theGaussRandom, GaussRandom *theSpreadRandom);
void updateCitizenStatuses(country *theCountry);

int simulationStep(country *theCountry, GaussRandom *theMoveRandom, GaussRandom *theSpreadRandom);
int goBackHome(country *theCountry, double threshold);
int moveCitizens(country *theCountry, city *theCity, GaussRandom *moveRandom, int startIndex);

int spreadPhenomenon(country *theCountry, GaussRandom *spreadRandom);
void infectCitizensInCity(city *theCity, int toInfect);


country *createCountry(int numberOfCities);
city *createCity(int city_id, double area, int population, int infected, double lat, double lon);

citizen *createCitizen(int id, int homeTown);
void freeCountry(country **theCountry);
void freeCity(city **theCity);
void freeCitizen(citizen **theCitizen);


void *start_and_loop(void * args);

#endif //FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
