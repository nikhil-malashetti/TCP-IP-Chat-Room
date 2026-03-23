#include "server.h"

#define SUCCESS 1
#define FAILURE 0

extern user_node *head;
extern active_user *active_head;

pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg)
{
	int conn_fd = *((int *)arg);
	free(arg);

	while(1)
	{
		auth_info credentials;
		msg_frame frm;
		memset(&frm, 0, sizeof(frm));
		if(recv(conn_fd, &credentials, sizeof(credentials), 0) < 0)
		{
			perror("recv");
			exit(0);
		}

		/* For user Login */
		if(credentials.action == 1) 
		{
			if(find_username(credentials.uname) == SUCCESS)
			{
				if(find_active_fd(credentials.uname, credentials.passwd) == -1)
				{
					if(verify_password(credentials.passwd, credentials.uname) == SUCCESS)
					{
						printf("%s joined the network...\n", credentials.uname);
						strcpy(frm.data, "OK");
						frm.mtype = NOTIFY;
						send(conn_fd, &frm, sizeof(frm), 0);
						broadcast_login(credentials.uname);
						add_active_user(credentials.uname, conn_fd);
						send_online_list(conn_fd, credentials.uname);
					}
					else
					{
						strcpy(frm.data, "WRONG PASSWORD");
						frm.mtype = NOTIFY;
						send(conn_fd, &frm, sizeof(frm), 0);
						continue;
					}
				}
				else
				{
					strcpy(frm.data, "USER ALREADY ACTIVE");
					frm.mtype = NOTIFY;
					send(conn_fd, &frm, sizeof(frm), 0);
					continue;

				}
			} 
			else
			{
				strcpy(frm.data, "UNKNOWN USER");
				frm.mtype = NOTIFY;
				send(conn_fd, &frm, sizeof(frm), 0); 
				continue;
			}
		}
		/* For User registration */ 
		else if(credentials.action == 2)
		{
			if(find_username(credentials.uname) == FAILURE)
			{
				prepend_user(credentials.uname, credentials.passwd);
				save_users();
				strcpy(frm.data, "OK");
				frm.mtype = NOTIFY;
				send(conn_fd, &frm, sizeof(frm), 0); 
				broadcast_login(credentials.uname);
				add_active_user(credentials.uname, conn_fd);
				send_online_list(conn_fd, credentials.uname);
			}
			else
			{
				strcpy(frm.data, "USERNAME TAKEN");
				frm.mtype = NOTIFY;
				send(conn_fd, &frm, sizeof(frm), 0); 
			}
		}
		while(1)
		{
			msg_frame frm;
			memset(&frm, 0, sizeof(frm));
			int ret = recv(conn_fd, &frm, sizeof(frm), 0);
			if(ret <= 0)
			{
				close(conn_fd);
				pthread_exit(NULL);
			}
			if(frm.mtype == SINGLE_CHAT)
			{
				int fd = find_active_fd(frm.to, frm.from);
				if(fd != -1)
				{
					send(fd, &frm, sizeof(frm), 0);
				}
				else
				{
					memset(&frm, 0, sizeof(frm));
					strcpy(frm.data, "RECIPIENT UNAVAILABLE");
					frm.mtype = NOT_AVAIL;
					send(conn_fd, &frm, sizeof(frm), 0);
				}
			}
			else if(frm.mtype == GROUP_CHAT)
			{
				pthread_mutex_lock(&active_mutex);
				active_user *temp = active_head;
				while(temp != NULL)
				{
					if(strcmp(temp->uname, frm.from) == 0)
					{
						temp = temp->next;
						continue;
					}
					send(temp->conn_fd, &frm, sizeof(frm), 0);
					temp = temp->next;
				}
				pthread_mutex_unlock(&active_mutex);
			}
			else if(frm.mtype == EXIT)
			{
				remove_active_user(frm.from);
				broadcast_logout(frm.from);
				close(conn_fd);
				pthread_exit(NULL);  
			}
		}
	}
	close(conn_fd);
}

void handle_shutdown(int signum)
{
	broadcast_crash();
	signal(SIGINT, SIG_DFL);
	usleep(1000);
	kill(getpid(), SIGINT);
}

void load_users()
{
	user_node *new = malloc(sizeof(user_node));
	FILE *fptr = fopen("users.txt", "r");
	if(fptr == NULL)
	{
		perror("fopen");
		exit(-1);
	}
	char uname[30];
	char passwd[30];
	while(fscanf(fptr, "%[^;];%[^\n]\n", uname, passwd) != EOF)
	{
		prepend_user(uname, passwd);
	}
}

void prepend_user(char *uname, char *passwd)
{
	user_node *temp;
	user_node *new = malloc(sizeof(user_node));
	if(new == NULL)
	{
		perror("malloc");
		return;
	}
	strcpy(new->uname, uname);
	strcpy(new->passwd, passwd);
	new->next = head;
	head = new; 
}

void save_users()
{
	FILE *fptr = fopen("users.txt", "w");
	user_node *temp = head;
	while(temp != NULL)
	{
		fprintf(fptr, "%s;%s\n", temp->uname, temp->passwd);
		temp = temp->next;
	}
	fclose(fptr);
}

int find_username(char *uname)
{
	user_node *temp = head;
	if(head == NULL)
	{
		return FAILURE;
	}
	while(temp != NULL)
	{
		if(strcasecmp(temp->uname, uname) == 0)
		{
			return SUCCESS;
		}
		temp = temp->next;
	}
	return FAILURE;
}

int verify_password(char *passwd, char *uname)
{
	user_node *temp = head;
	if(head == NULL)
	{
		return FAILURE;
	}
	while(temp != NULL)
	{
		if((strcasecmp(temp->passwd, passwd) == 0) && (strcasecmp(temp->uname, uname) == 0))
		{
			return SUCCESS;
		}
		temp = temp->next;
	}
	return FAILURE;
}


void add_active_user(char *uname, int fd)
{
	pthread_mutex_lock(&active_mutex);
	active_user *new = malloc(sizeof(active_user));
	strcpy(new->uname, uname);
	new->conn_fd = fd;
	new->next = NULL;
	if(active_head == NULL)
	{
		active_head = new;
	}
	else
	{
		new->next = active_head;
		active_head = new;
	}
	pthread_mutex_unlock(&active_mutex);
}

void send_online_list(int conn_fd, char *uname)
{
	msg_frame frm;
	pthread_mutex_lock(&active_mutex);
	active_user *temp = active_head;
	while(temp != NULL)
	{
		if(strcmp(temp->uname, uname) == 0)
		{
			temp = temp->next;
			continue;
		}

		memset(&frm, 0, sizeof(frm));
		frm.mtype = USR_LIST;
		strcpy(frm.data, temp->uname);
		send(conn_fd, &frm, sizeof(frm), 0);

		temp = temp->next;
	}
	pthread_mutex_unlock(&active_mutex);
	memset(&frm, 0, sizeof(frm));
	strcpy(frm.data, "end");
	send(conn_fd, &frm, sizeof(frm), 0);
}

int find_active_fd(char *uname, char *from)
{
	pthread_mutex_lock(&active_mutex);
	active_user *temp = active_head;
	while(temp != NULL)
	{
		if((strcmp(temp->uname, uname) == 0) && (strcmp(temp->uname, from) != 0))
		{
			pthread_mutex_unlock(&active_mutex);
			return temp->conn_fd;
		}
		temp = temp->next;
	}
	pthread_mutex_unlock(&active_mutex);
	return -1;
}

void broadcast_login(char *uname)
{
	pthread_mutex_lock(&active_mutex);
	active_user *temp = active_head;
	msg_frame frm;
	while(temp != NULL)
	{
		memset(&frm, 0, sizeof(frm));
		frm.mtype = USR_LIST;
		strcpy(frm.data, uname);
		send(temp->conn_fd, &frm, sizeof(frm), 0);
		temp = temp->next;
	}
	pthread_mutex_unlock(&active_mutex);
}

void remove_active_user(char *uname)
{
	pthread_mutex_lock(&active_mutex);
	active_user *temp = active_head;
	active_user *prev = NULL;

	while(temp != NULL)
	{
		if(strcmp(temp->uname, uname) == 0)
		{
			if(prev == NULL)
			{
				active_head = temp->next;
			}
			else
			{
				prev->next = temp->next;   
			}

			printf("%s disconnected from network...\n", uname);
			free(temp);
			break;
		}
		else
		{
			prev = temp;
			temp = temp->next;   
		}

	}
	pthread_mutex_unlock(&active_mutex);
}

void broadcast_logout(char *uname)
{
	pthread_mutex_lock(&active_mutex);
	active_user *temp = active_head;
	msg_frame frm;
	while(temp != NULL)
	{
		memset(&frm, 0, sizeof(frm));
		frm.mtype = EXIT;
		strcpy(frm.from, uname);
		send(temp->conn_fd, &frm, sizeof(frm), 0);

		temp = temp->next;
	}
	pthread_mutex_unlock(&active_mutex);
}

void broadcast_crash(void)
{
	pthread_mutex_lock(&active_mutex);
	active_user *temp = active_head;
	msg_frame frm;
	while(temp != NULL)
	{
		memset(&frm, 0, sizeof(frm));
		frm.mtype = SERVER_CRASH;
		send(temp->conn_fd, &frm, sizeof(frm), 0);
		temp = temp->next;
	}
	pthread_mutex_unlock(&active_mutex);
}
