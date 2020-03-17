#ifndef CLIENT
#define CLIENT

#include <iostream>
#include <vector>
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h>
#include <string>

using namespace std;

class Client
{
public:
    Client();
    int score;
    void placeBets();
    void roundPlay();

    int dealerScore;
    int round;
    vector <string> cards;
    vector <string> dealerCards;
    string pickCard(); //pick a random card and remove it from cards 
    void processDealer();
    int countCards(vector <string> temp);
    void startRound();
    void singlePlayerGameLoop();
    void multiPlayerGameLoop(char *const *args);

private:
    int wallet;
    int bet;
    vector<string> handCards;



    void printCards(vector<string> handCards)
    {
        for (int i = 0; i < handCards.size(); i++)
        {
            cout << handCards.at(i) << " ";
        }
    }
    void initialCard()
    {
        cards.clear();
        for (int i = 2; i < 11; i++)
        {
            for (int j = 0; j < 4; ++j)
            {
                cards.push_back(to_string(i));
            }
        }
        for (int j = 0; j < 4; ++j)
        {
            cards.push_back("J");
        }
        for (int j = 0; j < 4; ++j)
        {
            cards.push_back("Q");
        }
        for (int j = 0; j < 4; ++j)
        {
            cards.push_back("K");
        }
        for (int j = 0; j < 4; ++j)
        {
            cards.push_back("A");
        }

        int size = cards.size();
        for (int i = 0; i < size - 1; i++)
        {
            int j = i + rand() % (size - i);
            swap(cards[i], cards[j]);
        }
    }

};
#endif

