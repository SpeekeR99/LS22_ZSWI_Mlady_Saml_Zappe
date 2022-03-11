#ifndef ___C_SERV_FUNCTIONS___
#define ___C_SERV_FUNCTIONS___

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void *hello(int connfd, void *arg){
    printf("hello there!\n");
}

void *out(int connfd, void *arg){
    write(connfd, "exit", strlen("exit"));
    exit(0);
}

void *add(int connfd, void *arg){
    int i, j;
    char out[80] = {0};

    //printf((char *)arg);
    sscanf((char *)arg, "%*s %i %i", &i, &j);
    printf("%i\n", i+j);
    sprintf(out, "%i", i+j);

    write(connfd, out, sizeof(out));
}

#endif