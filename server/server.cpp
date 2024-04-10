

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

main func {

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

    start time_elapsed_tracker

    while loop for 60 seconds {

        if gameSession.player2 != null
            return
    }

    close client socket
    
}

threadSession func (gameSession) {

    client1 = gameSession.player1
    client2 = gameSession.player2

    start player 1 game
    start player 2 game

    ... game logic -> first to guess wins

    answer = rng number between 1 to 10

    bool player1Correct = False;
    bool player2Correct = False;
    portFirst = -1;
    gameSem = new Semaphore

    clientHandlerThread(gameSem, client1, answer, player1Correct, portFirst)
    clientHandlerThread(gameSem, client2, answer, player2Correct, portFirst)

    check if player 1 or player 2 got it correct

        if both got correct check which one grabbed gameSem first

        if both lose too bad so sad

    provide results to player1 and player2 and end their suffering

}

clientHandlerThread func (gameSem, client, answer, clientGuessedCorrectly, portFirst) {

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