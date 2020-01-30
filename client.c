/******************************* Distributed Calculator *****************************************/
/******************************* Client-Server Architecture *************************************/
/******************************* Computer Network Course Project ********************************/
/******************************* Matin Moezi #9512058 *******************************************/
/******************************* Fall 2019 ******************************************************/

/******************************* Client Program *************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXMSGSIZE 512			// Maximum request message buffer size
#define MAXRESSIZE 1024			// Maximum response message buffer size

// create a socket and connect to the server
// return socket file descriptor
int sock_connect(const char *host, int port)
{
	// create IPv4 TCP socket
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("create socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in addr = {
		AF_INET,
		htons(port),
	};

	// convert dot-decimal host address to network byte address
	if(inet_pton(AF_INET, host, &(addr.sin_addr)) == -1)
	{
		perror("server address");
		exit(EXIT_FAILURE);
	}

	// connect socket to the server
	if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("connect socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

// parse response message to result and calculation time
// return -1 if response message format is invalid
int res_parser(char *response, double *exec_time, double *result)
{
	if(sscanf(response, "Calculation Response:\n\n$ %lg $ %lg $\n", exec_time, result) == 0)
		return -1;
	return 0;
}

// convert operation number to string value
char *optostring(int op)
{
	switch(op)
	{
		case 1:
			return "ADD";
		case 2:
			return "Subtract";
		case 3:
			return "Divide";
		case 4:
			return "Multiply";
		case 5:
			return "Sin";
		case 6:
			return "Cos";
		case 7:
			return "Tan";
		case 8:
			return "Cot";
		default:
			return NULL;
	}
}

// create reqeust message with operation and operands
int create_msg(int op, double *op1, double *op2, char *res)
{
	if(op1 == NULL)
	{
		fprintf(stderr, "invalid operands.\n");
		return -1;
	}
	else if(op2 == NULL)
		sprintf(res, "Calculation Request:\n\n$ %s $ %lg $\n", optostring(op), *op1);
	else
		sprintf(res, "Calculation Request:\n\n$ %s $ %lg $ %lg $\n", optostring(op), *op1, *op2);
	return 0;

}

int main(int argc, char *argv[])
{
	int op, port;
	double op1, op2;
	char response[MAXRESSIZE], msg[MAXMSGSIZE], *host = "127.0.0.1";

	// check arguments format
	if(argc == 2)
		port = atoi(argv[1]);
	else if(argc == 3)
	{
		port = atoi(argv[2]);
		strcpy(host, argv[1]);
	}
	else
	{
		fprintf(stderr, "usage: %s [host] port\n  host: the server host in dot-decimal default is 127.0.0.1\n  port: the server port\n", argv[0]);
		return EXIT_FAILURE;
	}

	// connect to the server
	int sockfd = sock_connect(host, port);

	printf("Connected to the server.\n");

	while(1)
	{
		// get operation and operand(s) from user
		printf("\nSelect one of the operations:\n1.Add\n2.Subtract\n3.Divide\n4.Multiply\n5.Sin\n6.Cos\n7.Tan\n8.Cot\n9.Exit\n");
		scanf("%d", &op);
		if(op <= 4)
		{
			printf("Enter operands:\n");
			scanf("%lg %lg", &op1, &op2);
			printf("%s %lg %lg\n", optostring(op), op1, op2);
			if(create_msg(op, &op1, &op2, msg) == -1)
				break;
		}
		else if(op <= 8)
		{
			printf("Enter operand:\n");
			scanf("%lg", &op1);
			printf("%s %lg\n", optostring(op), op1);
			if(create_msg(op, &op1, NULL, msg) == -1)
				break;
		}
		else if(op == 9)
			return EXIT_SUCCESS;
		else
		{
			fprintf(stderr, "invalid operation.\n");
			break;
		}

		// send request message to the socket
		printf("Sending calculation request to the server...\n");
		if(send(sockfd, (void *)msg, strlen(msg), 0) < 0)
		{
			perror("sending message");
			break;
		}

		// receive response from the socket and put in the buffer
		printf("Receiving response from the server...\n");
		if(recv(sockfd, response, MAXRESSIZE, 0) <= 0)
		{
			perror("receiving response");
			break;
		}
		double exec_time, result;
		if(res_parser(response, &exec_time, &result) == -1)
		{
			fprintf(stderr, "invalid response format.\n");
			return EXIT_FAILURE;
		}
		printf("Result %lg calculated in %lf s\n", result, exec_time);
	}
	return EXIT_SUCCESS;
}
