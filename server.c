#include "server.h"

user_node *head = NULL;

active_user *active_head = NULL;

int main()
{
	load_users();
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(COMM_PORT);
	addr.sin_addr.s_addr  = inet_addr(HOST_ADDR);
	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind");
		return -1;
	}

	listen(sockfd, 100);
	printf("Waiting for connections...\n");
	signal(SIGINT, handle_shutdown);
	while(1)
	{
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		int *conn_fd = malloc(sizeof(int));
		*conn_fd = accept(sockfd, (struct sockaddr *)&client_addr, &len);
		pthread_t tid;
		/* Handling the concurrency of the clients */ 
		pthread_create(&tid, NULL, handle_client, conn_fd); 
		pthread_detach(tid);  
	}

}
