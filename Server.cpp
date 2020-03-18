#include "Server.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include<vector>
#include <stdlib.h>
#include <stdio.h>
#include<time.h>


const int CONNECTION_REQUEST_SIZE = 10;
//rule: no insurance, no double, no split, no surrender

bool playerOneJoined;
string playerOneName;
int playerOneSockFileDesc;

bool playerTwoJoined;
string playerTwoName;
int playerTwoSockFileDesc;

using namespace std;

struct thread
{
    int id;
    int fileDesc;
};

Server::Server()
{

}

string Server::pickCard()
{
    srand(time(0));
    int index = rand() % cards.size();// pick a random index
    string value = cards[index];
    cards.erase(cards.begin()+index);//remove
    return value;
}

void Server::processDealer()
{
    dealerCards.push_back(pickCard());
    dealerCards.push_back(pickCard());
    cout << "test dealer: " << endl;
    cout << "Player cant see in game ||| dealer scroe:" << countCards(dealerCards) << endl;
    if (countCards(dealerCards) < 17)
    {
        cout << "Player cant see in game ||| Dealer has less than 17, get new card " << endl;
        dealerCards.push_back(pickCard());
    }
    cout << "Player cant see in game ||| ";
    for (int i = 0;i < dealerCards.size();i++)
    {
        cout << dealerCards.at(i) << " ";
    }
    cout << "dealer scroe:" << countCards(dealerCards) << endl;
    dealerScore = countCards(dealerCards);
}

int Server::countCards(vector <string> temp)
{
    int count=0;
    int countA = 0;
    for (int i = 0;i < temp.size();i++)
    {
        if (temp.at(i) == "J" ||
            temp.at(i) == "Q" ||
            temp.at(i) == "K" )
        {
            count += 10;
        }
        else if (temp.at(i) == "A")
        {
            countA++;
        }
        else
        {
            count += stoi(temp.at(i));
        }
    }
    
    if (countA > 0)
    {
        for (int i = 0;i < countA;i++)
        {
            if (count + 11 > 21)
            {
                count += 1;
            }
            else
            {
                count += 11;
            }
        }
    }
    return count;
}

void Server::startRound()
{
    initialCard();
    round += 1;
    cout << "Round " << round << endl;
    processDealer();
}
//---------------------------------------------------------------
//---------------------------------------------------------------

string getSocketMessage(int socketFileDescriptor){
    string responseHeader = "";
    char lastChar = 0;
    while ( true )
    {
        char currentChar = 0;
        recv(socketFileDescriptor , &currentChar , 1 , 0);
        if (currentChar == '.' ){
            break;
        }
        else responseHeader += currentChar;
        lastChar = currentChar;
    }
    return responseHeader;
}

void *serverResponsePlayer(void *threadData) {
    struct thread *data;
    data = (struct thread *) threadData;
    bool isGET = false;

    //parses response from client
    string responseHeader = "";
    char lastChar = 0;
    bool lookForName = false;
    string currentName = "";
    while (true) {
        char currentChar = 0;
        recv(data->fileDesc, &currentChar, 1, 0);
        if (lookForName && currentChar != '1') {
            if (currentChar == ' ') {
                lookForName = false;
            } else {
                currentName += currentChar;
            }

        }
        if (currentChar == '1') {
            lookForName = true;
        }
        if (currentChar == '.') {
            break;
        } else responseHeader += currentChar;
        lastChar = currentChar;
    }
    cout << "This is what the server recieved: " + responseHeader << endl;
    string statusCode;
    //server will reply to player

    //this is for when the 2 players initially join
    cout << "current player name is: " + currentName << endl;
    if (playerOneJoined == false) {
        playerOneName = currentName;
        playerOneSockFileDesc = data->fileDesc;
        playerOneJoined = true;
    } else if (playerTwoJoined == false) {
        playerTwoName = currentName;
        playerTwoSockFileDesc = data->fileDesc;
        playerTwoJoined = true;
    }
    cout << "Current Players in game(limited to two):" << endl;
    if (playerOneJoined) {
        cout << "1. " + playerOneName << endl;
    }
    if (playerTwoJoined) {
        cout << "2. " + playerTwoName << endl;
    }

    if (responseHeader[0] == '1') { //if response starts with a one then a player has joined the game
        cout << responseHeader.substr(1, responseHeader.length()) << endl;
    }

    //this is the game loop where the game truly starts between the two players
    bool gameEnd = false;
    vector<int> cards;
    vector<int> playerOneCards;
    vector<int> playerTwoCards;
    vector<int> dealerCards;
    string possibleCards[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"}; //All possible cards

    while (playerOneJoined && playerTwoJoined) {
        cout << "Starting Game.." << endl;
        //start by dealing the first card on the table
        srand(time(0));
        int dealerCard1 = rand() % 13;
        int dealerCard2 = rand() %
                          13; //this card is hidden from the players and will only be revealed once players exceed combined amount.
        int card1 = rand() % 13;
        int card2 = rand() % 13;
        int card3 = rand() % 13;
        int card4 = rand() % 13;
        if (dealerCard1 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
            dealerCard1 = 10;
        }
        if (card1 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
            card1 = 10;
        }
        if (card2 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
            card2 = 10;
        }
        if (card3 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
            card3 = 10;
        }
        if (card4 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
            card4 = 10;
        }
        cards.push_back(dealerCard1);
        string message = "Dealers first card: " + possibleCards[dealerCard1] + ".";
        //add to dealer card stack
        dealerCards.push_back(dealerCard1);

        //send message of 1st dealer card to players
        send(playerOneSockFileDesc, &message[0], message.size(), 0); // send message to player1
        send(playerTwoSockFileDesc, &message[0], message.size(), 0); // send message to player2

        cout << "Dealer is handing out individual cards" << endl;
        //send second message to each player with their own hand
        string message1 = "You have drawn a " + possibleCards[card1] + " and a " + possibleCards[card2] + ".";
        send(playerOneSockFileDesc, &message1[0], message1.size(), 0); // send message to player1
        playerOneCards.push_back(card1);
        playerOneCards.push_back(card2);

        string message2 = "You have drawn a " + possibleCards[card3] + " and a " + possibleCards[card4] + ".";
        send(playerTwoSockFileDesc, &message2[0], message2.size(), 0); // send message to player2
        playerTwoCards.push_back(card3);
        playerTwoCards.push_back(card4);

        int dealerTotal = 0;
        while(true) {//loop for current game

            //waiting to see if player will HIT or STAND--------------------------------------------------------------
            int cardTemp;
            string playerOneMove = getSocketMessage(playerOneSockFileDesc);//grabbing player one's message
            if (playerOneMove == "1") { //1 = HIT, 2 = STAND
                cout << playerOneName + " has decided to hit." << endl;
                cardTemp = rand() % 13;
                playerOneCards.push_back(cardTemp);
                //send player message on which card they had drawn.
                string temp = "You have drawn a " + possibleCards[cardTemp] + ".";
                send(playerOneSockFileDesc, &temp[0], temp.size(), 0);
            } else {
                cout << playerOneName + " has decided to stand." << endl;
            }

            string playerTwoMove = getSocketMessage(playerTwoSockFileDesc); //grabbing player twos message
            if (playerTwoMove == "1") { //1 = HIT, 2 = STAND
                cout << playerTwoName + " has decided to hit." << endl;
                cardTemp = rand() % 13;
                playerTwoCards.push_back(cardTemp);
                //send player message which card they had drawn
                string temp = "You have drawn a " + possibleCards[cardTemp] + ".";
                send(playerTwoSockFileDesc, &temp[0], temp.size(), 0);
            } else {
                cout << playerTwoName + " has decided to stand." << endl;
            }

            for(int i = 0;i<dealerCards.size();i++){
                dealerTotal+=dealerCards[i]+1;
            }


            if(dealerTotal < 17){// if dealer has less than 17 the dealer will HIT
                cardTemp = rand() % 13;
                dealerTotal += cardTemp + 1;
                message = "Dealer has decided to hit.";
            }else{
                message = "Dealer has decided to stand.";
            }
            //send dealer actino to players
            send(playerOneSockFileDesc, &message[0], message.size(), 0); // send message to player1
            send(playerTwoSockFileDesc, &message[0], message.size(), 0); // send message to player2
            cout << message << endl;//this is message to server console to keep track of the game

            //game check to see if someone has lost------------------------------------------------------------

            //player1 check
            int playerOneTotal = 0;
            for (int i = 0; i < playerOneCards.size(); i++) {
                playerOneTotal += playerOneCards[i] + 1; //add one for index disparity
                //cout << "1adding " <<playerOneCards[i] << " to " +playerOneName + " total count." << endl;
            }
            cout << playerOneName + " total is: " << playerOneTotal << endl;

            //player2 check
            int playerTwoTotal = 0;
            for (int i = 0; i < playerTwoCards.size(); i++) {
                playerTwoTotal += playerTwoCards[i] + 1;
                //cout << "2adding " <<playerTwoCards[i] << " to " +playerTwoName + " total count." << endl;
            }
            cout << playerTwoName + " total is: " << playerTwoTotal << endl;



            //if someone has lost then send endgame message to player-------------------------------------
            cout << "Dealer total is: " << dealerTotal << endl;
            string dealerInfo;
            if (playerOneTotal > 21 || dealerTotal == 21) { //losing from total amount
                message = "1 dealer has drawn a "+to_string(dealerTotal)+".";//1 = lose
                gameEnd = true;
            } else if (((playerOneTotal > dealerTotal) && (dealerTotal > 17)) || (dealerTotal > 21)) {
                message = "2 dealer has drawn a "+to_string(dealerTotal)+".";// dealer lose
                gameEnd = true;
            } else {
                message = "3."; //2 = able to keep playing

            }

            send(playerOneSockFileDesc, &message[0], message.size(), 0);//send msg for player1



            if (playerTwoTotal > 21 || dealerTotal == 21) { //losing from total amount
                message = "1dealer has drawn a "+to_string(dealerTotal)+".";//1 = lose
                gameEnd = true;
            } else if(((playerOneTotal > dealerTotal) && (dealerTotal > 17)) || (dealerTotal > 21)){
                message = "2dealer has drawn a "+to_string(dealerTotal)+".";// dealer lose
                gameEnd = true;
            } else {
                message = "3."; //2 = able to keep playing

            }

            cout << "message is: " + message << endl;
            send(playerTwoSockFileDesc, &message[0], message.size(), 0); //send msg for player2


            if(gameEnd) {
                break;
            }
        }
        cout << "Game is over, server will now close" << endl;
        exit(0);
        //ask if players want to play again-----------------------------------------
        /*
        cout << "getting options" << endl;
        string option1 = getSocketMessage(playerOneSockFileDesc);
        string option2 = getSocketMessage(playerTwoSockFileDesc);
        cout << "option: " << option1 << endl;
        cout << "option2: " << option2 << endl;
        if(option1 == "1" && option2 == "1"){
            message = "1Both players want to play again.";
            cout << message << endl;
            gameEnd = false;
        }else{
            message = "2One player has quit the game, the game will now end. \n";
            cout << message << endl;
            gameEnd = true;
        }
        send(playerOneSockFileDesc, &message[0], message.size(), 0); //send msg for player2
        send(playerTwoSockFileDesc, &message[0], message.size(), 0); //send msg for player2

        if(gameEnd){
            break;
        }
         */
    }

    //close(data->fileDesc);
    return 0;
}


int main(int argumentNumber , char *argumentValues[])
{
    playerOneJoined = false;
    playerTwoJoined = false;
    cout << "Waiting for players to join the game.." << endl;
    if ( argumentNumber != 2 ) // Change this to
    {
        cout << "Invalid number of argument. The program does not accept any argument at all";
        return -1;
    }
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    memset(&hints , 0 , sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 or v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int addressInfoStatus = getaddrinfo(nullptr , argumentValues[ 1 ] , &hints , &serverInfo);
    if ( addressInfoStatus != 0 )
    {
        cout << "Unable to connect";
        cout << gai_strerror(addressInfoStatus);
        return -1;
    }
    int socketFileDescriptor;
    int serverBindResult;
    struct addrinfo *possibleConnection;
    // go through all possible connection server can use. Use the first one that is appropriate
    for ( possibleConnection = serverInfo;possibleConnection != nullptr; possibleConnection = possibleConnection->ai_next )
    {
        socketFileDescriptor = socket(possibleConnection->ai_family , possibleConnection->ai_socktype ,
                                      possibleConnection->ai_protocol);
        if ( socketFileDescriptor == -1 )
        {
            continue;
        }
        int optionValue = 1;
        setsockopt(socketFileDescriptor , SOL_SOCKET , SO_REUSEADDR , &optionValue , sizeof(optionValue));
        serverBindResult = bind(socketFileDescriptor , possibleConnection->ai_addr , possibleConnection->ai_addrlen);
        if ( serverBindResult == -1 )
        {
            cout << "Unable to bind to the socket.";
            close(socketFileDescriptor);
            continue;
        }
        break; // at this point, have successfully bind to a socket
    }
    if ( possibleConnection == NULL )
    {
        cout << "Unable to connect.";
        return -1;
    }
    freeaddrinfo(serverInfo);

    int listenUsingSocketResult = listen(socketFileDescriptor , CONNECTION_REQUEST_SIZE);
    if ( listenUsingSocketResult != 0 )
    {
        cout << "Socket file desc error.";
        return -1;
    }
    int count = 1;
    //Keep looking for connections
    while ( true ){
        struct sockaddr_storage clientSocket;
        socklen_t clientSocketSize = sizeof(clientSocket);
        int clientFileDescriptor = accept(socketFileDescriptor , (struct sockaddr *) &clientSocket , &clientSocketSize);
        if ( clientFileDescriptor == -1 )
        {
            cout << "Unable to connect to client." << endl;
            continue;
        }
        pthread_t new_thread;
        struct thread data;
        data.id = count;
        data.fileDesc = clientFileDescriptor;
        int threadResult = pthread_create(&new_thread , nullptr , serverResponsePlayer , (void *) &data);
        if ( threadResult != 0 )
        {
            continue;
        }
        count++;
    }
}
