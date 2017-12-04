/*
 ** client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "34490" // the port client will be connecting to
#define NAMELEN 100 // max number of bytes we can get at once
#define SERVERIP "127.0.0.1"

int myrecv(int sockfd, void *buff, int size, char *err_msg); /* recv function with error checking*/
int mysend(int sockfd, char *msg, int len, char *err_msg); /* send function with error checking*/

int main(int argc, char *argv[])
{
	char name[NAMELEN];
	int main_sockfd;
	struct addrinfo hints, *servinfo;
	int rv;
	char receive = '0';

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(SERVERIP, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	if((main_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol)) == -1) {
		perror("client: socket");
		exit(1);
	}

	if(connect(main_sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		perror("client: connect");
		close(main_sockfd);
		exit(1);
	}

	printf("Connected\n");
	printf("Welcome to the number guessing game, please login to play\n");
	printf("Player name: ");
	scanf("%s", name);

	mysend(main_sockfd, name, strlen(name), "name");
	myrecv(main_sockfd, &receive, 1, "receive start");
	if(receive != 'S') {
		printf("Error occur at Start message");
		exit(1);
	}

	printf("\nThe game is starting\n");
	int guessed_number, converted_number;
	while(1) {
		myrecv(main_sockfd, &receive, 1, "ROUND");
		if(receive == '-') {
			printf("The game is over\n");
			close(main_sockfd);
			exit(0);
		} else if(receive != 'R')
			continue;

		printf("\nIt’s your turn\n");
		printf("Guess a number: ");
		scanf("%d", &guessed_number);

		converted_number = htonl(guessed_number);
		mysend(main_sockfd, &converted_number, sizeof(uint32_t),
				"guessed number");

		myrecv(main_sockfd, &receive, 1, "receive start");
		switch(receive) {
		case '=':
			printf("Well done, your answer is correct\n");
			printf("The game is over\n");
			sleep(2);
			close(main_sockfd);
			exit(0);
			break;
		case '<':
			printf("Too low, try a bigger number\n");
			break;
		case '>':
			printf("Too high, try a smaller number\n");
			break;
		default:
			printf("error in switch case\n");
			close(main_sockfd);
			exit(1);
			break;
		}

	}

	return 0;
}

int mysend(int sockfd, char *msg, int len, char *err_msg)
{
	int bytes_send;
	if((bytes_send = send(sockfd, msg, len, 0)) == -1) {
		perror("send");
		printf("%s at %d\n", err_msg, __LINE__);
		exit(0);
	}
	return bytes_send;
}

int myrecv(int sockfd, void *buff, int len, char *err_msg)
{
	int numbytes;
	if((numbytes = recv(sockfd, buff, len, 0)) == -1) {
		perror("recv");
		printf("%s at %d\n", err_msg, __LINE__);
		exit(1);
	}
	return numbytes;
}
