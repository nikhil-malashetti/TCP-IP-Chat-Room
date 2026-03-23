#include "client.h"

auth_info credentials;
int sockfd;

int main()
{
	msg_frame frm;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(COMM_PORT);
	addr.sin_addr.s_addr  = inet_addr(HOST_ADDR);
	if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
	{
		printf("\n***** Connection failed ***** \n");
		return -1;
	}
	printf("\n***** Connected to the server ***** \n");
	do
	{ 
		printf("\n======== Main Menu =========\n");
		printf("1. Login\n2. Register\n"); 
		printf("Select an option: ");
		__fpurge(stdin);
		scanf("%d", &credentials.action);
		if(credentials.action != 1 && credentials.action != 2)
		{
			printf("‼️-> Invalid Option <-‼️\n -> Re-enter the option <-\n");
			continue;
		}
		__fpurge(stdin);
		printf("Enter username: ");
		scanf("%[^\n]", credentials.uname);
		__fpurge(stdin);
		printf("Enter password: ");
		scanf("%[^\n]", credentials.passwd);
		
		send(sockfd, &credentials, sizeof(credentials), 0);
		
		recv(sockfd, &frm, sizeof(frm), 0);

		if(strcmp(frm.data, "OK") == 0)
		{
			printf("\n -> Authentication successful <- \n");
			break;
		}
		else
		{
			printf("\n -> %s <-\n", frm.data);
			fflush(stdout);
		}
	} while(1);

	signal(SIGINT, sigint_handler);
	pthread_t tid;
	pthread_create(&tid, NULL, recv_handler, &sockfd);

	while(1)
	{
		usleep(10000);
		printf("\n=========== MESSAGE MENU ===========\n");
		printf("1. Private Chat\n2. Group Chat\n3. Disconnect\n");
		int opt;
		printf("Select an option: \n");
		do
		{
			opt = 0;
			fflush(stdout);
			__fpurge(stdin);
			scanf("%d", &opt);
			if(opt != 1 && opt != 2 && opt != 3)
				printf(" ***** Invalid Option *****‼️\n Re-enter the option: ");

		} while(opt != 1 && opt != 2 && opt != 3);
		if(opt == 1)
		{
			memset(&frm, 0, sizeof(frm));
			frm.mtype = PRIVATE_CHAT;
			strcpy(frm.from, credentials.uname);
			
			printf("Enter recipient name: ");
			fflush(stdout);
			__fpurge(stdin);
			scanf("%[^\n]", frm.to); 
			
			printf("Enter message: ");
			fflush(stdout);
			__fpurge(stdin);
			scanf("%[^\n]", frm.data);
			
			send(sockfd, &frm, sizeof(frm), 0);   
		}
		else if(opt == 2)
		{
			memset(&frm, 0, sizeof(frm));
			frm.mtype = GROUP_CHAT;
			strcpy(frm.from, credentials.uname);

			printf("Enter message: ");
			fflush(stdout);
			__fpurge(stdin);
			scanf("%[^\n]", frm.data);
			
			send(sockfd, &frm, sizeof(frm), 0);

		}
		else if(opt == 3)
		{
			memset(&frm, 0, sizeof(frm));
			frm.mtype = EXIT;
			strcpy(frm.from, credentials.uname);
			send(sockfd, &frm, sizeof(frm), 0);
		
			printf("Disconnecting...\n");
			exit(0);
		}
	}
}

void *recv_handler(void *args)
{
	int sockfd = *((int *)args);
	msg_frame frm;
	while(1)
	{
		memset(&frm, 0, sizeof(frm));
		int ret = recv(sockfd, &frm, sizeof(frm), 0);
		if(ret <= 0)
		{ 
			break;   
		}
		if(frm.mtype == PRIVATE_CHAT)
		{
			printf("\nPrivate message from %s : ", frm.from);
			printf("%s\n", frm.data);       
			fflush(stdout);

		}
		else if(frm.mtype == GROUP_CHAT)
		{
			printf("\nGroup message from %s : ", frm.from);
			printf("%s\n", frm.data);
			fflush(stdout);

		}
		else if(frm.mtype == USR_LIST)
		{
			if(strcmp(frm.data, "end") != 0)
			{
				printf("\n -> %s is online <- \n", frm.data); 
			}
			fflush(stdout);
		}
		else if(frm.mtype == NOT_AVAIL)
		{
			printf("\n -> %s <-\n", frm.data);
			fflush(stdout);
		}
		else if(frm.mtype == EXIT)
		{
			printf("\n-> %s is offline <-\n", frm.from);
			fflush(stdout);
		}
		else if(frm.mtype == SERVER_CRASH)
		{
			printf("\n -> HOST UNREACHABLE <-\n\n -> SERVER CRASHED <-\n\n");
			signal(SIGINT, SIG_DFL);
			kill(getpid(), SIGINT);
		}
	} 
}

void sigint_handler(int num)
{
	msg_frame frm;
	memset(&frm, 0, sizeof(frm));
	frm.mtype = EXIT;
	strcpy(frm.from, credentials.uname);
	send(sockfd, &frm, sizeof(frm), 0);
	signal(SIGINT, SIG_DFL);
	usleep(1000);
	kill(getpid(), SIGINT);
}
