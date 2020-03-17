#include "Server.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>
//rule: no insurance, no double, no split, no surrender

const string THREAD_MESSAGE = "Creating new thread with count: ";
const int CONNECTION_REQUEST_SIZE = 10;
const string OK_RESPONSE = "HTTP/1.1 200 OK\r\n"; //this represents a player hit
const string DOES_NOT_EXIST_RESPONSE = "HTTP/1.1 404 Not Found\r\n"; //this represents a player stay
const string UNAUTHORIZED_RESPONSE = "HTTP/1.1 401 Unauthorized\r\n";
const string FORBIDDEN_RESPONSE = "HTTP/1.1 403 Forbidden\r\n";
const string BAD_REQUEST_RESPONSE = "HTTP/1.1 400 Bad Request\r\n";

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
string parseHeaderInfo(int socketFileDescriptor)
{
    string responseHeader = "";
    char lastChar = 0;
    while ( true )
    {
        char currentChar = 0;
        recv(socketFileDescriptor , &currentChar , 1 , 0);
        if ( currentChar == '\n' || currentChar == '\r' )
        {
            if ( lastChar == '\r' && currentChar == '\n' ) // For each header, it is ended with a \r\n
                break;
        }
        else responseHeader += currentChar;
        lastChar = currentChar;
    }
    return responseHeader;
}

void prepareResponseData( bool isGET , string &statusCode)
{
    if ( isGET )
    {
        if(statusCode == OK_RESPONSE){
            cout << "Player has hit, draw next card." << endl;
        }else if(statusCode == DOES_NOT_EXIST_RESPONSE){
            cout << "Player has decided to stay. Next turn." << endl;
        }
    }
    else
    {
        // Could not recognize the get request
        statusCode = "A player has joined the game.";
    }

}

void *processGETRequest(void *threadData)
{
    struct thread *data;
    data = (struct thread *) threadData;
    bool isGET = false;
   //work in progress waiting for client to work
    string statusCode;
    prepareResponseData( isGET , statusCode);
    string response = statusCode;
    cout << "Server: "+ response << endl;
    send(data->fileDesc , &response[ 0 ] , response.size() , 0);
    close(data->fileDesc);
    return 0;
}

int main(int argumentNumber , char *argumentValues[])
{
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
        cout << THREAD_MESSAGE + to_string(count) << endl;
        // Spawn a thread to do the work
        int threadResult = pthread_create(&new_thread , nullptr , processGETRequest , (void *) &data);
        if ( threadResult != 0 )
        {
            cout << "Thread error, please retry." << endl;
            continue;
        }
        count++;
    }
}
