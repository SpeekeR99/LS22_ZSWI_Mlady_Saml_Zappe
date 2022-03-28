/* ------- INCLUDES, GENERAL CONSTANTS  */
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
/*
#include "serv_functions.h"
#include "D:\_skola\ZSWI\PRJ\fem-like-spreading-modelling\C\server\serv_functions.h"
*/
//#include "D:\_skola\ZSWI\PRJ\fem-like-spreading-modelling\C\server\serv_functions.h"
#include "serv_functions.h"

#define DEF_IP NULL
#define DEF_PORT 4242
#define MSG_MAX_LEN 2048
#define CMD_MAX_LEN 14

/*-------- PROGRAM ARGUMENTS */

#define REQ_ARGNUM 2
#define ARGNUM 2
#define MIN_ARGNUM 1 + 2*REQ_ARGNUM
/* all possible switches on commandline */
char *available_args[ARGNUM] = {"-port", "-ip4"};

/* -------- COMMANDS TO THE PROGRAM */

#define CMDNUM 3
/* The array of commands */
char *cmds[CMDNUM] = {"send_data","start","out"};
/* The array of functions invoked by commands
    The functions return void * if they return anything and accept 
    the connfd and additional args stored as void * 
    functions are defined in a separate file 
    (serv_function.h in this case) */
void *(*cmd_fns[CMDNUM])(int, void *) = {&send_data_from_simulation,&start_simulation,&out};

/* -------- CODE SECTION */

/**
 * @brief Creates a listening socket on supplied IP and port and returns its sockfd
 * 
 * @param IP The IP address to listen on. If null, INADDR_ANY is used.
 * @param port The port to listen on
 * @return The sockfd (int) of the listening socket, ready to accept a client
 * The queue of pending connections is of length 5, but only 1 client should request communication
 */
int create_listen_socket(const char *IP, int port){
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
    servaddr.sin_addr.s_addr = IP ? inet_addr(IP) : htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
   
    /* Binding newly created socket to given IP and verification */
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded.\n");
   
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
 * @brief The infinite loop for communication with the client
 *        Client sends command and arguments, server does the command
 * 
 * @param connfd the connection's file descriptor
 */
void comm_loop(int connfd){
    char bf[MSG_MAX_LEN] = {0};
    char cmd[CMD_MAX_LEN] = {0};

    //printf("Entering comm loop");
    for (;;)
    {
        bzero(bf,MSG_MAX_LEN);
        bzero(cmd, CMD_MAX_LEN);
        while(!read(connfd,bf,MSG_MAX_LEN)){
            printf("Connection lost\n");
            return;
        }
        sscanf(bf,"%s",cmd);
        /* now: bf contains the recieved line, cmd the first word 
           (should be a name of a command from cmds array) */

        //printf("%s %s", bf, cmd);

        for (size_t i = 0; i < CMDNUM; i++)
            /* find command and call it with the connection file descriptor and the recieved line as its arguments */
            if(!strcmp(cmds[i],cmd)){
                printf("Calling command: %s\n", cmd);
                cmd_fns[i](connfd,(void *)bf);
                break;
            }

            //printf("Message recieved, but was not command: %s\n", bf);                
        
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
        printf("Warning: IP not specified. Using default: INADDR_ANY.\n"), ip=DEF_IP;    

    //printf("%s %i", ip, port);

    /* STARTING SERVER */

    sockfd = create_listen_socket(ip, port);

    /* Indefinitely, accept a connection and then enter a communication loop with the client
       in the communication, the client gives commands to the server */
    for(;;){
        connfd = create_connection(sockfd);
        comm_loop(connfd);
    }


    return 0;
}
