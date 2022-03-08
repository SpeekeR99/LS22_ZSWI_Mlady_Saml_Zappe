/* ------- INCLUDES, GENERAL CONSTANTS  */
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "serv_functions.h"

#define DEF_IP "127.0.0.1"
#define DEF_PORT 4242

/*-------- PROGRAM ARGUMENTS */

#define REQ_ARGNUM 2
#define ARGNUM 2
#define MIN_ARGNUM 1 + 2*REQ_ARGNUM
/* all possible switches on commandline */
char *available_args[ARGNUM] = {"-port", "-ip4"};

/* -------- COMMANDS TO THE PROGRAM */

#define CMDNUM 3
/* The array of commands */
char *cmds[CMDNUM] = {"hello","add", "out"};
/* The array of functions invoked by commands
    The functions return void * if they return anything and accept 
    the conffd and additional args stored as void * 
    functions are defined in a separate file 
    (serv_function.h in this case) */
void *(*cmd_fns[CMDNUM])(int, void *) = {&hello,&add, &out};

/* -------- CODE SECTION */

/**
 * @brief Creates a listening socket on supplied IP and port and returns its sockfd
 * 
 * @param IP The IP address to listen on
 * @param port The port to listen on
 * @return The sockfd (int) of the listening socket, ready to accept a client
 * The queue of pending connections is of length 5, but only 1 client should request communication
 */
int create_listen_socket(char *IP, int port){
    /* Code by Yogesh Shukla et al. on GeeksforGeeks.org */
    /* https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/ */


    int sockfd;
    struct sockaddr_in servaddr;
   
    /* socket create and verification */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    memset(&servaddr, 0, sizeof(servaddr));   
    /* assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(port);
   
    /* Binding newly created socket to given IP and verification */
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded to %s:%d\n", IP,port);
   
    /* Now server is ready to listen and verification */
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    return sockfd;
}

/**
 * @brief Establishes connection between the server and incomming client request
 * 
 * @param sockfd The sockfd of a listening socket (the server)
 * @return The sockfd of the connection, through which the server and the client communicate
 * After this function, the server can read() from and write() to the client (using the returned sockfd)
 */
int create_connection(int sockfd){
    int connfd, len;
    struct sockaddr_in cli;
    len = sizeof(cli);

    /* Accept the data packet from client and verification
       This will halt the program until an outside client tries to connect */
    connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");

    return connfd;
}

/**
 * @DEPRACETED - just reimplements memcpy lol
 * 
 * @brief Sets the value to the destination in memory, both of witch share common size in bytes
 * 
 * @param value ptr to the value
 * @param dest  ptr to the locacion in memory where to store the value
 * @param size_of_val_and_dest the length of the two in bytes
 */
void set_arg_value(const void *value, void *dest, size_t sizeof_val_and_dest){
    const unsigned char *bvalue = (const unsigned char *)value;
    unsigned char *bdest = (unsigned char *)dest;

    for (size_t i = 0; i < sizeof_val_and_dest; i++)
    {
        bdest[i] = bvalue[i];
    }
    
}

/**
 * @brief Finds indices for command line switch values in the argv argument. 
 *        Indices are stored in indices in the order corresponding to the ordering in the
 *        array of all possible switches (defined above, available_args)
 * 
 * @param argc number of arguments (always at least 1 - program name)
 * @param argv the array of arguments (index 0 is the program name)
 * @param indices output array containing the indices to the argv 
 *                on which the values for corresponding switches 
 *                (as defined in available_args)
 *                reside
 * 
 * example: 
 *          argv:           | prg.exe | -ip4 | 127.0.0.1 | -port | 8080 |
 *          available_args: | -port | -ip4 |
 *       -> indices:        |   4   |   2  |
 */
void find_arg_indices(int argc, char const *argv[], int indices[]){

    /* search the arguments of the program and handle them
       start at 1 since argv[0] is name of the program */
    for (size_t i = 1; i < argc; i++)
        for (size_t j = 0; j < ARGNUM; j++)
            if(!strcmp( available_args[j], argv[i] )){
                /* found the switch
                   -> its value is the next argv element
                      and also skip the value, it's not a switch (hence ++i) */
                indices[j] = ++i;
                break;
            }    
}

int main(int argc, char const *argv[])
{
    int sockfd, connfd;
    int port = -1;
    const char *ip = NULL;

    /* ARGS HANDLING */
    /* args contain swithes and values */
    /* each switch is followed by its value*/

    /* indices to the argv where the values of corresponding switches reside (same order as available_args) 
       0 is an invalid index, since argv[0] is the program name */
    int args_indices[ARGNUM] = { 0 };
    find_arg_indices(argc, argv, args_indices);

    if(argc < MIN_ARGNUM)
        printf("Warning: not enough parameters. IP or port missing. Default will be used.\n");
       
    
    if(args_indices[0])
        port = (int)strtol(argv[args_indices[0]],NULL,10);
    else
        printf("Warning: port not specified. Using default: %i.\n", port=DEF_PORT);

    if(args_indices[1])
        ip = argv[args_indices[1]];
    else
        printf("Warning: IP not specified. Using default: %s.\n", ip=DEF_IP);    

    //printf("%s %i", ip, port);

    /* STARTING SERVER AND ESTABLISHING CONNECTION */


    char bf[80] = {0};
    char *tkn;
    for (;;)
    {
        bzero(bf,80);
        fgets(bf,80,stdin);
        sscanf(bf,"%s",tkn);

        for (size_t i = 0; i < CMDNUM; i++)
            if(!strcmp(cmds[i],tkn))
                cmd_fns[i](0,(void *)bf);
        
    }
    


    return 0;
}
