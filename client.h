#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

typedef struct 
{
	char uname[30];
	char passwd[30];
	int action;
}auth_info;

typedef struct 
{
	char from[30];
	char to[30];
	char data[100];
	int mtype;   
}msg_frame;


#define HOST_ADDR "127.0.0.1"
#define COMM_PORT 6333

#define USR_LIST    1
#define PRIVATE_CHAT  2
#define GROUP_CHAT 3
#define NOTIFY      4
#define EXIT        5
#define NOT_AVAIL   6
#define SERVER_CRASH   7

void display_online_users(int sockfd, char *uname);
void *recv_handler(void *args);
void sigint_handler(int num);
#endif
