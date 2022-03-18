#ifndef ___C_SERV_FUNCTIONS___
#define ___C_SERV_FUNCTIONS___

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../simulation/simulation.h"

#define SEND_MAX_SIZE 4194304

char SIM_STARTED = 0;

void *out(int connfd, void *arg){
    write(connfd, "exit", strlen("exit"));
    exit(0);
}

/**
 * @brief Starts the simulation in another thread. The simulaition can be started only once!
 *        for more info, read description of the start_and_loop function in simulation.c
 * 
 * @param connfd the connection file descriptor (unused)
 * @param arg    unused
 * @return void* pointer to the thread id of the constructed thread
 */
void *start_simulation(int connfd, void *arg){
    if(SIM_STARTED) return NULL;

    pthread_t tid;

    pthread_create(&tid, NULL, start_and_loop,NULL);

    SIM_STARTED = 1;

    return &tid;

}

/**
 * @brief Sends the CSV data from the simulation. 
 * The CSV is on a path specified by the SIMULATION_CSV_FILE constant defined at the top of this source file
 * The CSV is sent as 4 MB* chunks of text until the end of the file - so the client might recieve more messages (if the csv is more than 4 MB)
 * After the CSV is completely sent, server will send a string with only a char with the value 4 ('\x04' - ascii character for end of transmission)
 * and then, the command is done
 * 
 * *4 MB = 4194304
 * 
 * @param connfd connection descriptor
 * @param arg    pointer to the arguments string as if passed in command line - "<command_name> <arg1>",
 *               <arg1> being the number of frame to send
 * @return NULL
 */
void *send_data_from_simulation(int connfd, void *arg){
    char *bff = malloc(SEND_MAX_SIZE * sizeof(char));

    int frame = 0;
    sscanf((const char *)arg, "%*s %d", &frame);

    FILE *csv = fopen(SIMULATION_INI_CSV, "r");
    int i = 0, next;
    
    /* go through all characters of the csv, filling the buffer
       if buffer is full, send all its data, free it and continue from begining
     */
    while((next = fgetc(csv)) != EOF){
        bff[i] = (char)next;

        if(i == SEND_MAX_SIZE){
            write(connfd,bff,SEND_MAX_SIZE);
            bzero(bff, SEND_MAX_SIZE);
            i = -1; /* next line of code will set this to 0 */
        }
        i++;
    }
    /* write the final part and then the end of transmission */
    write(connfd,bff,SEND_MAX_SIZE);
    write(connfd, "\x04",1);

    free(bff);
    return NULL;
}

#endif