#ifndef SERVER_H
#define SERVER_H

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

typedef struct node
{
	char uname[30];
	char passwd[30];
	struct node *next;
}user_node;

typedef struct active_node
{
	char uname[30];
	int conn_fd;
	struct active_node *next;
}active_user;

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
#define SINGLE_CHAT  2
#define GROUP_CHAT 3
#define NOTIFY      4
#define EXIT        5
#define NOT_AVAIL   6
#define SERVER_CRASH   7

void *handle_client(void *arg);
void load_users();
void save_users();
int verify_password(char *passwd, char *uname);
int find_username(char *uname);
void add_active_user(char *uname, int fd);
void send_online_list(int conn_fd, char *uname);
void broadcast_login(char *uname);
int find_active_fd(char *uname, char *from);
void prepend_user(char *uname, char *passwd);
void broadcast_logout(char *uname);
void remove_active_user(char *uname);
void handle_shutdown(int signum);
void broadcast_crash(void);
#endif
