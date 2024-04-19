#include <iostream>
#include <vector>
#include <thread>
#include "../include/socketserver.h"
#include "../include/Semaphore.h"
#include "../include/socket.h"
#include <string.h>
#include <chrono>
#include <memory>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;
using namespace Sync;

struct Client {
    shared_ptr<Socket> socket;
    int port;
};
struct GameSession {
    shared_ptr<Client> player1;
    shared_ptr<Client> player2;
    bool hasJoined = false;
};

void threadSession(shared_ptr<GameSession> gameSession);
void clientHandlerThread(Semaphore *gameSem, shared_ptr<Client> client, int *answer, bool *clientGuessedCorrectly, int *portFirst);
void timerThread(shared_ptr<GameSession> gameSession, bool *player1Joined);
void runServer(bool *shutdown, vector<shared_ptr<GameSession>> gameSessions, shared_ptr<SocketServer> server, vector<shared_ptr<thread>> threadSessions);
void shutdownServer(vector<shared_ptr<GameSession>> gameSessions, vector<shared_ptr<thread>> threadSessions, vector<shared_ptr<thread>> timerThreads);
/**
 * Timer Thread() - This function is responsible for timing the player's connection to the server.
 * @params shared_ptr<GameSession> gameSession - GameSession object to handle the game session between two players
 * @params bool *player1Joined - A boolean to check if player 1 has joined the lobby
*/
void timerThread(shared_ptr<GameSession> gameSession, bool *player1Joined) {
    try{
        auto start = chrono::high_resolution_clock::now();
        int passedTime = 0;
        int timeElapsed = 0;

        while (timeElapsed < 10) {
            // Checking to see if player 2 has joined the lobby
            if (gameSession->hasJoined) {
                return;
            }
            auto end = chrono::high_resolution_clock::now();
            timeElapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
            
            // Printing Out the Time Elapsed
            if (timeElapsed != passedTime) {
                cout << "[Timer Thread] - Time Elapsed: " << timeElapsed << endl; // Debugging
                passedTime = timeElapsed;
            }
        }
        // Player 2 Failed to join in time
        cout << "[Timer Thread] - Closing Client 1 Socket" << endl;
        *player1Joined = false;
        gameSession->player1->socket->Close();
        return;
    } catch (exception e) {
        cerr << "[Timer Thread] - Error in timerThread: " << e.what() << endl;
    }
}

/**
 * Thread Session() - This function is responsible for handling the game session between two players.
 * @params shared_ptr<GameSession> gameSession - GameSession object to handle the game session between two players
*/
void threadSession(shared_ptr<GameSession> gameSession) {
    shared_ptr<Client> player1 = gameSession->player1;
    shared_ptr<Client> player2 = gameSession->player2;

    // start game message  
    string initGameMsg = "Init";
    // start game 1
    player1->socket->Write(ByteArray(initGameMsg));
    // start game 2
    player2->socket->Write(ByteArray(initGameMsg));

    int answer = rand() % 10 + 1;

    cout << "[Server] - Answer: " << answer << endl;

    bool player1Correct = false;
    bool player2Correct = false;
    int portFirst = -1;
    Semaphore gameSem = Semaphore("gameSem", 1, true);

    thread player1Thread(clientHandlerThread, &gameSem, player1, &answer, &player1Correct, &portFirst);
    thread client2Thread(clientHandlerThread, &gameSem, player2, &answer, &player2Correct, &portFirst);

    player1Thread.join();
    client2Thread.join();

    if (player1Correct && player2Correct) {
        // Check to see which player guessed first
        if (portFirst == player1->port) {
            // player 1 wins
            player1->socket->Write(ByteArray("You win!"));
            player2->socket->Write(ByteArray("You lose!"));
            cout << "[Server] - Player 1 wins" << endl;
        } else {
            // player 2 wins
            player1->socket->Write(ByteArray("You lose!"));
            player2->socket->Write(ByteArray("You win!"));
            cout << "[Server] - Player 2 wins" << endl;
        }
    } else if (player1Correct && !player2Correct) {
        // player 1 wins
        player1->socket->Write(ByteArray("You win!"));
        player2->socket->Write(ByteArray("You lose!"));
        cout << "[Server] - Player 1 wins" << endl;
    } else if (player2Correct && !player1Correct) {
        // player 2 wins
        player1->socket->Write(ByteArray("You lose!"));
        player2->socket->Write(ByteArray("You win!"));
        cout << "[Server] - Player 2 wins" << endl;
    } else {
        // both lose
        player1->socket->Write(ByteArray("You lose!"));
        player2->socket->Write(ByteArray("You lose!"));
        cout << "[Server] - Both players lose" << endl;
    }

    player1->socket->Close();
    player2->socket->Close();

    gameSession->hasJoined = false;

}

/**
 * Client Handler Thread() - This function is responsible for handling each Players's guess and checking if the player guessed correctly.
 * @param Semaphore *gameSem - Semaphore object to lock the critical section
 * @param shared_ptr<Client> client - Client object to handle the client's guess
 * @param int *answer - The correct answer to the game
 * @param bool *clientGuessedCorrectly - A boolean to check if the client guessed correctly
 * @param int *portFirst - The port number of the client who guessed first
*/
void clientHandlerThread(Semaphore *gameSem, shared_ptr<Client> client, int *answer, bool *clientGuessedCorrectly, int *portFirst) {
    // on data from client
    ByteArray buffer;
    shared_ptr<Socket> clientSocket = client->socket;

    int clientGuess;

    clientSocket->Read(buffer);

    //Print Out the Buffer
    cout << "[Client Handler Thread] - Buffer: " << buffer.ToString() << endl;

    gameSem->Wait(); 

    clientGuess = stoi(buffer.ToString());

    if (*answer == clientGuess) { // If the Answer is correct
        *clientGuessedCorrectly = true;

        if (*portFirst == -1) { // If the player is first to guess
            *portFirst = client->port;
        }
    } else {
        *clientGuessedCorrectly = false;
    }

    gameSem->Signal();
}

/**
 * Main() - This function is responsible for starting the server and handling the game sessions between two players.
 * @returns 0 - If the server runs successfully
*/
// int main(void) {
void runServer(bool *shutdown, vector<shared_ptr<GameSession>> gameSessions, shared_ptr<SocketServer> server, vector<shared_ptr<thread>> threadSessions) {
    // vector<shared_ptr<thread>> threadSessions;
    shared_ptr<Socket> waiting_client;
    Socket *clientSocket;
    // vector<shared_ptr<GameSession>> gameSessions;
    shared_ptr<GameSession> gameSession;
    bool player1Joined = false;

    cout << "[Server] - Starting Server Socket at 2000" << endl;
    

    while (!*shutdown) {
        cout << "[Server] - Waiting on Connection from Client" << endl;
        clientSocket = new Socket(server->Accept());
        cout << "[Server] - Connection Established" << endl;
        if (waiting_client == nullptr) {
            // gameSession = make_shared<GameSession>();
            gameSessions.push_back(make_shared<GameSession>());
            cout << "[Server] - Player 1 has joined" << endl;
            waiting_client = make_shared<Socket>(*clientSocket);
        }
        thread timerTh;

        try {
            if (!player1Joined) { // Player 1 Enters the lobby
                cout << "[Server] - Player 1 has joined, Starting TimerThread" << endl;
                gameSession = gameSessions.back();
                timerTh = thread(timerThread, gameSession, &player1Joined);
                timerTh.detach();
                player1Joined = true;
                cout << "[Server] - Player1Joined: " << boolalpha << player1Joined << endl;
            } else { // Player 2 enters the lobby
                gameSession = gameSessions.back();
                gameSession->hasJoined = true;
                gameSession->player1 = make_shared<Client>();
                gameSession->player1->socket = waiting_client;
                gameSession->player2 = make_shared<Client>();
                gameSession->player2->socket = make_shared<Socket>(*clientSocket);
                cout << "[Server] - Player 2 has been found" << endl;
                // if (timerTh.joinable()) {
                //     cout << "[Server] - Ending thread" << endl;
                //     timerTh.join();
                // }
                player1Joined = false;
                threadSessions.push_back(make_shared<thread>(threadSession, gameSession));
                cout << "[Server] - Thread Session Size: " << threadSessions.size() << endl;
                waiting_client = NULL; // remove player 1 from the waiting room
            }
        } catch (exception e) {
            cout << "[Server] - Error: " << e.what() << endl;
            return;
        }

    }

}

void shutdownServer(vector<shared_ptr<GameSession>> gameSessions, vector<shared_ptr<thread>> threadSessions, vector<shared_ptr<thread>> timerThreads) {
    for (auto &session : gameSessions) {
        if (session->player1->socket) {
            session->player1->socket->Close();
        }
        if (session->player2->socket) {
            session->player2->socket->Close();
        }
    }
    for (auto &thread : threadSessions) {
        if (thread->joinable()) {
            cout << "[Shutdown] - Ending session thread" << endl;
            thread->join();
        }
    }

    for (auto &thread : timerThreads) {
        if (thread->joinable()) {
            cout << "[Shutdown] - Ending timer thread" << endl;
            thread->join();
        }
    }
}

int main(void) {

    vector<shared_ptr<GameSession>> gameSessions;
    vector<shared_ptr<thread>> threadSessions;
    shared_ptr<SocketServer> server = make_shared<SocketServer>(2000);

    pid_t cpid = fork();
    bool shutdown = false;

    if (cpid == 0)
    {
        // Child process waits for input to shutdown the server
        std::cin.get();
        // shutdown = true;
        // server->Shutdown();
        // shutdownServer(gameSessions, threadSessions, timerThreads);
        cout << "parent" << endl;
        
        kill(0, SIGTERM);
    }
    else if (cpid > 0)
    {
        // Parent process will continue to run the server
        // Need a thread to perform server operations
        runServer(&shutdown, gameSessions, server, threadSessions);

        // This will wait for input to shutdown the server
        FlexWait cinWaiter(1, stdin);
        cinWaiter.Wait();

        kill(0, SIGTERM);
    }

    return 0;
}