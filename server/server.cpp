#include <iostream>
#include <vector>
#include <thread>
#include "../include/socketserver.h"
#include "../include/Semaphore.h"
#include <string.h>
#include <chrono>

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

struct GameSession{
    Socket player1;
    Socket player2;

};

void threadSession(GameSession gameSem);
void clientHandlerThread(Semaphore *gameSem, Client *client, int *answer, bool *clientGuessedCorrectly, int *portFirst);
void timerThread(GameSession *gameSession);

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

int main(void) {

    vector<thread> threadSessions;
    bool timerStarted = false;
    thread timerThread;

    SocketServer server = SocketServer(2000);

    while (1) {

        auto newGameSession = make_shared<GameSession>();
        Socket clientSocket(server.Accept());
        thread timerTh;

        if (!timerStarted) {
            newGameSession->player1 = clientSocket;
            timerTh = thread(timerThread, newGameSession); // Fix: Pass the address of the timerThread function
            timerStarted = true;
        } else {
            newGameSession->player2 = clientSocket;
            timerTh.join();
            timerStarted = false;
            threadSessions.push_back(thread(threadSession, newGameSession));
        }

    
    }

}

void timerThread(GameSession *gameSession) {

    auto start = chrono::high_resolution_clock::now();

    int timeElapsed = 0;
    // fix 
    while (timeElapsed < 60) {
        if (gameSession->player2 != nullptr) {
            return;
        }
        auto end = chrono::high_resolution_clock::now();
        timeElapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
    }

    gameSession->player1.socket.Close();
}

void threadSession(GameSession *gameSession) {

    Client& client1 = gameSession->player1;
    Client& client2 = gameSession->player2;

    // start game message  
    string initGameMsg = "Init";
    // start game 1
    client1.socket.Write(ByteArray(initGameMsg));
    // start game 2
    client2.socket.Write(ByteArray(initGameMsg));

    int answer = rand() % 10 + 1;
    bool player1Correct = false;
    bool player2Correct = false;
    int portFirst = -1;
    Semaphore gameSem = Semaphore("gameSem", 0, true);

    thread client1Thread(clientHandlerThread, &gameSem, client1, &answer, &player1Correct, &portFirst);
    thread client2Thread(clientHandlerThread, &gameSem, client2, &answer, &player2Correct, &portFirst);

    client1Thread.join();
    client2Thread.join();

    if (player1Correct && player2Correct) {
        if (portFirst == client1.port) {
            // player 1 wins
            client1.socket.Write(ByteArray("You win!"));
            client2.socket.Write(ByteArray("You lose!"));
        } else {
            // player 2 wins
            client1.socket.Write(ByteArray("You lose!"));
            client2.socket.Write(ByteArray("You win!"));
        }
    } else if (player1Correct && !player2Correct) {
        // player 1 wins
        client1.socket.Write(ByteArray("You win!"));
        client2.socket.Write(ByteArray("You lose!"));
    } else if (player2Correct && !player1Correct) {
        // player 2 wins
        client1.socket.Write(ByteArray("You lose!"));
        client2.socket.Write(ByteArray("You win!"));
    } else {
        // both lose
        client1.socket.Write(ByteArray("You lose!"));
        client2.socket.Write(ByteArray("You lose!"));
    }

    client1.socket.Close();
    client2.socket.Close();

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

    ByteArray buffer;

    Socket clientSocket = client->socket;

    int clientGuess;

    // Wait for a message from the client
    while (clientSocket.Read(buffer) == 0)
    {
        sleep(1);
    }

    gameSem->Wait();

    clientGuess = stoi(buffer.ToString());

    if (*answer == clientGuess) {
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