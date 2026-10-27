#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>

//卡牌
typedef struct{
    int suit;//花色
    int rank;//点数
}Card;

typedef struct{
    Card five[5];
    int src[5];
    int score; 
}BestHand;

typedef struct{
    Card hole[2]; //两张手牌
    bool folded; //是否弃牌
    int chips; //筹码量
    int bet; //本轮下筹码量
    bool act; //本轮行动变量

    bool allin; //是否ALLIN
    int put; //本局下筹码总量
}Player;


void initDeck(Card deck[]);
void shufflecard(Card deck[]);
void initPlayers(Player seats[], int n, int startChips);
void resetHand(Player seats[], int n);
int countWithChips(Player seats[], int n);


void payToPot(Player &p, int &pot, int amt);
void callBet(Player &p, int &pot, int currentBet);
void raiseBet(Player &p, int &pot, int &currentBet, int newBet, int &miniRaise);
void afterRaise(Player seats[], int n,int who);
void payBlind(Player &p, int &pot, int amt);
int sbIndex(Player seats[], int BTN, int n);
int bbIndex(Player seats[], int BTN, int n);
bool canCheck(Player p, int currentBet);
void foldPlayer(Player &p);
void doAction(Player seats[], int &pot, int &currentBet, int newBet, int act, int &miniRaise, int n, int who);

void bettingRound(Player seats[], int &pot, int start, int n, int round, Card board[]);


void resetBets(Player seats[],int n);

int nextActive(Player seats[], int n, int from);
int firstAvtivePlayer(Player seats[], int n, int from);
bool allCaughtUp(Player seats[], int n, int currentBet);
bool allHaveActive(Player seats[], int n);
bool bettingOver(Player seats[], int n,int currentBet);

int countActivePlayer(Player seats[],int n);
int lastActive(Player seats[], int n);
int showDownWinners(BestHand p[], Player seats[],int n, int win[]);
Card dealcard(Card deck[],int &top);
void dealHole(Card deck[], int &top,Player player[], int n);
void printcard(Card c);
std::string cardName(Card c);

void dealflop(Card deck[],int &top,Card board[]);
void dealturn(Card deck[], int &top, Card board[]);
void dealriver(Card deck[], int &top, Card board[]);

bool hasPair(Card c[]);
bool hasTwoPairs(Card c[]);
bool hasThreeOfAKind(Card c[]);
bool hasStraight(Card c[]);
bool hasFlush(Card c[]);
bool hasFullHouse(Card c[]);
bool hasFourOfAKind(Card c[]);
bool hasFlushStraight(Card c[]);

int handRank(Card c[]);
int pack(int cat, int a, int b, int c, int d, int e);
int handscore(Card c[]);
void bestScore(Card c[], Card board[], BestHand &p);

void printHandRank(BestHand p);
std::string HandrankName(BestHand p);
void printAction(int n);
void printActionInfo(Player p, int pot, int currentBet, int miniRaise, int round, Card board[]);

int readPlayerNum();
int readTotalHands();
int readStartChips();
int readAction(Player p, int currentBet, int &newBet, int miniRaise);

int minPositive(int remain[], int n);
int peelLayer(int remain[], int n);
int layerElig(Player seats[], int remain[], int n, int elig[]);
int showDownLayer(BestHand p[], int elig[], int ecnt, int win[]);

