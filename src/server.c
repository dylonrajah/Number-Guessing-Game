/*
 ** HASAN HÜSEYİN PAY 3.12.2017
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>

#define PORT "34490"  // the port users will be connecting to
#define PLAYERCOUNT 3
#define NAMELEN 100   // player max names length

/*
 * Interrupt handler to exit with CTRL-C
 */
void INThandler(int);

/*
 * get the server sockfd, fill the buff with player name, return player sockfd
 */
int connection_handler(int sockfd, char *buff);

/*
 * send and recv function with error checking
 */
int myrecv(int sockfd, void *buff, int size, char *err_msg);

int mysend(int sockfd, char *msg, int len, char *err_msg);

/*
 * game play in game_loop
 */
void game_loop(int *players_fd, char players_names[PLAYERCOUNT][NAMELEN]);

int server_sockfd;

int main(void)
{
        signal(SIGINT, INThandler);

        int player_sockfd[PLAYERCOUNT];
        char player_name[PLAYERCOUNT][NAMELEN];
        struct addrinfo hints, *servinfo;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        int rv;
        if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                return 1;
        }
        while (1) {  // main loop
                if ((server_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
                        close(server_sockfd);
                        perror("server: socket");
                        exit(1);
                }

                if (bind(server_sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
                        close(server_sockfd);
                        perror("server: bind");
                        exit(1);
                }

                if (listen(server_sockfd, PLAYERCOUNT) == -1) {
                        close(server_sockfd);
                        perror("server: listen");
                        exit(1);
                }

                printf("\n\nThe game will be started, waiting for %d players \n", PLAYERCOUNT);

                for (int i = 0; i < PLAYERCOUNT; i++) {
                        player_sockfd[i] = connection_handler(server_sockfd, player_name[i]); // get the player sockfd
                }
                close(server_sockfd); // no need anymore

                game_loop(&player_sockfd[0], player_name); // game start

                for (int i = 0; i < PLAYERCOUNT; i++) // close player sockfds
                        close(player_sockfd[0]);
        }

        return 0;
}

int connection_handler(int sockfd, char *buff)
{
        struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size = sizeof(their_addr);
        int new_fd; // sockfd of incoming conneciton

        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);
        if (new_fd == -1) {
                perror("accept");
        }

        int numbytes;
        printf("Connection accepted..\n");

        // get player name
        numbytes = myrecv(new_fd, buff, NAMELEN - 1, "get player name");

        buff[numbytes] = '\0';
        printf("%s logged in\n", buff);
        return new_fd;

}

void game_loop(int *players_fd, char players_names[PLAYERCOUNT][NAMELEN])
{
        printf("The game is starting \n");
        srand(time(NULL));

        int random_number = (rand() % 99) + 1;
        int guessed_number = 0, converted_number;

        for (int i = 0; i < PLAYERCOUNT; i++) {
                mysend(players_fd[i], "S", 1, "p1 S send"); /* inform player 1 to game is starting */
        }

        sleep(1);
        int round = 1;

        while (1) {

                printf("\n");
                for (int i = 0; i < PLAYERCOUNT; i++) {

                        printf("Round %d, %s's turn\n", round, players_names[i]);
                        mysend(players_fd[i], "R", 1, "p1 R send"); // send R char to wake player 1

                        myrecv(players_fd[i], &guessed_number, sizeof(int), "player  guessed number");
                        converted_number = ntohl(guessed_number);
                        printf("%s guessed %d\n", players_names[i], converted_number);

                        if (converted_number == random_number) {
                                printf("%s won the game\n", players_names[i]);
                                for (int j = 0; j < PLAYERCOUNT; j++) {
                                        if (i == j)
                                                mysend(players_fd[j], "=", 1, "= send");
                                        else
                                                mysend(players_fd[j], "-", 1, "- send"); // inform player
                                }
                                return;
                        } else if (converted_number > random_number) {
                                mysend(players_fd[i], ">", 1, " > send");
                        } else if (converted_number < random_number) {
                                mysend(players_fd[i], "<", 1, " < send");
                        } else {
                                printf("error at %d\n", __LINE__);
                                exit(1);
                        }
                }
                round++;
        }

}

int mysend(int sockfd, char *msg, int len, char *err_msg)
{
        int bytes_send = 0;
        if ((bytes_send = (int) send(sockfd, msg, (size_t) len, 0)) == -1) {
                perror("send");
                printf("%s at %d\n", err_msg, __LINE__);
                exit(0);
        }
        return bytes_send;
}

int myrecv(int sockfd, void *buff, int len, char *err_msg)
{
        int numbytes;
        if ((numbytes = (int) recv(sockfd, buff, (size_t) len, 0)) == -1) {
                perror("recv");
                printf("%s at %d\n", err_msg, __LINE__);
                exit(1);
        }
        return numbytes;
}

void INThandler(int sig)
{
        printf("    closing server socket: %d\n", server_sockfd);
        close(server_sockfd);
        exit(0);
}

