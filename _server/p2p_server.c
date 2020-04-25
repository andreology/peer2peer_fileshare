//Andre Barajas, Gabriel Espejo
//Project 3
//Spring 2020
//Peer to Peer file sharing software
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>

#include "time.h"
#define TRUE 1
#define BUFFER_LENGTH 1024
#define FALSE 0
#define PORT 8080
//creating a structure to hold servant info
struct P2pServent
{
    char date[9];
    uint32_t globalunique_id;
    char servant_file[50];
    uint32_t live;
};
//creating a structure to hold clients in list
struct P2pRegistry
{
    int amt;
    struct P2pServent clients[2];
};
//use this global value to hold registered servants
struct P2pRegistry registry;
//global variables

int fdserver, fdudp;
char buffer[BUFFER_LENGTH] = {0};
int soc_new, soc_new0, unique_id;
struct tm *data_time;
struct P2pServent received;
struct sockaddr_in addy_server, addy_client;
int sizeof_addy = sizeof(addy_server);
char message[100];
time_t tym_atm;

ssize_t value_rec;
socklen_t buffer_size;

//gets filename
char* getFileName(char * filename)
{
    char *start = &filename[10];
    char *end = &filename[strlen(filename)];
    char *substr = (char *)calloc(1, end - start);
    memcpy(substr, start, end-start); //get filename from buffer
    int len = strlen(substr);
    if(substr[len-1] == '\n'){ //Removes newline
        substr[len-1] = '\0';
    }
    return substr;
}
//Compares file names to filelists
int searchFile(char* filename){
    char* filenameFinal = getFileName(filename);
    for(int i = 0; i < 2; i++ ){
        if(strstr(registry.clients[i].servant_file,filenameFinal) != NULL){ //Compares filename to file lists from servants
            return i + 1;
        }
    }
    return 0;
}

//manipulating time to be able to check every 200 seconds
int timein_seconds(char curr_time[9])
{
    int minutes;
    //check string for time
    if (curr_time[4] == '0')
    {
        minutes = 10;
    }
    else
    {
        minutes = curr_time[4] - '0';
    }
    //removing 0 and 9 from digits
    int first = (curr_time[6] - '0') * 10;
    int second = curr_time[7] - '0';
    int seconds = first + second;

    return 60 * minutes + seconds;
}
//show clients GUID and files belonging to client
void show_client_data(int counter)
{
    printf("\n*-------------------------------------------------*\n");
    if (registry.clients[counter].live == 1)
    {
        //check connection
        printf("...connected client %d with GUID: %d",
               counter + 1, registry.clients[counter].globalunique_id);
        printf("\n...connected client %d with files %s",
               counter + 1, registry.clients[counter].servant_file);

        printf("\n...connected client %d registered at ", counter + 1);
        puts(registry.clients[counter].date);
    }
    printf("*-------------------------------------------------*\n");
}
//adding clients to new array and marking clients that are not reponsive
void checkclient_status(int registered_client, struct P2pServent listof_servents[2])
{
    for (int i = 0, k = 0; i < registry.amt; i++)
    {
        if (i == registered_client)
        {
            printf("\n...client at %d .... not found", i);
            continue;
        }
        else
        {
            //check servants
            listof_servents[k].globalunique_id = registry.clients[i].globalunique_id;
            listof_servents[k].live = registry.clients[i].live;
            strcpy(listof_servents[k].servant_file, registry.clients[i].servant_file);
            strcpy(listof_servents[k].date, registry.clients[i].date);
            k++;
        }
    }
}
//check client time input and change to new time
void check_node_time(int counter)
{
    struct tm *data_time;
    time_t tym_atm;
    time(&tym_atm);
    data_time = localtime(&tym_atm);
    //register new time for client
    strftime(registry.clients[counter].date, sizeof(registry.clients[counter].date), "%H:%M:%S", data_time);
}
//updating registered clients
void udpate_reg(struct P2pServent *servant_at, struct P2pServent *curr_servant)
{
    //traverse list of servant clients
    for (int i = 0; i < registry.amt; i++)
    {
        servant_at[i].globalunique_id = curr_servant[i].globalunique_id;
        servant_at[i].live = curr_servant[i].live;
        strcpy(servant_at[i].servant_file, curr_servant[i].servant_file);
        strcpy(servant_at[i].date, curr_servant[i].date);
    }
}
//method to check connection with clients is still valid using Udp protocal
void *ping(void *arg)
{

    char *hello = "HELLO";
    //create UDP socket
    if ((fdudp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("ERROR: failed to create socket ");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n...succesfully created udp Socket\n");
    }

    memset(&addy_server, 0, sizeof(addy_server));
    memset(&addy_client, 0, sizeof(addy_client));

    //providing socket data needed
    addy_server.sin_addr.s_addr = INADDR_ANY;
    addy_server.sin_family = AF_INET;
    addy_server.sin_port = htons(PORT);

    struct timeval finish_time;
    finish_time.tv_usec = 50;
    finish_time.tv_sec = 0;

    setsockopt(fdudp, SOL_SOCKET, SO_RCVTIMEO, &finish_time, sizeof finish_time);
    //attemp to bind socket to machine
    if (bind(fdudp, (const struct sockaddr *)&addy_server, sizeof(addy_server)) < 0)
    {
        perror("ERROR: failed to bind udp");
        exit(EXIT_FAILURE);
    }
    //simulate a wait for client
    while (TRUE)
    {

        value_rec = recvfrom(fdudp, (char *)buffer, BUFFER_LENGTH, MSG_WAITALL, (struct sockaddr *)&addy_client, &buffer_size);
        buffer[value_rec] = '\0';
        //wait for a message from clients
        if (value_rec == -1)
        {
            printf("\n."); //if no ping from clients
        }
        else
        {
            printf("\n\n...received ping from client[%s]", buffer);
        }
        int globalunique_id = buffer[0] - '0';
        //verify time from client is less than the current time
        char this_time[9];
        time(&tym_atm);
        data_time = localtime(&tym_atm);
        strftime(this_time, sizeof(this_time), "%H:%M:%S", data_time);

        int tym_atm = timein_seconds(this_time);
        //retreive message from clients, and if client is still live update the registry
        for (int registered_client = 0; registered_client < registry.amt; registered_client++)
        {

            if (registry.clients[registered_client].globalunique_id != globalunique_id)
            {
                int timestamp = timein_seconds(registry.clients[registered_client].date);

                if (abs(timestamp - tym_atm) >= 5)
                {
                    //duplicate list
                    struct P2pServent *registry_temp = malloc(sizeof(registry.clients));
                    checkclient_status(registered_client, registry_temp); //take out client from reg
                    registry.amt = registry.amt - 1;                      // delete first registry
                    memset(registry.clients, 0, sizeof(registry.clients));
                    udpate_reg(registry.clients, registry_temp); //update next registry
                }
            }
            else
            {
                check_node_time(registered_client);
            }
        }

        bzero(buffer, sizeof(buffer));
        sleep(1);
    }
    close(fdudp);
    pthread_exit(NULL);
}

//method to transfer any neccesary files using tcp protocal
void *file_transfers(void *arg)
{
    //creating socket
    if ((fdserver = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("ERROR: tcp failed to connect");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n...created tcp socket succesfully\n");
    }
    addy_server.sin_addr.s_addr = INADDR_ANY;
    addy_server.sin_family = AF_INET;
    addy_server.sin_port = htons(PORT);

    //binding port
    if (bind(fdserver, (struct sockaddr *)&addy_server, sizeof(addy_server)) < 0)
    {
        perror("ERROR: failed to connect tcp bind");
        exit(EXIT_FAILURE);
    }
    //continue to listen for connections
    if (listen(fdserver, 3) < 0)
    {
        perror("ERROR: listening unsuccessful");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n...listening at tcp socket\n");
    }
    //attempt to accept connections
    if ((soc_new = accept(fdserver, (struct sockaddr *)&addy_server, (socklen_t *)&sizeof_addy)) < 0)
    {
        perror("ERROR: accepting connection failure\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        //attempt to register client
        printf("\n...succesfully connected tcp socket One\n");
        struct P2pServent received;
        unique_id = 0;

        recv(soc_new, &received.servant_file, sizeof(received.servant_file), 0);
        received.globalunique_id = ++unique_id;
        time(&tym_atm);
        data_time = localtime(&tym_atm);
        strftime(received.date, sizeof(received.date), "%H:%M:%S", data_time);
        //assign client values
        received.live = 1;
        registry.clients[registry.amt++] = received;
        show_client_data(registry.amt - 1);
        int output = send(soc_new, &received.globalunique_id, sizeof(received.globalunique_id), 0);
    }
    //attemp next connection with client 2
    if ((soc_new0 = accept(fdserver, (struct sockaddr *)&addy_server, (socklen_t *)&sizeof_addy)) < 0)
    {
        perror("ERROR: accepting connection failure 2\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        //attemp to register next client
        printf("\n...succesfully connected tcp socket two\n");
        recv(soc_new0, &received.servant_file, sizeof(received.servant_file), 0);
        received.globalunique_id = ++unique_id;

        time(&tym_atm);
        data_time = localtime(&tym_atm);
        strftime(received.date, sizeof(received.date), "%H:%M:%S", data_time);

        received.live = 1;
        registry.clients[registry.amt++] = received;
        show_client_data(registry.amt - 1);
        int next_output = send(soc_new0, &received.globalunique_id, sizeof(received.globalunique_id), 0);
    }
    //edit sockets to non blocking to prevent deadlock
    fcntl(soc_new0, F_SETFL, O_NONBLOCK);
    fcntl(soc_new, F_SETFL, O_NONBLOCK);
    fcntl(fdserver, F_SETFL, O_NONBLOCK);

    while (TRUE)
    {
        int valread = recv(soc_new, buffer, BUFFER_LENGTH, 0);
        int valread2 = recv(soc_new0, buffer, BUFFER_LENGTH, 0);
        if(strstr(buffer, ".txt") != NULL || strstr(buffer, ".c") != NULL){ //If there is a file name in buffer
            int location = searchFile(buffer);
            if(location != 0)
            {
                sprintf(message, "File is in client[%d]",location);
                if(valread > -1 && valread2 == -1)
                {                                        
                    printf("\n%s\n",message);
                    int sflag = send(soc_new, message, sizeof(message),0);
                    memset(message, 0, sizeof(message));
                }
                else if (valread == -1 && valread2 > -1)
                {
                    printf("\n%s\n",message);
                    int sflag = send(soc_new0, message, sizeof(message),0);
                    memset(message, 0, sizeof(message));
                }
            }
            else{            
                //If there is no file name in buffer
                strcpy(message,"File Not Found");
                if(valread > -1 && valread2 == -1){
                    printf("\n%s\n", message);
                    int sflag = send(soc_new, message, sizeof(message),0);
                    memset(message, 0, sizeof(message));
                }
                else if(valread == -1 && valread2 > -1){
                    printf("\n%s\n", message);
                    int sflag = send(soc_new0, message, sizeof(message), 0);
                    memset(message, 0, sizeof(message));
                }
                
            }
        }
        else{
            printf("%s", buffer);
        }
        bzero(buffer, sizeof(buffer));
        sleep(3);
    }

    pthread_exit(NULL);
}
//tester method to test threads
int main(int argc, char const *argv[])
{
    //create both threads to ping and transfer between client and server registry
    pthread_t threads[2];
    int conn_returned = pthread_create(&threads[0], NULL, &file_transfers, NULL);
    if (conn_returned)
    {
        printf("ERROR: thread unsuccessfully created %d \n", conn_returned);
        exit(-1);
    }
    conn_returned = pthread_create(&threads[1], NULL, &ping, NULL);
    if (conn_returned)
    {
        printf("ERROR: thread unsuccessfully created %d \n", conn_returned);
        exit(-1);
    }
    //joining threads to end gracefully
    for (int i = 0; i < 2; i++)
    {
        conn_returned = pthread_join(threads[i], NULL);
        if (conn_returned)
        {
            printf("ERROR: unsuccessfully joined thread %d \n", conn_returned);
        }
    }
    while (TRUE)
    {
        printf("ERROR: unsuccessfully joined thread \n");
    }
    //close resources
    close(soc_new);
    close(soc_new0);

    return 0;
}
