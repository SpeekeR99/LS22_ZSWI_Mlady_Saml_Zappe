#ifndef FEM_LIKE_SPREADING_MODELLING_CSVMANAGER_H
#define FEM_LIKE_SPREADING_MODELLING_CSVMANAGER_H

#include "simulation.h"

#define SAVE_FILEPATH "./DATA/frames/save.bin"
#define LATITUDE_COLUMN_NAME "latitude"
#define LONGITUDE_COLUMN_NAME "longitude"
#define POPULATION_COLUMN_NAME "pocet_obyvatel"
#define INFECTED_COLUMN_NAME "pocet_nakazenych"
#define CITY_ID_COLUMN_NAME "kod_obce"
#define CITY_AREA_COLUMN_NAME "vymera"

country *create_country_from_csv(const char *filepath, int create_citizens);
int create_csv_from_country(country *the_country, const char *filepath, int date);
int save_state(country *the_country, int date);
int load_state(country *the_country);

#endif
