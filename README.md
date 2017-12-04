# Number-Guessing-Game

### Goal

In this assignment, you are expected to design and implement a client-server application for a multi-player number guessing game by using POSIX sockets in C. The server application randomly selects a number from 1 to 99. Clients have to guess the number by making guesses until one of them finds the chosen number.

### Scenario

You will implement a client - server application (client.c, server.c). Server and client programs will start only with the program’s name. In one terminal, server will be started and in other terminals clients will be started. New instances of terminals will be used to start new clients.
You should define a variable (e.g. #define playerCount 2) to test the application with variable player counts. The minimum number of players (clients) is 2 whereas there must be only one server. All client terminals and server terminal will be running in the same Debian machine.
The server application is to be continuous (should not end unless it is stopped or killed). It waits for the defined number of client connection to start a new game. It manages the game and give a turn to each player (client) to guess a number. The game ends when one of the players finds the chosen number. Then, the server waits for new players to start a new game. Socket messaging between the server and the clients is illustrated in following figure for the game with two-players.