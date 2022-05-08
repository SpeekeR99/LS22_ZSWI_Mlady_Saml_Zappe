#ifndef ___C_SERV_FUNCTIONS___
#define ___C_SERV_FUNCTIONS___

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../simulation/simulation.h"

#define SEND_MAX_SIZE 4194304
#define NO_DATA_MESSAGE "no data\x04"

char SIM_STARTED = 0;

void *out(int connfd, void *arg) {
    write(connfd, "exit\x04", strlen("exit\x04"));
    exit(0);
}

/**
 * @brief Starts the simulation in another thread. The simulaition can be started only once!
 *        for more info, read description of the start_and_loop function in simulation.c
 * 
 * @param connfd the connection file descriptor (unused)
 * @param arg    unused
 * @return void* pointer to the thread id of the constructed thread or NULL if the simulation already running
 */
void *start_simulation(int connfd, void *arg) {
    if (SIM_STARTED) {
        printf("Simulation already running\n");
        return NULL;
    }

    pthread_t tid;

    pthread_create(&tid, NULL, start_and_loop, NULL);

    SIM_STARTED = 1;

    return NULL;

}

/**
 * @brief Sends the CSV data from the simulation. 
 * The CSV is on a path specified by the CSV_NAME_FORMAT constant defined in simulation.h
 * The first argument of this command is taken as the number of frame and sprintf'd into the format.
 * The CSV is sent as 4 MB* chunks of text until the end of the file - so the client might recieve more messages (if the csv is more than 4 MB)
 * After the CSV is completely sent, server will send a string with only a char with the value 4 ('\x04' - ascii character for end of transmission)
 * and then, the command is done
 * 
 * *4 MB = 4194304 B
 * 
 * @param connfd connection descriptor
 * @param arg    pointer to the arguments string as if passed in command line - "<command_name> <arg1>",
 *               <arg1> being the number of frame to send
 * @return NULL
 */
void *send_data_from_simulation(int connfd, void *arg) {
    char *bff = malloc(SEND_MAX_SIZE * sizeof(char));

    int frame = 0;
    sscanf((const char *) arg, "%*s %d", &frame);

    /* debug
    printf((const char *)arg);
    printf("\n");
    printf("frame: %i\n", frame);
    return;*/

    char fname[40] = {0};
    sprintf(fname, CSV_NAME_FORMAT, frame);

    printf("Sending data from %s\n", fname);

    FILE *csv;
    if (!(csv = fopen((const char *) fname, "r"))) {
        write(connfd, NO_DATA_MESSAGE, strlen(NO_DATA_MESSAGE));
        return NULL;
    }


    int i = 0, next;

    /* go through all characters of the csv, filling the buffer
       if buffer is full, send all its data, free it and continue from begining
     */
    while ((next = fgetc(csv)) != EOF) {
        bff[i] = (char) next;

        if (i == SEND_MAX_SIZE) {
            write(connfd, bff, SEND_MAX_SIZE);
            bzero(bff, SEND_MAX_SIZE);
            i = -1; /* next line of code will set this to 0 */
        }
        i++;
    }
    fclose(csv);
    /* write the final part and then the end of transmission */
    write(connfd, bff, strlen(bff));
    write(connfd, "\x04", 1);


    //FILE *dbg = fopen("c_dbg.log","w");
    //fprintf(dbg,"%s",bff);
    //fclose(dbg);

    free(bff);
    return NULL;
}

#endif