#ifndef ___C_SERV_FUNCTIONS___
#define ___C_SERV_FUNCTIONS___

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../simulation.h"

void *out(int connfd, void *arg){
    write(connfd, "exit", strlen("exit"));
    exit(0);
}

void *start_simulation(int connfd, void *arg){

}

void *send_data_from_simulation(int connfd, void *arg){
    char bff[128] = {0};
    sprintf(bff,"Naceradec,530212,49.610286,14.906434,%d,256",rand());
    write(connfd, bff, strlen(bff));
}

#endif