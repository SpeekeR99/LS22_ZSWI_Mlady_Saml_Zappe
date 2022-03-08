#ifndef ___C_SERV_FUNCTIONS___
#define ___C_SERV_FUNCTIONS___

#include <stdlib.h>
#include <stdio.h>

void *hello(int connfd, void *arg){
    printf("hello\n");
}

void *out(int connfd, void *arg){
    exit(0);
}

void *add(int connfd, void *arg){
    int i, j;

    //printf((char *)arg);
    sscanf((char *)arg, "%*s %i %i", &i, &j);
    printf("%i\n", i+j);
}

#endif