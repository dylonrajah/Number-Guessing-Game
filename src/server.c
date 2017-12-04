/*
 ** server.c -- a stream socket server demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>

#define PORT "34490"  // the port users will be connecting to
#define PLAYERCOUNT 3     // how many pending connections queue will hold
#define NAMELEN 100

void INThandler(int); /* Interrupt handler */
int connection_handler(int sock, char *buff);

int myrecv(int sockfd, void *buff, int size, char *err_msg); /* recv function with error checking*/
int mysend(int sockfd, char *msg, int len, char *err_msg); /* send function with error checking*/
void game_loop(int p1_fd, int p2_fd, char *p1_name, char *p2_name);

int main_sockfd;

int main(void)
{
	signal(SIGINT, INThandler);

	struct addrinfo hints, *servinfo;
	int rv;

	int p1_sockfd;
	int p2_sockfd;
	char p1_name[NAMELEN];
	char p2_name[NAMELEN];

	int player_sockfd[PLAYERCOUNT];
	char player_name[PLAYERCOUNT][NAMELEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	while(1) {  // main loop
		if((main_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
			close(main_sockfd);
			perror("server: bind");
			exit(1);
		}

		if(bind(main_sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
			close(main_sockfd);
			perror("server: bind");
			exit(1);
		}

		freeaddrinfo(servinfo); // all done with this structure

		if(listen(main_sockfd, PLAYERCOUNT) == -1) {
			close(main_sockfd);
			perror("listen");
			exit(1);
		}

		printf("\n\nThe game will be started, waiting for %D players \n",
		PLAYERCOUNT);
		for(int i = 0; i < PLAYERCOUNT; i++) {
			player_sockfd[i] = connection_handler(main_sockfd, player_name[i]);
		}
		close(main_sockfd);

		game_loop(player_sockfd,player_name);

	}

	return 0;
}

int connection_handler(int sockfd, char *buff)
{
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size = sizeof(their_addr);
	int new_fd;

	new_fd = accept(main_sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(new_fd == -1) {
		perror("accept");
	}

	int numbytes;
	printf("Connection accepted..\n");
	// get player name
	numbytes = myrecv(new_fd, buff, NAMELEN, "get player name");

	buff[numbytes] = '\0';
	printf("%s logged in\n", buff);
	return new_fd;

}

void game_loop(int (*players_fd)[], char *players_names[][])
{
	printf("The game is starting \n");
	srand(time(NULL));
	int random_number = (rand() % 99) + 1;
	int guessed_number = 0, converted_number;

	for(int i = 0; i <PLAYERCOUNT; i++) {
		mysend(players_fd[i], "S", 2, "p1 S send"); /* inform player 1 to game is starting */
	}

	sleep(1);
	int round = 1;

	while(1) {

		for(int i = 0; i < PLAYERCOUNT; i++) {

			printf("\nRound %d, %s's turn\n", round, players_names[i]);
			mysend(players_fd[i], "R", 1, "p1 R send"); // send R char to wake player 1

			myrecv(p1_fd, &guessed_number, sizeof(int), "player  guessed number");
			converted_number = ntohl(guessed_number);
			printf("%s guessed %d\n", p1_name, converted_number);

			if(converted_number == random_number) {
				printf("%s won the game\n", p1_name);
				mysend(p1_fd, "=", 1, "p1 = send");
				mysend(p2_fd, "-", 1, "p2 - send");
				return;
			} else if(converted_number > random_number) {
				mysend(p1_fd, ">", 1, "p1 > send");
			} else if(converted_number < random_number) {
				mysend(p1_fd, "<", 1, "p1 < send");
			} else {
				printf("error at %d\n", __LINE__);
				exit(1);
			}
		}
		myrecv(p1_fd, &guessed_number, sizeof(int), "p1 recv guessed number");
		converted_number = ntohl(guessed_number);
		printf("%s guessed %d\n", p1_name, converted_number);

		if(converted_number == random_number) {
			printf("%s won the game\n", p1_name);
			mysend(p1_fd, "=", 1, "p1 = send");
			mysend(p2_fd, "-", 1, "p2 - send");
			return;
		} else if(converted_number > random_number) {
			mysend(p1_fd, ">", 1, "p1 > send");
		} else if(converted_number < random_number) {
			mysend(p1_fd, "<", 1, "p1 < send");
		} else {
			printf("error at %d\n", __LINE__);
			exit(1);
		}

		printf("Round %d, %s's turn \n", round, p2_name);
		mysend(p2_fd, "R", 1, "p1 R send"); // send R char to wake player 2

		myrecv(p2_fd, &guessed_number, sizeof(int), "p2 recv guessed number");
		converted_number = ntohl(guessed_number);
		printf("%s guessed %d\n", p2_name, converted_number);

		if(converted_number == random_number) {
			printf("%s won the game\n", p2_name);
			mysend(p1_fd, "-", 1, "p1 - send");
			mysend(p2_fd, "=", 1, "p2 = send");
			return;
		} else if(converted_number > random_number) {
			mysend(p2_fd, ">", 1, "p2 > send");
		} else if(converted_number < random_number) {
			mysend(p2_fd, "<", 1, "p2 < send");
		} else {
			printf("error at %d\n", __LINE__);
			exit(1);
		}

		round++;
	}

}

int mysend(int sockfd, char *msg, int len, char *err_msg)
{
	int bytes_send = 0;
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

void INThandler(int sig)
{
	printf("    closing main socket: %d\n", main_sockfd);
	close(main_sockfd);
	exit(0);
}

