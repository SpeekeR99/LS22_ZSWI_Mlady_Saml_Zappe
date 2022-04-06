#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include "simulation.h"
#include "random.h"
#include "csvManager.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define radians(degrees) degrees * (M_PI / 180.0)

/** This function is one step of simulation where the citizens are moving between different cities
 *
 * @param theCountry initialized country
 * @param theGaussRandom gaussRandom struct with initialized mean and standard deviation
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int simulationStep(country *theCountry, GaussRandom *theGaussRandom) {
    int i;
    int j;
    int k;
    int index;
    double *doublePointer;
    city *theCity;
    arrayList *theList;
    citizen *theCitizen;

    if (!theCountry || !theGaussRandom) return EXIT_FAILURE;

    doublePointer = malloc(sizeof(double));
    if (!doublePointer) return EXIT_FAILURE;

    //go through all cities
    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];
        computeDistances(i, theCountry);
        qsort(theCountry->distances, theCountry->numberOfCities, sizeof(cityDistance *), cmpCitiesByDistance);

        //go through all citizens in a city
        for (j = 0; j < theCity->citizens->size; j++) {
            theList = theCity->citizens->array[j];

            //todo takes just 50% of citizens
            for (k = 0; k < theList->filledItems; k += 2) {
                theCitizen = (citizen *) arrayListGetPointer(theList, k);

                //something to stop one citizen to move more than once per step (will be somehow changed)
                if (theCitizen->timeFrame == -1) {
                    theCitizen->timeFrame = 0;
                    continue;
                }
                theCitizen->timeFrame = -1;

                //maybe we could delete this, what can possibly happen :)
                if (nextNormalDistDouble(theGaussRandom, doublePointer) == EXIT_FAILURE) {
                    free(doublePointer);
                    return EXIT_FAILURE;
                }

                //finds city which is the closest (not really) to the distance which citizen should travel
                index = interpolationSearch(*doublePointer, theCountry->numberOfCities, theCountry->distances);
                //todo
                index = theCountry->distances[index]->id;

                //move the citizen from one city to another
                hashTableRemoveElement(j, k, theCity->citizens);
                hashTableAddElement(theCitizen, theCitizen->id, theCountry->cities[index]->citizens);
            }
        }
    }

    //update population of cities to current state
    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCountry->cities[i]->population = theCountry->cities[i]->citizens->filledItems;
    }

    free(doublePointer);
    return EXIT_SUCCESS;
}

/**
 * This function computes distances (not with haversine, but with the faster function)
 * to all cities in theCountry from city at cityIndex and saves them in the distances
 * array
 * @param cityIndex index of the city from which the distances are calculated
 * @param theCountry wrapper of all the cities
 */
void computeDistances(int cityIndex, country *theCountry) {
    int i;
    double distance;
    city *theCity;
    if (cityIndex < 0 || !theCountry) return;

    theCity = theCountry->cities[cityIndex];

    //all cities before city at cityIndex
    for (i = 0; i < cityIndex; i++) {
        distance = compute_distance(theCity, theCountry->cities[i]);
        theCountry->distances[i]->distance = distance;
        theCountry->distances[i]->id = i;
    }

    //all cities after city at cityIndex
    for (i = cityIndex + 1; i < theCountry->numberOfCities; i++) {
        distance = compute_distance(theCity, theCountry->cities[i]);
        theCountry->distances[i]->distance = distance;
        theCountry->distances[i]->id = i;
    }

    //make city at cityIndex unreachable
    theCountry->distances[cityIndex]->distance = DBL_MAX;
    theCountry->distances[cityIndex]->id = cityIndex;
}

/**
 * Creates new country with specified numberOfCities
 * @param numberOfCities must be greater than zero
 * @return pointer to country struct or NULL if parameter was invalid or it
           wasn't possible to allocate memory
 */
country *createCountry(int numberOfCities) {
    int i;
    country *theCountry;
    if (numberOfCities <= 0) return NULL;

    theCountry = malloc(sizeof(country));
    if (!theCountry) return NULL;

    theCountry->cities = malloc(numberOfCities * sizeof(city *));
    if (!theCountry->cities) {
        free(theCountry);
        return NULL;
    }

    theCountry->distances = malloc(numberOfCities * sizeof(cityDistance *));
    for (i = 0; i < numberOfCities; i++) {
        theCountry->distances[i] = createCityDistance();
    }

    theCountry->numberOfCities = numberOfCities;

    return theCountry;
}

/**
 * Creates new city specified by parameters
 * @param city_id unique identifier, must be non-negative
 * @param population must be greater than zero
 * @param lat in degrees, must be in interval <0, 90>
 * @param lon in degrees, must be in interval <0, 180>
 * @return pointer to city struct or NULL if parameters are invalid or it is
 *         not possible to allocate memory
 */
city *createCity(int city_id, int area, int population, int infected, double lat, double lon) {
    city *theCity;
    if (population <= 0) return NULL;
    theCity = calloc(1, sizeof(city));
    if (!theCity) return NULL;

    theCity->citizens = createHashTable(population / 500, sizeof(citizen *));
    if (!theCity->citizens) {
        free(theCity);
        return NULL;
    }

    theCity->city_id = city_id;
    theCity->area = area;
    theCity->population = population;
    theCity->infected = infected;
    theCity->lat = lat;
    theCity->lon = lon;

    return theCity;
}

/**
 * Creates new citizen struct
 * @param id must be unique and greater than zero
 * @return pointer to new citizen or NULL if parameter is invalid or it is
 *         not possible to allocate memory
 */
citizen *createCitizen(int id) {
    citizen *theCitizen;
    if (id < 0) return NULL;
    theCitizen = calloc(1, sizeof(citizen));
    if (!theCitizen) return NULL;
    theCitizen->id = id;

    return theCitizen;
}

/**
 * Deallocates memory used by country struct
 * @param theCountry pointer to pointer to struct country
 */
void freeCountry(country **theCountry) {
    int i;
    if (!theCountry || !(*theCountry)) return;

    for (i = 0; i < (*theCountry)->numberOfCities; i++) {
        if ((*theCountry)->cities[i]) {
            freeCity(&(*theCountry)->cities[i]);
        }
        if ((*theCountry)->distances[i]) freeCityDistance(&(*theCountry)->distances[i]);
    }

    free((*theCountry)->cities);
    free((*theCountry)->distances);
    free(*theCountry);
    *theCountry = NULL;
}

/**
 * Deallocates memory used by city struct
 * @param theCity pointer to pointer to struct city
 */
void freeCity(city **theCity) {
    if (!theCity || !*theCity) return;

    freeHashTable(&(*theCity)->citizens);
    free(*theCity);
    *theCity = NULL;
}

/**
 * Deallocates memory used by citizen struct
 * @param theCitizen pointer to pointer to struct citizen
 */
void freeCitizen(citizen **theCitizen) {
    if (!theCitizen || !*theCitizen) return;

    free(*theCitizen);
    *theCitizen = NULL;
}

/**
 * Hardcore function to compute distance between two places on Earth (geoid), very time consuming
 * but very precise. For our needs it is a bit overkill (in this moment ☹), maybe later alligator
 * @param latitude1 in degrees, must be in interval <0, 90>
 * @param longitude1 in degrees, must be in interval <0, 180>
 * @param latitude2 in degrees, must be in interval <0, 90>
 * @param longitude2 in degrees, must be in interval <0, 180>
 * @return distance from one place to another in kilometers
 */
double computeDistanceHaversine(double latitude1, double longitude1, double latitude2, double longitude2) {

    double sinLatitude = sin(radians((latitude1 - latitude2) * 0.5));
    double cosLatitude1 = cos(radians(latitude1));
    double cosLatitude2 = cos(radians(latitude2));
    double sinLongitude = sin(radians((longitude1 - longitude2) * 0.5));
    return 6371 * 2 * asin(sqrt(sinLatitude * sinLatitude + cosLatitude1 * cosLatitude2 * sinLongitude * sinLongitude));
}

/**
 * Finds index in the array where the distance should be, if it would be placed
 * in the array
 * @param distance distance to be searched in array @param cityDistances
 * @param citiesSize greater than zero
 * @param cityDistances array of cityDistances with @param citiesSize length
 * @return index in the array where @param distance should be
 */
int interpolationSearch(double distance, int citiesSize, cityDistance **cityDistances) {
    int left;
    int middle;
    int right;

    left = 0;
    right = citiesSize - 1;
    if (cityDistances[left]->distance > distance) return 0;
    if (cityDistances[right]->distance < distance) return citiesSize - 1;

    while (cityDistances[left]->distance < distance && cityDistances[right]->distance >= distance) {
        middle = (int) (left + ((distance - cityDistances[left]->distance) * (right - left))
                               / (cityDistances[right]->distance - cityDistances[left]->distance));
        if (cityDistances[middle]->distance > distance) {
            right = middle - 1;
        } else if (cityDistances[middle]->distance < distance) {
            left = middle + 1;
        } else {
            return middle;
        }
    }

    return left;
}

/**
 * Computes distance between two cities based on geographic coordinates.
 * Can be used only on smaller distances (eg. in Czech Republic it is ok, because
 * Czech Republic is approximately 400x600 km big)
 *
 * @param firstCity not null
 * @param secondCity not null
 * @return distance from first city to second city in kilometers
 */
double compute_distance(city *firstCity, city *secondCity) {
    double coef = 110.25;
    double x = secondCity->lat - firstCity->lat;
    double y = (secondCity->lon - firstCity->lon) * cos(radians(firstCity->lat));
    return coef * sqrt(x * x + y * y);
}

/**
 * Dummy function to create new cityDistance struct
 * @return pointer to new struct or NULL if it was not possible to allocate memory
 */
cityDistance *createCityDistance() {
    cityDistance *theCityDistance;
    theCityDistance = malloc(sizeof(cityDistance));
    if (!theCityDistance) return NULL;

    return theCityDistance;
}

/**
 * Dummy function to deallocate memory used by cityDistance
 * @param theCityDistance pointer to pointer to cityDistance struct
 */
void freeCityDistance(cityDistance **theCityDistance) {
    if (!theCityDistance || !*theCityDistance) return;

    free(*theCityDistance);
    *theCityDistance = NULL;
}

/**
 * Compares two cityDistances by their distance attribute
 * @param a not null pointer to cityDistance struct
 * @param b not null pointer to cityDistance struct
 * @return -1 if a->distance is smaller than b->distance, else 1
 */
int cmpCitiesByDistance(const void *a, const void *b) {
    return (*(cityDistance **) a)->distance < (*(cityDistance **) b)->distance ? -1 : 1;
}

/**
 * @brief Initializes mandatory structs and starts the simulation, looping indefinetely
 *        This function is possible to be passed as an argument to pthread_create()
 *
 * @param args the arguments passed as an array (not used)
 * @return void* the output returned as an array (always NULL)
 */
void *start_and_loop(void * args){
    country *ctry = create_country_from_csv(SIMULATION_INI_CSV);
    GaussRandom *grand = createRandom(MEAN, STDDEV);

    /* filename: frameXXXX.csv = 13+1 chars = 14 (+1 = null term.) */
    char filename[14] = {0};

    for(int date = 0 ;; date++) {
        sprintf(filename, CSV_NAME_FORMAT, date);
        simulationStep(ctry, grand);
        create_csv_from_country(ctry, filename, date);
    }
}

