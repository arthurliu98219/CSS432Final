#include "Client.h"

#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <fstream>

using namespace std;

char *serverName;
Client c;
bool joinedGame;
bool currentTurn;

//this is where the client will play singleplayer by themselves if they wish to do so
void Client::singlePlayerGameLoop(){
    cout << "Starting single player mode." << endl;
    string playerMove;
    cout << "Please enter your name: " << endl;
    string username;
    cin >> username;
    cout << "Welcome "+username+" to Blackjack." << endl << endl;

    bool gameEnd = false;
    while(true){
        bool gameEnd = false;
        vector<int> cards;
        vector<int> playerOneCards;
        vector<int> playerTwoCards;
        vector<int> dealerCards;
        string possibleCards[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"}; //All possible cards
            cout << "Starting Game.." << endl;
            //start by dealing the first card on the table
            srand(time(0));
            int dealerCard1 = rand() % 13;
            int dealerCard2 = rand() %
                              13; //this card is hidden from the players and will only be revealed once players exceed combined amount.
            int card1 = rand() % 13;
            int card2 = rand() % 13;
            if (dealerCard1 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
                dealerCard1 = 10;
            }
            if (card1 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
                card1 = 10;
            }
            if (card2 > 10) { //because there are 10, J , Q, and K which are all worth 10 points
                card2 = 10;
            }
            cards.push_back(dealerCard1);
            string message = "Dealers first card: " + possibleCards[dealerCard1] + ".";
            //add to dealer card stack
            dealerCards.push_back(dealerCard1);

            //send message of 1st dealer card to players
            cout << message << endl;

            //send second message to each player with their own hand
            string message1 = "You have drawn a " + possibleCards[card1] + " and a " + possibleCards[card2] + ".";
            cout << message1 << endl;
            playerOneCards.push_back(card1);
            playerOneCards.push_back(card2);


            int dealerTotal = 0;
            while(true) {//loop for current game

                //waiting to see if player will HIT or STAND--------------------------------------------------------------
                int cardTemp;
                string playerOneMove;
                cout << "Enter '1' to HIT, enter '2' to STAND" << endl;
                while(true) {
                    cin >> playerOneMove;
                    if (playerOneMove == "1" || playerOneMove == "2") {
                        break;
                    }
                }
                if (playerOneMove == "1") { //1 = HIT, 2 = STAND
                    cout << username + " has decided to hit." << endl;
                    cardTemp = rand() % 13;
                    playerOneCards.push_back(cardTemp);
                    //send player message on which card they had drawn.
                    string temp = "You have drawn a " + possibleCards[cardTemp] + ".";
                    cout << temp << endl;
                } else {
                    cout << username + " has decided to stand." << endl;
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
                cout << message << endl;//this is message to server console to keep track of the game

                //game check to see if someone has lost------------------------------------------------------------

                //player1 check
                int playerOneTotal = 0;
                for (int i = 0; i < playerOneCards.size(); i++) {
                    playerOneTotal += playerOneCards[i] + 1; //add one for index disparity
                    //cout << "1adding " <<playerOneCards[i] << " to " +playerOneName + " total count." << endl;
                }

                //if someone has lost then send endgame message to player-------------------------------------

                string dealerInfo;
                if (playerOneTotal > 21 || dealerTotal == 21) { //losing from total amount
                    message = "dealer total is "+to_string(dealerTotal)+".";//you = lose
                    cout << "Game over, you have lost." << endl;
                    gameEnd = true;
                    cout << message << endl;
                } else if (((playerOneTotal > dealerTotal) && (dealerTotal > 17)) || (dealerTotal > 21)) {
                    message = "dealer total is "+to_string(dealerTotal)+".";//you win
                    cout << "Congratulations on beating the dealer!" << endl;
                    gameEnd = true;
                    cout << message << endl;
                }

                if(gameEnd) {
                    break;
                }
            }
            gameEnd = false;
            cout << "Do you want to keep playing? Enter '1' to keep playing, enter '2' to quit." << endl;
            while(true){
                string option;
                cin >> option;
                if(option == "1"){
                    break;
                }else if(option == "2"){
                    gameEnd = true;
                    exit(0);
                }
            }

        if(gameEnd){
            break;
        }
    }
}

void Client::placeBets()
{
    cout << "Inter your bet amount." << endl;
    cin >> bet;
    wallet -= bet;
}

void Client::roundPlay()
{
    //to add: if round starts, disable new player to join

    //get two cards
    handCards.push_back(pickCard());
    handCards.push_back(pickCard());
    //print cards on hand
    printCards(handCards);
    cout << "Card score: " << countCards(handCards) << endl;
    cout << "Dealer: " << dealerCards.at(0) << endl;
    while (true)
    {
        cout << "Enter your option:" << endl;
        cout << "1 for Hit" << endl;
        cout << "2 for Stand" << endl;
        int option;
        cin >> option;

        if (option == 1)
        {
            handCards.push_back(pickCard());
            printCards(handCards);
            score = countCards(handCards);
            cout << score << endl;
            if (score > 21)
            {
                cout << "you lose" << endl;
                break;
            }
        }
        else if (option == 2)
        {
            break;
        }
        else
        {
            cout << "enter 1 or 2" << endl;
        }
    }

}

string Client::pickCard()
{
    srand(time(0));
    int index = rand() % cards.size();// pick a random index
    string value = cards[index];
    cards.erase(cards.begin() + index);//remove
    return value;
}

void Client::processDealer()
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
    for (int i = 0; i < dealerCards.size(); i++)
    {
        cout << dealerCards.at(i) << " ";
    }
    cout << "dealer score:" << countCards(dealerCards) << endl;
    dealerScore = countCards(dealerCards);
}

Client ::Client(){

}

int Client::countCards(vector <string> temp)
{
    int count = 0;
    int countA = 0;
    for (int i = 0; i < temp.size(); i++)
    {
        if (temp.at(i) == "J" ||
            temp.at(i) == "Q" ||
            temp.at(i) == "K")
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
        for (int i = 0; i < countA; i++)
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

void Client::startRound()
{
    initialCard();
    round += 1;
    cout << "Round " << round << endl;
    processDealer();
}

string parseHeaderInfo(int socketFileDescriptor)
{
    string responseHeader = "";
    char currentChar = 0;
    recv(socketFileDescriptor , &currentChar , 1 , 0);
    return responseHeader;
}

string getSocketMessage(int socketFileDescriptor){
    string responseHeader = "";
    char lastChar = 0;
    while ( true )
    {
        char currentChar = 0;
        recv(socketFileDescriptor , &currentChar , 1 , 0);
        //cout << "client currently reading: " << currentChar << endl;//used for debugging
        if (currentChar == '.' ){
            break;
        }
        else responseHeader += currentChar;
        lastChar = currentChar;
    }
    return responseHeader;
}

int sendServerMove(int socketFileDescriptor)
{

    string playerMove;
    if(joinedGame == false){
        cout << "Please enter your name: " << endl;
        string username;
        cin >> username;
        playerMove = username +" has joined the game.";
    }else{

        cout << "Sending move to Server: " << endl;
    }

    cout << playerMove << endl;
    playerMove = "1" + playerMove;
    int sendResult = send(socketFileDescriptor , playerMove.c_str() , strlen(playerMove.c_str()) , 0);
    if ( sendResult <= 0 )
    {
        cout << "Unable to send the player move to server.";
        return -1;
    }


    //THIS IS CLIENT RECIEVING MESSAGE FROM SERVER-----------------------------------------------------------------
    while(true) {
        cout << "getting table cards" << endl;
        string serverResponse = getSocketMessage(socketFileDescriptor);//get table cards
        cout << "Server: " + serverResponse << endl;

        cout << "getting player cards" << endl;
        serverResponse = getSocketMessage(socketFileDescriptor);//get player hand cards
        cout << "Current Player: " + serverResponse << endl;

        while (true) {//game loop for current game
            //Server will ask client to hit or stand based on their current cards
            cout << "Enter '1' to HIT or enter '2' to STAND" << endl;
            string option = "";
            while (true) {
                cin >> option;

                if (option == "1") {// if HIT pick up card from server
                    cout << "You have chosen to HIT" << endl;
                    option += ".";
                    send(socketFileDescriptor, option.c_str(), strlen(option.c_str()), 0);//send option to hit to server
                    serverResponse = getSocketMessage(socketFileDescriptor);//getting your card when hit
                    cout << serverResponse << endl;
                    break;
                } else if (option == "2") {
                    cout << "You have chosen to STAND" << endl;
                    option += ".";//still have to send message even if the player decides to stand
                    send(socketFileDescriptor, option.c_str(), strlen(option.c_str()), 0);//send option to hit to server
                    break;
                } else {
                    cout << "Please enter a valid option and try again." << endl;
                }
            }

            serverResponse = getSocketMessage(socketFileDescriptor);
            cout << serverResponse << endl;

            //THIS IS CLIENT CHECKING TO SEE IF PLAYER HAS LOST THE GAME
            serverResponse = getSocketMessage(socketFileDescriptor);
            if (serverResponse[0] ==
                '1') {//server will check to make sure it will be 1 or 2 so dont need to check for other responses
                cout << serverResponse.substr(1, serverResponse.length()) << endl;
                cout << "Game over, you have lost." << endl;
                break;
            } else if (serverResponse[0] == '2') {
                cout << serverResponse.substr(1, serverResponse.length()) << endl;
                cout << "Congratulations on beating the dealer!" << endl;
                break;
            }
        }
        cout << "The game will now exit" << endl;
        exit(0);


        /* //unable to implement exit game loop for some reason
        cout << "Do you want to play again? Enter '1' to play again, enter '2' to quit" << endl;
        string option;
            cin >> option;
            if (option == "1") {
                cout << "You have chosen to play again." << endl;
            } else {
                cout << "You have chosen to quit." << endl;
                exit(0);
            }
        send(socketFileDescriptor, option.c_str(), strlen(option.c_str()), 0);

        string message = getSocketMessage(socketFileDescriptor);

        if (message[0] == '1') {
            cout << message.substr(1,message.length()) << endl;
        }else{
            cout << message.substr(1,message.length()) << endl;
            break;
        }
*/

    }
    return 0;

}

int setUpSocket(char *const *argumentValues)
{
    struct addrinfo hints; //define what the getaddrinfo going to do. Define IPV4 or v6, what kind of connection,..etc
    struct addrinfo *serverInfo; // getaddrinfo will put the results in here. And we can go though this to get the address
    memset(&hints , 0 , sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    serverName = argumentValues[ 1 ];
    int addrInfoStatus = getaddrinfo(serverName , argumentValues[ 2 ] , &hints , &serverInfo);
    if ( addrInfoStatus != 0 )
    {
        cout << "Unable to connect";
        cout << gai_strerror(addrInfoStatus); // print out error message
        return -1;
    }
    struct addrinfo *possibleConnection;
    int socketFileDescriptor;
    int socketConnectionResult;
    // Go through all connection that was found and connect to the first one
    for ( possibleConnection = serverInfo;
          possibleConnection != NULL; possibleConnection = possibleConnection->ai_next )
    {
        socketFileDescriptor = socket(possibleConnection->ai_family , possibleConnection->ai_socktype ,
                                      possibleConnection->ai_protocol);
        if ( socketFileDescriptor == -1 )
        {
            cout << "Invalid one socket file descriptor detected. Looking for next one";
            continue;
        }
        socketConnectionResult = connect(socketFileDescriptor , possibleConnection->ai_addr ,
                                         possibleConnection->ai_addrlen);
        if ( socketConnectionResult == -1 )
        {
            cout << "Invalid one socket connection result detected. Looking for next one";
            continue;
        }
        cout << "Player has connected to Server" << endl;
        break;
    }
    // If still null, then it means that we went through all possible connections but none satisfied
    if ( possibleConnection == NULL )
    {
        cout << "Unable to connect or empty result was given";
        return -1;
    }
    freeaddrinfo(serverInfo);
    return socketFileDescriptor;
}

void Client::multiPlayerGameLoop(char *const *args){
    cout << "Starting multiplayer mode." << endl;

    /* //feature to maybe be implemented if there is time, just makes user interface nicer/easier to understand
    string serverName;
    cout << "Please enter server name/IP" << endl;
    cin >> serverName;
    string portNumber;
    cout << "Please enter port number" << endl;
    cin >> portNumber;
     */
    int sockFileDesc = setUpSocket(args);//connect to socket and get file descriptor for socket
    if ( sockFileDesc == -1 ){
        cout << "Unable to create a socket";
    }
    //test
    sendServerMove(sockFileDesc);

}

int main(int argumentNumber , char *args[]) {
    //infinite loop until user enters in either 1 for singleplayer or 2 for multiplayer
    while (true) {
        cout << "Welcome to Blackjack, enter '1' to play singleplayer, '2' to play multiplayer" << endl;
        string answer;
        cin >> answer;
        if (answer == "1") {
            cout << "You have chosen singleplayer mode." << endl;
            c.singlePlayerGameLoop();
            break;
        } else if (answer == "2") {
            joinedGame = false;
            cout << "You have chosen multiplayer mode." << endl;
            c.multiPlayerGameLoop(args);
            break;
        } else {
            cout << "Invalid option, choose again" << endl;
        }
    }

}