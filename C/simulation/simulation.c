#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "simulation.h"
#include "random.h"
#include "fileManager.h"

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define radians(degrees) (degrees) * (M_PI / 180.0)

double MOVE_STD_DEV;
double MOVE_MEAN;
double MEETING_FACTOR;
int INFECTION_TIME_MEAN;
int INFECTION_TIME_STD_DEV;
int IMMUNITY_TIME_MEAN;
int IMMUNITY_TIME_STD_DEV;
double MOVING_CITIZENS;
double SPREAD_MEAN;
double SPREAD_STD_DEV;
double DEATH_THRESHOLD;
double GO_BACK_THRESHOLD_HIGH;
double GO_BACK_THRESHOLD_LOW;

/**
 * Simulates a day of the simulation (one step is one hour of "real time")
 * Calls simulationStep every hour
 * Calls updateCitizenStatuses after each day
 *
 * @param theCountry initialized country
 * @param theGaussRandom gaussRandom struct with initialized mean and standard deviation
 * @param theSpreadRandom gaussRandom struct with initialized mean and standard deviation
 */
void simulateDay(country *theCountry, GaussRandom *theGaussRandom, GaussRandom *theSpreadRandom) {
    int hour;
    for (hour = 0; hour < 24; hour++) {
        simulationStep(theCountry, theGaussRandom, theSpreadRandom);
        if ((hour + 1) % 8 == 0) goBackHome(theCountry, GO_BACK_THRESHOLD_HIGH);
        else goBackHome(theCountry, GO_BACK_THRESHOLD_LOW);
    }
    updateCitizenStatuses(theCountry);
}

/**
 * If the citizen is either infected or cured, their timeFrame gets incremented
 * Every infected citizen has a chance of dying (being removed from the hashTable)
 * If an infected citizen survived 14 days, he becomes cured
 * If cured citizen is recovered for more than 30 days, he becomes infect-able again
 *
 * @param theCountry initialized country
 */
void updateCitizenStatuses(country *theCountry) {
    double deathChance;
    int i, j, k;
    city *theCity;
    arrayList *theList;
    citizen *theCitizen;
    GaussRandom *infectedRandom;
    GaussRandom *immunityRandom;
    double *randomDate;

    if (!theCountry) return;

    infectedRandom = createRandom(INFECTION_TIME_MEAN, INFECTION_TIME_STD_DEV);

    if (!infectedRandom) return;

    immunityRandom = createRandom(IMMUNITY_TIME_MEAN, IMMUNITY_TIME_STD_DEV);

    if (!immunityRandom) {
        freeRandom(&infectedRandom);
        return;
    }

    randomDate = malloc(sizeof(double));

    if (!randomDate) {
        freeRandom(&infectedRandom);
        freeRandom(&immunityRandom);
        return;
    }


    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];

        for (j = 0; j < theCity->citizens->size; j++) {
            theList = theCity->citizens->array[j];

            for (k = 0; k < theList->filledItems; k++) {
                theCitizen = arrayListGetPointer(theList, k);

                // if citizen is either infected or cured, increment days infected (or cured)
                if (theCitizen->status != NORMAL) {
                    theCitizen->timeFrame++;

                    // infected citizen
                    if (theCitizen->status == INFECTED) {

                        // if the citizen is infected, there is a chance he will die
                        deathChance = (double) rand() / RAND_MAX;
                        if (deathChance < DEATH_THRESHOLD) {
                            hashTableRemoveElement(j, k, theCity->citizens);
                            freeCitizen(&theCitizen);
                            theCity->infected--;
                            theCity->population--;
                            continue;
                        }

                        nextNormalDistDouble(infectedRandom, randomDate);
                        // if the citizen was infected for 14 days, he is cured now
                        if (theCitizen->timeFrame >= *randomDate) {
                            theCitizen->status = RECOVERED;
                            theCity->infected--;
                            theCitizen->timeFrame = 0;
                            continue;
                        }
                    }

                    // if the citizen is cured for 30 days, he can be re-infected again
                    nextNormalDistDouble(immunityRandom, randomDate);
                    if (theCitizen->status == RECOVERED && theCitizen->timeFrame >= *randomDate) {
                        theCitizen->status = NORMAL;
                        theCitizen->timeFrame = 0;
                    }
                }
            }
        }
    }
    freeRandom(&infectedRandom);
    freeRandom(&immunityRandom);
    free(randomDate);
}

/** This function is one step of simulation where the citizens are moving between different cities
 *
 * @param theCountry initialized country
 * @param theMoveRandom gaussRandom struct with initialized mean and standard deviation
 * @param theSpreadRandom gaussRandom struct with initialized mean and standard deviation
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int simulationStep(country *theCountry, GaussRandom *theMoveRandom, GaussRandom *theSpreadRandom) {
    int i;
    city *theCity;

    if (!theCountry || !theMoveRandom || !theSpreadRandom) return EXIT_FAILURE;

    memset(theCountry->movedCitizens, 0, theCountry->movedCitizensLength * sizeof(char));
    int startIndex = 0;

    //go through all cities
    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];
        computeDistances(i, theCountry);
        qsort(theCountry->distances, theCountry->numberOfCities, sizeof(cityDistance *), cmpCitiesByDistance);

        startIndex = moveCitizens(theCountry, theCity, theMoveRandom, startIndex);
        if (startIndex == -1) return EXIT_FAILURE;
    }

    spreadPhenomenon(theCountry, theSpreadRandom);

    return EXIT_SUCCESS;
}

/**
 * Function computes how many people will be infected in all cities,
 * calls @function infectCitizensInCity to infect citizens
 * @param theCountry country with cities
 * @param spreadRandom GaussRandom set up with spreading probabilities
 *        otherwise can fall into infinite loop
 * @return EXIT_SUCCESS or EXIT_FAILURE in case of invalid parameters
 *         or if it's not possible to allocate memory
 */
int spreadPhenomenon(country *theCountry, GaussRandom *spreadRandom) {
    int i;
    int j;
    double populationDensity;
    city *theCity;
    double *spreadChance;
    int toInfect;

    if (!theCountry || !spreadRandom) return EXIT_FAILURE;

    spreadChance = malloc(sizeof(double));
    if (!spreadChance) return EXIT_FAILURE;

    for (i = 0; i < theCountry->numberOfCities; i++) {
        theCity = theCountry->cities[i];
        populationDensity = (double) theCity->population / theCity->area;
        toInfect = 0;

        //compute how many people will be infected in this city
        for (j = 0; j < theCity->infected; j++) {
            //we need only numbers in interval <0,1>
            do {
                nextNormalDistDouble(spreadRandom, spreadChance);
            } while (*spreadChance < 0 || *spreadChance > 1);

            toInfect += (int)(*spreadChance * populationDensity * MEETING_FACTOR);
        }

        infectCitizensInCity(theCity, toInfect);
    }

    free(spreadChance);
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
        listIndex = (int) ((double) rand() / RAND_MAX) * (theCity->citizens->size - 1);
        maxListIndex = theCity->citizens->array[listIndex]->filledItems;
        citizenIndex = (int) ((double) rand() / RAND_MAX) * (maxListIndex - 1);

        theCitizen = arrayListGetPointer(theCity->citizens->array[listIndex], citizenIndex);

        //already infected or cured citizen
        if (!theCitizen || theCitizen->status > 1) continue;

        theCitizen->status = INFECTED;
        theCitizen->timeFrame = 0;
        theCity->infected++;
    }
}

/**
 * Function preforms moving some percentage (defined by MOVING_CITIZENS of citizens in the country,
 * citizens travel from some city to another randomly selected city
 *
 * @param theCountry initialized country
 * @param theCity current city
 * @param moveRandom gaussRandom struct with initialized mean and standard deviation
 * @param startIndex index at which should moving start at, it is there because of parameterized
 *                   looping through citizens

 * @return startIndex for next city or -1 in case of invalid parameters or if it is not possible
 *         to allocate memory
 */
int moveCitizens(country *theCountry, city *theCity, GaussRandom *moveRandom, int startIndex) {
    int j;
    int k;
    int index;
    double *moveDistance;
    arrayList *theList;
    citizen *theCitizen;

    if (!theCountry || !theCity || !moveRandom) return -1;

    moveDistance = malloc(sizeof(double));
    if (!moveDistance) return -1;

    int moving = (int) (1.0 / MOVING_CITIZENS);
    k = startIndex;

    //go through all citizens in a city
    for (j = 0; j < theCity->citizens->size; j++) {
        theList = theCity->citizens->array[j];

        for (; k < theList->filledItems; k += moving) {

            theCitizen = (citizen *) arrayListGetPointer(theList, k);

            //citizen has moved already
            if (theCountry->movedCitizens[theCitizen->id] == 1) continue;

            //this citizen will be moved, flag which notifies about that
            theCountry->movedCitizens[theCitizen->id] = 1;

            //maybe we could delete this, what can possibly happen :)
            if (nextNormalDistDouble(moveRandom, moveDistance) == EXIT_FAILURE) {
                free(moveDistance);
                return -1;
            }

            //finds city which is the closest (not really) to the distance which citizen should travel
            index = interpolationSearch(ABS(*moveDistance), theCountry->numberOfCities, theCountry->distances);
            //todo
            index = theCountry->distances[index]->id;

            //if citizen is infected, counters must be updated
            if (theCitizen->status == INFECTED) {
                theCity->infected--;
                theCountry->cities[index]->infected++;
            }

            //move the citizen from one city to another
            hashTableRemoveElement(j, k, theCity->citizens);
            hashTableAddElement(theCitizen, theCitizen->id, theCountry->cities[index]->citizens);
            theCity->population--;
            theCountry->cities[index]->population++;
            //one citizen was removed, we need to decrement k, because otherwise
            // we will skip one item in arrayList
            k--;
        }
        k -= theList->filledItems;
    }

    free(moveDistance);
    return k;
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
                    if (returnChance <= threshold) {

                        //if citizen is infected, counters must be updated
                        if (theCitizen->status == INFECTED) {
                            theCity->infected--;
                            theCountry->cities[theCitizen->homeTown]->infected++;
                        }

                        //move the citizen from actual city to his hometown
                        hashTableRemoveElement(j, k, theCity->citizens);
                        hashTableAddElement(theCitizen, theCitizen->id, theCountry->cities[theCitizen->homeTown]->citizens);
                        theCity->population--;
                        theCountry->cities[theCitizen->homeTown]->population++;
                        //one citizen was removed, we need to decrement k, because otherwise
                        // we will skip one item in arrayList
                        k--;
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
        distance = computeDistance(theCity, theCountry->cities[i]);
        theCountry->distances[i]->distance = distance;
        theCountry->distances[i]->id = i;
    }

    //all cities after city at cityIndex
    for (i = cityIndex + 1; i < theCountry->numberOfCities; i++) {
        distance = computeDistance(theCity, theCountry->cities[i]);
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

    theCountry->cities = calloc(numberOfCities, sizeof(city *));

    if (!theCountry->cities) {
        free(theCountry);
        return NULL;
    }

    theCountry->distances = calloc(numberOfCities, sizeof(city *));

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
city *createCity(int city_id, double area, int population, int infected, double lat, double lon) {
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
    theCitizen->status = NORMAL;

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
    free((*theCountry)->movedCitizens);
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
double computeDistance(city *firstCity, city *secondCity) {
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
void *start_and_loop(void * args) {
    FILE *fp = NULL;
    country *ctry = NULL;
    clock_t start, end;
    int date = 0;

    fp = fopen(SAVE_FILEPATH, "rb");
    if (fp) {
        fclose(fp);
        ctry = create_country_from_csv(SIMULATION_INI_CSV, 0);
        start = clock();
        date = load_state(&ctry) + 1;
        end = clock();
        printf("Loaded state from frame %d successfully in %f sec.\n", date - 1, ((double)(end-start))/CLOCKS_PER_SEC);
    }
    else {
        ctry = create_country_from_csv(SIMULATION_INI_CSV, 1);
        printf("Starting the simulation from scratch.\n");
    }
    srand(time(NULL));

    if(!ctry){
        fprintf(stderr, "Error: Could not create country from ini csv file\n");
        return NULL;
    }
    if (load_parameters(PARAMETERS_FILE) == EXIT_FAILURE) {
        fprintf(stderr, "Error: Could not load parameters from parameters.cfg file\n");
        return NULL;
    }
    GaussRandom *moveRandom = createRandom(MOVE_MEAN, MOVE_STD_DEV);
    GaussRandom *spreadRandom = createRandom(SPREAD_MEAN, SPREAD_STD_DEV);

    /* filename: frameXXXX.csv = 13+1 chars = 14 (+1 = null term.) */
    char filename[40] = {0};

    for(;; date++) {
        start = clock();

        sprintf(filename, CSV_NAME_FORMAT, date);
        simulateDay(ctry, moveRandom, spreadRandom);
        create_csv_from_country(ctry, filename, date);

        end = clock();
        printf("Loop %i done in %f sec.\n",date, ((double)(end-start))/CLOCKS_PER_SEC);
        save_state(ctry, date);
        printf("Saved current state successfully.\n");
    }
}

