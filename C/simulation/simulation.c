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

#define radians(degrees) (degrees) * (M_PI / 180.0)
//number from <0,1) determines how many citizens will citizen meet
#define densityToAbsolute 0.03

#define INFECTION_TIME_IN_DAYS 14
#define IMMUNITY_AFTER_INFECTION_IN_DAYS 30

#define SPREAD_MEAN 0.05
#define SPREAD_STD_DEV 0.02

/**
 * Simulates a day of the simulation (one step is one hour of "real time")
 * Calls simulationStep every hour
 * Every 8 hours goBackHome function is called with threshold of 0.8
 * Other hours the threshold is set to 0.05
 * Calls update_citizen_statuses after each day
 *
 * @param theCountry initialized country
 * @param theGaussRandom gaussRandom struct with initialized mean and standard deviation
 * @param theSpreadRandom gaussRandom struct with initialized mean and standard deviation
 */
void simulate_day(country *theCountry, GaussRandom *theGaussRandom, GaussRandom *theSpreadRandom) {
    int hour;
    for (hour = 0; hour < 24; hour++) {
        simulationStep(theCountry, theGaussRandom, theSpreadRandom);
        if (hour % 8 == 0) goBackHome(theCountry, 0.8);
        else goBackHome(theCountry, 0.05);
    }
    update_citizen_statuses(theCountry);
}

/**
 * If the citizen is either infected or cured, their timeFrame gets incremented
 * Every infected citizen has a chance of dying (being removed from the hashTable)
 * If an infected citizen survived 14 days, he becomes cured
 * If cured citizen is recovered for more than 30 days, he becomes infect-able again
 *
 * @param theCountry initialized country
 */
void update_citizen_statuses(country *theCountry) {
    double death_chance;
    int i, j, k;
    city *theCity;
    arrayList *theList;
    citizen *theCitizen;

    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];

        for (j = 0; j < theCity->citizens->size; j++) {
            theList = theCity->citizens->array[j];

            for (k = 0; k < theList->filledItems; k++) {
                theCitizen = arrayListGetPointer(theList, k);

                // if citizen is either infected or cured, increment days infected (or cured)
                if (theCitizen->status != 1)
                    theCitizen->timeFrame++;

                // infected citizen
                if (theCitizen->status == 2) {
                    // if the citizen is infected, there is a chance he will die
                    death_chance = (double) rand() / RAND_MAX;
                    if (death_chance < 0.001) // todo fiddle around with this magic number
                        hashTableRemoveElement(j, k, theCity->citizens);
                    // if the citizen was infected for 14 days, he is cured now
                    if (theCitizen->timeFrame == INFECTION_TIME_IN_DAYS)
                        theCitizen->status = 3;
                }

                // if the citizen is cured for 30 days, he can be re-infected again
                if (theCitizen->status == 3 && theCitizen->timeFrame == IMMUNITY_AFTER_INFECTION_IN_DAYS)
                    theCitizen->status = 1;
            }
        }
    }
}

/** This function is one step of simulation where the citizens are moving between different cities
 *
 * @param theCountry initialized country
 * @param theGaussRandom gaussRandom struct with initialized mean and standard deviation
 * @param theSpreadRandom gaussRandom struct with initialized mean and standard deviation
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int simulationStep(country *theCountry, GaussRandom *theGaussRandom, GaussRandom *theSpreadRandom) {
    int i;
    city *theCity;

    if (!theCountry || !theGaussRandom) return EXIT_FAILURE;

    //go through all cities
    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];
        computeDistances(i, theCountry);
        qsort(theCountry->distances, theCountry->numberOfCities, sizeof(cityDistance *), cmpCitiesByDistance);

        moveCitizens(theCountry, theCity, theGaussRandom);
    }
    resetCitizenStatuses(theCountry);

    spreadPhenomenon(theCountry, theSpreadRandom);

    //update population of cities to current state
    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCountry->cities[i]->population = theCountry->cities[i]->citizens->filledItems;
    }

    return EXIT_SUCCESS;
}

/**
 * Resets statuses for citizens which have traveled
 * @param country
 */
void resetCitizenStatuses(country *country) {
    int i;
    int j;
    int k;
    city *theCity;
    arrayList *theList;
    citizen *theCitizen;
    if (!country) return;

    //go through all citizens
    for (i = 0; i < country->numberOfCities; i++) {
        theCity = country->cities[i];

        for (j = 0; j < theCity->citizens->size; j++) {
            theList = theCity->citizens->array[j];

            for (k = 0; k < theList->filledItems; k++) {
                theCitizen = arrayListGetPointer(theList, k);
                theCitizen->status = -theCitizen->status;
            }
        }
    }

}

/**
 * Function computes how many people will be infected in all cities,
 * calls @function infectCitizensInCity to infect citizens
 * @param theCountry country with cities
 * @param random GaussRandom set up with spreading probabilities
 *        otherwise can fall into infinite loop
 * @return EXIT_SUCCESS or EXIT_FAILURE in case of invalid parameters
 *         or if it's not possible to allocate memory
 */
int spreadPhenomenon(country *theCountry, GaussRandom *random) {
    int i;
    int j;
    double populationDensity;
    city *theCity;
    double *doublePointer;
    int toInfect;

    doublePointer = malloc(sizeof(double));
    if (!theCountry || !random || !doublePointer) return EXIT_FAILURE;

    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];
        populationDensity = (double) theCity->population / theCity->area;
        toInfect = 0;

        //compute how many people will be infected in this city
        for (j = 0; j < theCity->infected; j++) {
            //we need only numbers in interval <0,1>
            do {
                nextNormalDistDouble(random, doublePointer);
            } while (*doublePointer < 0 || *doublePointer > 1);

            toInfect += (int)(*doublePointer * populationDensity * densityToAbsolute);
        }

        infectCitizensInCity(theCity, toInfect);
    }

    free(doublePointer);
    return EXIT_SUCCESS;
}

/**
 * Function performs infecting of citizens in selected city
 * @param theCity where citizens will be infected
 * @param toInfect total number of citizens to be infected
 */
void infectCitizensInCity(city *theCity, int toInfect) {
    int i;
    int listIndex;
    int maxListIndex;
    int citizenIndex;
    citizen *theCitizen;
    if (!theCity || toInfect < 0) return;

    for (i = 0; i < toInfect; i++) {
        listIndex = (int) ((double) rand() / RAND_MAX) * theCity->citizens->size;
        maxListIndex = theCity->citizens->array[listIndex]->filledItems;
        citizenIndex = (int) ((double) rand() / RAND_MAX) * maxListIndex;

        theCitizen = arrayListGetPointer(theCity->citizens->array[listIndex], citizenIndex);

        //already infected citizen
        if (!theCitizen || theCitizen->status == 2) continue;

        theCitizen->status = 2;
        theCitizen->timeFrame = 0;
        theCity->infected++;
    }
}

/**
 * Function preforms moving of 50% of citizens in the country, citizents travel from some city
 * to another randomly selected city
 *
 * @param theCountry initialized country
 * @param theCity current city
 * @param theGaussRandom gaussRandom struct with initialized mean and standard deviation

 * @return EXIT_SUCCESS or EXIT_FAILURE in case of invalid parameters or if it is not possible
 *         to allocate memory
 */
int moveCitizens(country *theCountry, city *theCity, GaussRandom *theGaussRandom) {
    int j;
    int k;
    int index;
    double *doublePointer;
    arrayList *theList;
    citizen *theCitizen;

    if (!theCountry || !theCity || !theGaussRandom) return EXIT_FAILURE;

    doublePointer = malloc(sizeof(double));
    if (!doublePointer) return EXIT_FAILURE;

    //go through all citizens in a city
    for (j = 0; j < theCity->citizens->size; j++) {
        theList = theCity->citizens->array[j];

        //todo takes just 50% of citizens
        for (k = 0; k < theList->filledItems; k += 2) {
            theCitizen = (citizen *) arrayListGetPointer(theList, k);

            if (theCitizen->status < 0) continue;
            theCitizen->status = -theCitizen->status;
            //something to stop one citizen to move more than once per step (will be somehow changed)
//            if (theCitizen->timeFrame == -1) {
//                theCitizen->timeFrame = 0;
//                continue;
//            }
//            theCitizen->timeFrame = -1;

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

    free(doublePointer);
    return EXIT_SUCCESS;
}

/**
 * Processes return of selected percent of citizens to their hometowns
 * percent of citizens which return home can be changed by changing goBackThreshold macro
 * @param theCountry non-null pointer to country struct
 * @param threshold number from <0,1) determines how many citizen will return to their hometown
 * @return EXIT_SUCCESS or EXIT_FAILURE if country pointer is invalid
 */
int goBackHome(country *theCountry, double threshold) {
    int i;
    int j;
    int k;
    double returnChance;
    city *theCity;
    arrayList *theList;
    citizen *theCitizen;

    if (!theCountry) return EXIT_FAILURE;

    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];

        //go through all citizens in a city
        for (j = 0; j < theCity->citizens->size; j++) {
            theList = theCity->citizens->array[j];

            for (k = 0; k < theList->filledItems; k++) {
                theCitizen = (citizen *) arrayListGetPointer(theList, k);

                //citizen is not in his hometown
                if (i != theCitizen->homeTown) {

                    //value from <0,1) if smaller than threshold, citizen moves to his hometown
                    returnChance = (double) rand() / RAND_MAX;
                    if (returnChance < threshold) {
                        //move the citizen from this city to his hometown
                        hashTableRemoveElement(j, k, theCity->citizens);
                        hashTableAddElement(theCitizen, theCitizen->id, theCountry->cities[theCitizen->homeTown]->citizens);
                    }
                }
            }
        }
    }

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
 * @param homeTown index of city where citizen is from
 * @return pointer to new citizen or NULL if parameter is invalid or it is
 *         not possible to allocate memory
 */
citizen *createCitizen(int id, int homeTown) {
    citizen *theCitizen;
    if (id < 0) return NULL;
    theCitizen = calloc(1, sizeof(citizen));
    if (!theCitizen) return NULL;
    theCitizen->id = id;
    theCitizen->homeTown = homeTown;
    theCitizen->status = 1;

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
    if(!ctry){
        fprintf(stderr, "Error: Could not create country from ini csv file\n");
        return NULL;
    }
    GaussRandom *grand = createRandom(MEAN, STDDEV);
    GaussRandom *spreadRandom = createRandom(SPREAD_MEAN, SPREAD_STD_DEV);

    /* filename: frameXXXX.csv = 13+1 chars = 14 (+1 = null term.) */
    char filename[40] = {0};

    for(int date = 0 ;; date++) {
        sprintf(filename, CSV_NAME_FORMAT, date);
        simulate_day(ctry, grand, spreadRandom);
        create_csv_from_country(ctry, filename, date);
    }
}

