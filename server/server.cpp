#include <stdio.h>
#include <thread>
#include "../include/socketserver.h"
#include "../include/Semaphore.h"

/*
struct {

    ipAddress
    port
    socket

} Client;

struct {

    Client player1;
    Client player2;
} GameSession;
*/

using namespace std;
using namespace Sync;

struct Client{
    string ipAddress = "127.0.0.1"; //"<>.<>.<>"
    int port;
    Socket socket;
};

struct GameSession{
    Client player1;
    Client player2;
};

void threadSession(GameSession gameSem);
void clientHandlerThread(Semaphore *gameSem, Client *client, int *answer, bool *clientGuessedCorrectly, int *portFirst);

/*
main func {
    TODO: Michael
    vector<threadSession> threadSessions;
    timerThread;
    timerStarted = false;

    while loop {

        on new client
            client = new Client
            
            if timerStarted == false
                new gameSession
                gameSession.player1 = client
                timerThread(gameSession)
                timerStarted = true
            else 
                gameSession.player2 = client
                timerThread.join()
                timerStarted = false
                create threadSession(gameSession)
    return

    }
}

timerThread func (gameSession) {
    TODO: Michael
    start time_elapsed_tracker

    while loop for 60 seconds {

        if gameSession.player2 != null
            return
    }

    close client socket
    
} */

void threadSession(GameSession gameSession) {

    Client *client1 = &gameSession.player1;
    Client *client2 = &gameSession.player2;

    // start game message  
    string initGameMsg = "Init";
    // start game 1
    client1->socket.Write(ByteArray(initGameMsg));
    // start game 2
    client2->socket.Write(ByteArray(initGameMsg));

    int answer = rand() % 10 + 1;
    bool player1Correct = false;
    bool player2Correct = false;
    int portFirst = -1;
    Semaphore gameSem = Semaphore("gameSem", 0, true);

    clientHandlerThread(&gameSem, client1, &answer, &player1Correct, &portFirst);
    clientHandlerThread(&gameSem, client2, &answer, &player2Correct, &portFirst);

    if (player1Correct && player2Correct) {
        if (portFirst == client1->port) {
            // player 1 wins
            client1->socket.Write(ByteArray("You win!"));
            client2->socket.Write(ByteArray("You lose!"));
        } else {
            // player 2 wins
            client1->socket.Write(ByteArray("You lose!"));
            client2->socket.Write(ByteArray("You win!"));
        }
    } else if (player1Correct && !player2Correct) {
        // player 1 wins
        client1->socket.Write(ByteArray("You win!"));
        client2->socket.Write(ByteArray("You lose!"));
    } else if (player2Correct && !player1Correct) {
        // player 2 wins
        client1->socket.Write(ByteArray("You lose!"));
        client2->socket.Write(ByteArray("You win!"));
    } else {
        // both lose
        client1->socket.Write(ByteArray("You lose!"));
        client2->socket.Write(ByteArray("You lose!"));
    }

}

/*
clientHandlerThread func (gameSem, client, answer, clientGuessedCorrectly, portFirst) {
    TODO: Piotr

    on data from client

    gamSem.Pend()

    if answer == correct
        clientGuessedCorrectly = true 

        if portFirst == -1
            portFirst = client.port        
    else
        clientGuessedCorrectly = false

    gameSem.Post()
}
*/

void clientHandlerThread(Semaphore *gameSem, Client *client, int *answer, bool *clientGuessedCorrectly, int *portFirst) {

    // on data from client
    // TODO: implement getting data from client

    gameSem->Wait();

    if (*answer == 1) {
        *clientGuessedCorrectly = true;

        if (*portFirst == -1) {
            *portFirst = client->port;
        }
    } else {
        *clientGuessedCorrectly = false;
    }

    gameSem->Signal();
}

// add main driver function?