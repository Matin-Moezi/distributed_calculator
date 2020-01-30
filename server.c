/******************************* Distributed Calculator *****************************************/
/******************************* Client-Server Architecture *************************************/
/******************************* Computer Network Course Project ********************************/
/******************************* Matin Moezi #9512058 *******************************************/
/******************************* Fall 2019 ******************************************************/

/******************************* Server Program *************************************************/

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXMSGSIZE 512			// Maximum request message buffer size
#define MAXRESSIZE 1024			// Maximum response message buffer size

// thread function argument type
struct client_conn_t {
	int fd;
	struct sockaddr_in addr;
};
typedef struct client_conn_t client_conn_t;

// initialization the server
// create socket and listen 
int init_server()
{
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("create socket");
		exit(EXIT_FAILURE);
	}
	if(listen(sockfd, SOMAXCONN) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

// get socket hostname(dot-decimal) and port
int getsockaddr(int fd, char *hostname, int *port)
{
	struct sockaddr_in addr;
	int len = sizeof(addr);
	if(getsockname(fd, (struct sockaddr *)&addr, &len) == -1)
	{
		perror("get socket address");
		return -1;
	}
	if(hostname != NULL)
	{
		char host[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET, &(addr.sin_addr), host, INET_ADDRSTRLEN) == NULL)
		{
			perror("network byte address to dot-decimal address");
			return -1;
		}
		strcpy(hostname, host);
	}
	*port = ntohs(addr.sin_port);
	return 0;
}

// parse request message to operation and operands
// return -1 if request message format is invalid
int req_parser(char *request, char *op, double *op1, double *op2)
{
	int i;
	if((i = sscanf(request, "Calculation Request:\n\n$ %s $ %lg $ %lg $\n", op, op1, op2)) < 2)
		return -1;
	return i;
}

// calculate operands by operation
// return result and calculation time
// return -1 if operation of operands is invalid
int calc(char *op, double op1, double op2, double *result, double *time)
{
	double end, begin = clock();
	if(strcmp(op, "ADD") == 0)
	{
		*result = op1 + op2;
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Subtract") == 0)
	{
		*result = op1 - op2;
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Divide") == 0)
	{
		*result = op1 / op2;
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Multiply") == 0)
	{
		*result = op1 * op2;
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Sin") == 0)
	{
		*result = sin(op1);
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Cos") == 0)
	{
		*result = cos(op1);
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Tan") == 0)
	{
		*result = tan(op1);
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else if(strcmp(op, "Cot") == 0)
	{
		*result = tan(M_PI/2 - op1);
		end = clock();
		*time = (double)(end - begin) / CLOCKS_PER_SEC;
	}
	else
	{
		fprintf(stderr, "invalid operation.\n");
		return -1;
	}
	return 0;
}

// create response message format with result and time of calculation
void create_response(double result, double exec_time, char *response)
{
	sprintf(response, "Calculation Response:\n\n$ %lg $ %lg $\n", exec_time, result);
}

// handle each client connection
void  *connection_handler(void *arg)
{
	client_conn_t *client_conn = (client_conn_t *)arg;
	int sock_port, client_port = ntohs(client_conn->addr.sin_port);
	char client_host[INET_ADDRSTRLEN], request[MAXMSGSIZE];

	// convert network byte address to dot-decimal hostname
	if(inet_ntop(AF_INET, &(client_conn->addr.sin_addr), client_host, INET_ADDRSTRLEN) == NULL)
	{
		perror("connection handler");
		pthread_exit(NULL);
	}

	// get server socket address
	if(getsockaddr(client_conn->fd, NULL, &sock_port) == -1)
		pthread_exit(NULL);
	printf("client %s %d connected to port %d.\n", client_host, client_port, sock_port);
	while(recv(client_conn->fd, request, MAXMSGSIZE, 0) > 0)
	{
		double op1, op2, result, exec_time;
		char operation[10], response[MAXRESSIZE];

		// parse request message
		int parser_res = req_parser(request, operation, &op1, &op2);
		if(parser_res == -1)
		{
			sprintf(response, "invalid request format.\n");
			goto send_res;
		}
		else if(parser_res == 2)
			printf("client %s %d -> %s %lg\n", client_host, client_port, operation, op1);
		else if(parser_res == 3)
			printf("client %s %d -> %s %lg %lg\n", client_host, client_port, operation, op1, op2);

		// calculate client request
		if(calc(operation, op1, op2, &result, &exec_time) == -1)
		{
			sprintf(response, "invalid operation or operands.\n");
			goto send_res;
		}
		create_response(result, exec_time, response);

		// send response message to client
send_res:
		if(send(client_conn->fd, response, strlen(response), 0) < 0)
			perror("sending response");
		bzero(request, MAXMSGSIZE);
	}
	pthread_exit(NULL);
}


int main()
{
	int client_sockfd, sockfd = init_server();
	int port, len;
	struct sockaddr_in client_addr;
	len = sizeof(client_addr);
	if(getsockaddr(sockfd, NULL, &port) == -1)
		exit(EXIT_FAILURE);
	printf("Listening on port %d...\n", port);

	// waiting for incomming request
	while((client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)))
	{
		pthread_t conn;
		client_conn_t handler_arg = {
			client_sockfd,
			client_addr
		};

		// create a thread for each client socket
		pthread_create(&conn, NULL, connection_handler, (void *)&handler_arg);
		pthread_detach(conn);
	}
	return 0;
}
