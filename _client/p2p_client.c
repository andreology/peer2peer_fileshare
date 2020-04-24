//Andre Barajas, Gabriel
//Project 3
//Spring 2020
//Peer to Peer file sharing software
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>

#define TRUE 1
#define PORT 8080
#define MSG_LEN 100
#define IP_ADDR "127.0.0.1"
#define SIZEOF_BUFFER 1024
//creating a structure to hold servant info
struct P2pServent
{
    uint32_t live;
    uint32_t globalunique_id;
    char client_data[9];
    char servant_files[50];
};
//config for sockets needed UDP and TCP
int first_socket, sockerfor_udp, len, n;
struct sockaddr_in client_addr, addy_server;
struct P2pServent curr_data;
socklen_t CLADDR_LEN = sizeof(client_addr);
int inet_pton();
char message_buff[MSG_LEN];
char socket_buffer[SIZEOF_BUFFER] = {0};
char client_guid[10];
struct dirent *de;
//retreive input from user
void *get_input(void *arg)
{
    while (TRUE)
    {
        char label[] = "client 1: ";
        fgets(message_buff, MSG_LEN, stdin);
        combine(label, message_buff);
        strcpy(message_buff, label);
    }
    pthread_exit(NULL);
    return NULL;
}
//concatanate the two strings message + id
void combine(char *bam, char *wam)
{
    int yes, mam;
    yes = 0;
    while (bam[yes] != '\0')
    {
        yes++;
    }
    mam = 0;
    while (wam[mam] != '\0')
    {
        bam[yes] = wam[mam];
        mam++;
        yes++;
    }
    bam[yes] = '\0';
}
//TCP based socket to send and retreive messages
void *file_transfers(void *arg)
{
    int return_value = send(first_socket, &curr_data, sizeof(curr_data), 0);
    bzero(socket_buffer, sizeof(socket_buffer));
    recv(first_socket, &curr_data, sizeof(curr_data), 0);

    printf("...connected client One initiated %d\n", curr_data.globalunique_id);

    //convert to non blocking socket for security reasons
    fcntl(first_socket, F_SETFL, O_NONBLOCK);

    while (TRUE)
    {

        send(first_socket, message_buff, strlen(message_buff), 0);
        //delete data in  message
        memset(message_buff, 0, MSG_LEN);
        //show data message
        printf("%s", socket_buffer);
        bzero(socket_buffer, sizeof(socket_buffer));
        sleep(1);
    }
    //clean up resources used
    close(first_socket);
    pthread_exit(NULL);
    return NULL; //silence
}
//method to use UDP socket to check user client is alive or not
void *ping(void *arg)
{
    //generate UDP socket
    if ((sockerfor_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("ERROR: failed to connect udp");
        exit(0);
    }
    else
    {
        printf("\n...successfully connected client one at udp socket\n");
    }
    memset(&addy_server, 0, sizeof(addy_server));

    //input file server configuration
    addy_server.sin_port = htons(PORT);
    addy_server.sin_family = AF_INET;
    addy_server.sin_addr.s_addr = inet_addr(IP_ADDR);

    while (TRUE)
    {
        sleep(2);
        sprintf(client_guid, "%d", curr_data.globalunique_id);

        sendto(sockerfor_udp, (const char *)client_guid, strlen(client_guid),
               0, (const struct sockaddr *)&addy_server, sizeof(addy_server));

        printf("...connected client send message\n");
    }
    close(sockerfor_udp);
}
//driver to test udp and tcp methods
int main(int argc, char const *argv[])
{

    if (argv[1] != NULL)
    {
        DIR *dr = opendir(".");
        char dir_mess[50];
        int first = 0;
        while ((de = readdir(dr)) != NULL)
        {
            if (first == 0)
            {
                if(strstr(de->d_name, ".txt") != NULL || strstr(de->d_name, ".c") != NULL){
                    strcpy(dir_mess, de->d_name);
                    strcat(dir_mess, " ");
                    first = 1;
                }
            }
            else
            {
                if(strstr(de->d_name, ".txt") != NULL || strstr(de->d_name, ".c") != NULL){
                    strcat(dir_mess, de->d_name);
                    strcat(dir_mess, " ");
                }
            }
        }
        
        strcpy(curr_data.servant_files, dir_mess);
        printf("\n%s\n", curr_data.servant_files);
    }
    else
    {
        printf("\n Enter a file name\n");
        exit(0);
    }

    if ((first_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nERROR: unsuccessfully created socket \n");
        return -1;
    }
    memset(&addy_server, '0', sizeof(addy_server));

    //socket configurations
    addy_server.sin_family = AF_INET;
    addy_server.sin_port = htons(PORT);
    //convert ipv4 and 6 to binary format
    if (inet_pton(AF_INET, IP_ADDR, &addy_server.sin_addr) <= 0)
    {
        printf("\nERROR: this address isn't allowed  \n");
        return -1;
    }
    //connect to socket
    if (connect(first_socket, (struct sockaddr *)&addy_server, sizeof(addy_server)) < 0)
    {
        printf("\nERROR: failed to connect client one \n");
        return -1;
    }
    else
    {
        printf("\nSuccessfully connected client one");
        printf("\nip address at %s", IP_ADDR);
        printf("\nport number used %d\n", PORT);
    }
    int returned_conn;
    pthread_t threads[3];
    returned_conn = pthread_create(&threads[0], NULL, &get_input, NULL);
    if (returned_conn)
    {
        printf("ERROR: unsuccessfully created thread %d \n", returned_conn);
        exit(-1);
    }

    returned_conn = pthread_create(&threads[1], NULL, &file_transfers, NULL);
    if (returned_conn)
    {
        printf("ERROR: unsuccessfully created thread %d \n", returned_conn);
        exit(-1);
    }
    returned_conn = pthread_create(&threads[2], NULL, &ping, NULL);
    if (returned_conn)
    {
        printf("ERROR: unsuccessfully created thread %d \n", returned_conn);
        exit(-1);
    }
    //attempt to join threads
    for (int i = 0; i < 3; i++)
    {
        returned_conn = pthread_join(threads[i], NULL);
        if (returned_conn)
        {
            printf("ERROR: unsuccessfully joined threads %d \n", returned_conn);
        }
    }

    return 0;
}
