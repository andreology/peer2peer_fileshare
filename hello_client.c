//Andre Barajas, Gabriel
//Project 3
//Spring 2020
//Peer to Peer file sharing software
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#define PORT 8080
#define BUFFER_LENGTH 1024
int main()
{
  char newtork_buffer[BUFFER_LENGTH];
	int socket_client, length;
	char* sending_message = "HELLO(client)";
	struct sockaddr_in server_addy;
  //attempt to create udp socket
	if ((socket_client = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("UDP socket failed");
		exit(0);
	}
  //save data
	memset(&server_addy, 0, sizeof(server_addy));
   //input server data required for socket
   server_addy.sin_port = htons(PORT);
	server_addy.sin_family = AF_INET;
	server_addy.sin_addr.s_addr = inet_addr("127.0.0.1");
  printf("...Trial \n");
  //attemp to send message to server
	sendto(socket_client, (const char*)sending_message, strlen(sending_message),
		0, (const struct sockaddr*)&server_addy, sizeof(server_addy));
    //stand by for a message
    return_value = recvfrom(socket_client, (char *)newtork_buffer, BUFFER_LENGTH,
                MSG_WAITALL, (struct sockaddr *) &server_addy,
                &length);
    newtork_buffer[return_value] = '\0';
    printf("...succesfully received message from udp server\n");
    printf("...server registry %s\n", newtork_buffer);
    //close all resourcess
	close(socket_client);
	return 0;
}
