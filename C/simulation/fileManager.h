#ifndef FEM_LIKE_SPREADING_MODELLING_CSVMANAGER_H
#define FEM_LIKE_SPREADING_MODELLING_CSVMANAGER_H

#include "simulation.h"

#define SAVE_FILEPATH "./DATA/sim_frames/save.bin"
#define PARAMETERS_FILE "./parameters.cfg"
#define LATITUDE_COLUMN_NAME "latitude"
#define LONGITUDE_COLUMN_NAME "longitude"
#define POPULATION_COLUMN_NAME "pocet_obyvatel"
#define INFECTED_COLUMN_NAME "pocet_nakazenych"
#define CITY_ID_COLUMN_NAME "kod_obce"
#define CITY_AREA_COLUMN_NAME "vymera"

extern double MOVE_STD_DEV;
extern double MOVE_MEAN;
extern double densityToAbsolute;
extern int INFECTION_TIME_MEAN;
extern int INFECTION_TIME_STD_DEV;
extern int IMMUNITY_TIME_MEAN;
extern int IMMUNITY_TIME_STD_DEV;
extern double MOVING_CITIZENS;
extern double SPREAD_MEAN;
extern double SPREAD_STD_DEV;
extern double DEATH_THRESHOLD;
extern double GO_BACK_THRESHOLD_HIGH;
extern double GO_BACK_THRESHOLD_LOW;

country *create_country_from_csv(const char *filepath, int create_citizens);
int create_csv_from_country(country *the_country, const char *filepath, int date);
int save_state(country *the_country, int date);
int load_state(country **the_country);
int load_parameters(const char *filepath);

#endif
