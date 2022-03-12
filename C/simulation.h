#ifndef FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
#define stupidName 0.000061037018951994385
#define LATITUDE_COLUMN_NAME "latitude"
#define LONGITUDE_COLUMN_NAME "longitude"
#define POPULATION_COLUMN_NAME "pocet_obyvatel"

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
double compute_distance(double latitude1, double longitude1, double latitude2, double longitude2);


typedef struct {
    short cityID;
    char status;
    char timeFrame;
}citizen;

typedef struct {
    double lat;
    double lon;
    int population;
}city;

typedef struct {
    city **cities;
    int numberOfCities;
    citizen **citizens;
    int numberOfCitizens;
}country;


country* create_country_from_csv(const char *filepath);
country *createCountry(int numberOfCities, int numberOfCitizens);
city *createCity(int population, double lat, double lon);
citizen *createCitizen(short cityID);
void freeCountry(country **theCountry);
void freeCity(city **theCity);
void freeCitizen(citizen **theCitizen);

#endif //FEM_LIKE_SPREADING_MODELLING_SIMULATION_H
