//Andre Barajas, Gabriel Espejo
//Project 3
//Spring 2020
//Peer to Peer file sharing software
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>

#define TRUE 1
#define PORT 8080
#define LENGTHOF_MESSAGE 100
#define IP_ADDR "127.0.0.1"
#define SIZEOF_BUFFER 1024
//creating a structure to hold servant info
struct P2pServent{
    uint32_t live;
    uint32_t globalunique_id;
    char client_data[9];
    char servant_files[50];
};
//config for sockets needed UDP and TCP
int second_socket, sockerfor_udp;
int len, n;
struct sockaddr_in addy_client, addy_server;
struct P2pServent curr_data;
socklen_t CLADDR_LEN = sizeof(addy_client);
int inet_pton();
char socket_buffer[SIZEOF_BUFFER] = {0};
char message_buff[LENGTHOF_MESSAGE];
char client_guid[10];
struct dirent *de;
//concatanate the two strings
void combine(char* bam, char *wam){
   int yes, mam;
   yes = 0;
   while (bam[yes] != '\0') {
      yes++;
   }
   mam = 0;
   while (wam[mam] != '\0') {
      bam[yes] = wam[mam];
      mam++;
      yes++;
   }
   bam[yes] = '\0';
}
//retreive input from user message + id
void* get_input(void* arg){
    while(TRUE){
        char label[] = "client 2: ";
        fgets(message_buff, LENGTHOF_MESSAGE, stdin);
        combine(label, message_buff);
        strcpy(message_buff, label);
    }
    pthread_exit(NULL);
    return NULL;
}
//TCP based socket to send and retreive messages
void* file_transfers(void* arg){
    int send_return = send(second_socket, &curr_data.servant_files, sizeof(curr_data.servant_files), 0); // send object
    bzero(socket_buffer, sizeof(socket_buffer));
    recv(second_socket, &curr_data.servant_files, sizeof(curr_data.servant_files), 0);         // get message

    printf("...received from connected client 2  %d\n", curr_data.globalunique_id);

  //convert to non blocking socket for security reasons
    fcntl(second_socket, F_SETFL, O_NONBLOCK);

    while(TRUE){
        send(second_socket , message_buff , strlen(message_buff) , 0);
        memset(message_buff, 0, LENGTHOF_MESSAGE);
        //display  message
        printf("%s",socket_buffer);
        bzero(socket_buffer, sizeof(socket_buffer));
        sleep(1);
    }
    //clean up resources used
    close(second_socket);
    pthread_exit(NULL);
    return NULL;
}
//method to use UDP socket to check user client is alive or not
void* ping(void* arg){
	//attempting to create udp socket
	if ((sockerfor_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("ERROR: unsuccessfully connected udp socket ");
		exit(0);
	}
    else{
        printf("\n...connected client two with udp socket\n");
    }

	memset(&addy_server, 0, sizeof(addy_server));

	  //input file server configuration
	addy_server.sin_family = AF_INET;
	addy_server.sin_port = htons(PORT);
	addy_server.sin_addr.s_addr = inet_addr(IP_ADDR);

    while(TRUE){

        sleep(2);
        sprintf(client_guid, "%d", curr_data.globalunique_id);
        sendto(sockerfor_udp, (const char*)client_guid, strlen(client_guid),
            0, (const struct sockaddr*)&addy_server,
            sizeof(addy_server));

        printf("...connected client two send message \n");
    }
	close(sockerfor_udp);
}
//driver to test udp and tcp methods
int main(int argc, char const *argv[])
{

    if(argv[1] != NULL)
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
    else{
        printf("\nEnter a file name\n");
        exit(0);
    }

	if ((second_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\nERROR: unsuccessfully connected socket  \n");
		return -1;
	}

	memset(&addy_server, '0', sizeof(addy_server));

    //socket configurations
  	addy_server.sin_family = AF_INET;
    addy_server.sin_port = htons(PORT);

    //convert ipv4 and 6 to binary format
    if(inet_pton(AF_INET, IP_ADDR, &addy_server.sin_addr)<=0)
	{
		printf("\nERROR: address isn't allowed \n");
		return -1;
	}
  //connect to socket
	if (connect(second_socket, (struct sockaddr *)&addy_server, sizeof(addy_server)) < 0)
	{
		printf("\nERROR: unsuccessfully connected client two \n");
		return -1;
    }
    else{
        printf("\n...connected client two");
        printf("\nip address at %s",IP_ADDR);
        printf("\nport number at %d\n\n",PORT);
    }
    int returned_conn;
    pthread_t threads[3];
    returned_conn = pthread_create(&threads[0], NULL, &get_input, NULL);
    if(returned_conn){
        printf("Error: unable to create thread, %d \n", returned_conn);
        exit(-1);
    }

    returned_conn = pthread_create(&threads[1], NULL, &file_transfers, NULL);
    if(returned_conn){
        printf("Error: unable to create thread, %d \n", returned_conn);
        exit(-1);
    }
    returned_conn = pthread_create(&threads[2], NULL, &ping, NULL);
    if(returned_conn){
        printf("Error: unable to create thread, %d \n", returned_conn);
        exit(-1);
    }
    //attempt to join threads
    for(int i = 0; i < 3; i++){
        returned_conn = pthread_join(threads[i], NULL);
        if(returned_conn){
            printf("ERROR: unsuccessfully joined threads %d \n", returned_conn);
        }
    }


    return 0;
}
