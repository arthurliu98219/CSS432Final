#include "Server.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
#include<vector>
#include <stdlib.h>


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

void prepareResponseData( bool isGET , string &statusCode)
{
    /*
    cout << "Server calculating round..." << endl;
    if ( isGET )
    {
        if(statusCode == OK_RESPONSE){
            cout << "Player has hit, draw next card." << endl;
        }else if(statusCode == DOES_NOT_EXIST_RESPONSE){
            cout << "Player has decided to stay. Next turn." << endl;
        }
    }
    else{
        // Could not recognize the get request
        statusCode = "Unknown Move from Player.";
    }
     */

}

void *serverResponsePlayer(void *threadData)
{
    struct thread *data;
    data = (struct thread *) threadData;
    bool isGET = false;

    //parses response from client
    string responseHeader = "";
    char lastChar = 0;
    bool lookForName = false;
    string currentName = "";
    while (true)
    {
        char currentChar = 0;
        recv(data->fileDesc , &currentChar , 1 , 0);
        if(lookForName && currentChar != '1'){
            if(currentChar == ' '){
                lookForName = false;
            }else{
                currentName+=currentChar;
            }

        }
        if(currentChar =='1'){
            lookForName = true;
        }
        if ( currentChar == '.' )
        {
                break;
        }
        else responseHeader += currentChar;
        lastChar = currentChar;
    }
    cout << "This is what the server recieved: " + responseHeader << endl;
    string statusCode;
    //server will reply to player

    //this is for when the 2 players initially join
    cout << "current player name is: " +currentName << endl;
    if(playerOneJoined == false){
        playerOneName = currentName;
        playerOneSockFileDesc = data->fileDesc;
        playerOneJoined = true;
    }else if(playerTwoJoined == false){
        playerTwoName = currentName;
        playerTwoSockFileDesc = data->fileDesc;
        playerTwoJoined = true;
    }
    cout << "Current Players in game(limited to two):" << endl;
    if(playerOneJoined){
        cout << "1. "+playerOneName << endl;
    }
    if(playerTwoJoined){
        cout << "2. "+playerTwoName << endl;
    }

    if(responseHeader[0] == '1'){ //if response starts with a one then a player has joined the game
        cout << responseHeader.substr(1,responseHeader.length()) << endl;
    }

    //this is the game loop where the game truly starts between the two players
    bool gameEnd = false;
    vector<int> cards;
    vector<int> playerOneCards;
    vector<int> playerTwoCards;
    while(playerOneJoined && playerTwoJoined){
        cout << "Starting Game.." << endl;
        //start by dealing the first card on the table
        int randomNum = rand() % 15;
        int card1 = rand() % 15;
        int card2 = rand() % 15;
        int card3 = rand() % 15;
        int card4 = rand() % 15;
        if(randomNum > 10){ //because there are 10, J , Q, and K which are all worth 10 points
            randomNum = 10;
        }
        if(card1 > 10){ //because there are 10, J , Q, and K which are all worth 10 points
            card1 = 10;
        }
        if(card2 > 10){ //because there are 10, J , Q, and K which are all worth 10 points
            card2 = 10;
        }
        if(card3 > 10){ //because there are 10, J , Q, and K which are all worth 10 points
            card3 = 10;
        }
        if(card4 > 10){ //because there are 10, J , Q, and K which are all worth 10 points
            card4 = 10;
        }
        cards.push_back(randomNum);
        string message = "Current cards on table: " + to_string(randomNum) + ".";
        send(playerOneSockFileDesc,&message[0],message.size(),0); // send message to player1
        send(playerTwoSockFileDesc,&message[0],message.size(),0); // send message to player2

        cout << "Dealer is handing out individual cards" << endl;
        //send second message to each player with their own hand
        string message1 = "You have drawn a " + to_string(card1) + " and a " + to_string(card2)+".";
        send(playerOneSockFileDesc,&message1[0],message1.size(),0); // send message to player1
        playerOneCards.push_back(card1);
        playerOneCards.push_back(card2);

        string message2 = "You have drawn a " + to_string(card3) + " and a " + to_string(card4)+".";
        send(playerTwoSockFileDesc,&message2[0],message2.size(),0); // send message to player2
        playerOneCards.push_back(card3);
        playerOneCards.push_back(card4);


        gameEnd = true;




        if(gameEnd){
            break;
        }
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
    struct addrinfo hints; // define how the server will be configure
    struct addrinfo *serverInfo; // used to store all the connections that the server can use
    memset(&hints , 0 , sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 or v6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int addressInfoStatus = getaddrinfo(nullptr , argumentValues[ 1 ] , &hints , &serverInfo);
    if ( addressInfoStatus != 0 )
    {
        cout << "Unable to connect";
        cout << gai_strerror(addressInfoStatus); // print out error message
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
            cout << "Invalid one socket file descriptor detected. Looking for next one";
            continue;
        }
        int optionValue = 1;
        setsockopt(socketFileDescriptor , SOL_SOCKET , SO_REUSEADDR , &optionValue , sizeof(optionValue));
        serverBindResult = bind(socketFileDescriptor , possibleConnection->ai_addr , possibleConnection->ai_addrlen);
        if ( serverBindResult == -1 )
        {
            cout << "Unable to bind to the socket using this file descriptior";
            close(socketFileDescriptor);
            continue;
        }
        break; // at this point, have successfully bind to a socket
    }
    if ( possibleConnection == NULL )
    {
        cout << "Unable to connect or empty result was given";
        return -1;
    }
    freeaddrinfo(serverInfo);

    int listenUsingSocketResult = listen(socketFileDescriptor , CONNECTION_REQUEST_SIZE);
    if ( listenUsingSocketResult != 0 )
    {
        cout << "Unable to listen using the socket file descriptor";
        return -1;
    }
    int count = 1;
    // Keep looping and listening to possible connection
    while ( true ){
        struct sockaddr_storage clientSocket;
        socklen_t clientSocketSize = sizeof(clientSocket);
        int clientFileDescriptor = accept(socketFileDescriptor , (struct sockaddr *) &clientSocket , &clientSocketSize);
        if ( clientFileDescriptor == -1 )
        {
            cout << "Unable to connect to client. Trying again" << endl;
            continue;
        }
        pthread_t new_thread;
        struct thread data;
        data.id = count;
        data.fileDesc = clientFileDescriptor;
        // Spawn a thread to do the work
        int threadResult = pthread_create(&new_thread , nullptr , serverResponsePlayer , (void *) &data);
        if ( threadResult != 0 )
        {
            cout << "Thread error, please retry." << endl;
            continue;
        }
        count++;
    }
}
