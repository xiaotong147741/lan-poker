#include "card.h"

//初始化牌堆
void initDeck(Card deck[]){
    int count=0;
    for(int s=0;s<4;s++){
        for(int r=2;r<=14;r++){
            deck[count].suit = s;
            deck[count].rank = r;
            count++; 
        }
    }
}


//洗牌
void shufflecard(Card deck[]){
    for(int i=51;i>0;i--){
        int j = rand()%(i+1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

//初始化玩家状态
void initPlayers(Player seats[], int n, int startChips){
    for(int i=0;i<n;i++){
        seats[i].folded = false;
        seats[i].chips = startChips;
        seats[i].bet = 0;
        seats[i].act = false;
        seats[i].put = 0;
        seats[i].allin = false;
    }
}

//同局重置
void resetHand(Player seats[], int n){
    for(int i=0;i<n;i++){
        seats[i].folded = (seats[i].chips <= 0);
        seats[i].bet = 0;
        seats[i].act = false;
        seats[i].put = 0;
        seats[i].allin = false;
    }
}

//统计存活玩家个数
int countWithChips(Player seats[], int n){
    int cnt = 0;
    for(int i = 0; i < n; i++){
        if(seats[i].chips > 0)
            cnt++;
    }
    return cnt;
}

//下注
void payToPot(Player &p, int &pot, int amt){
    p.chips -= amt;
    pot += amt;
    p.put += amt;
    if(p.chips == 0)
        p.allin = true;
}

//跟注
void callBet(Player &p, int &pot, int currentBet){
    int amt = currentBet - p.bet;
    if(amt > p.chips)
        amt = p.chips;
    payToPot(p, pot, amt);
    p.bet += amt;
    p.act = true;
}

//加注
void raiseBet(Player &p, int &pot, int &currentBet, int newBet, int &miniRaise){
    int amt = newBet - p.bet;
    if(amt > p.chips)
        amt = p.chips;
    newBet = p.bet + amt;
    payToPot(p, pot, amt);
    p.bet = newBet;
    if(newBet - currentBet >= miniRaise)
        miniRaise = newBet - currentBet;
    currentBet = newBet;
}

//加注过后
void afterRaise(Player seats[], int n,int who){
    for(int i=0;i<n;i++)
        seats[i].act = false;
    seats[who].act = true;
}

//判断能否check
bool canCheck(Player p, int currentBet){
    return p.bet == currentBet;
}

//弃牌
void foldPlayer(Player &p){
    p.folded = true;
    p.act = true;
}

//行动
void doAction(Player seats[], int &pot, int &currentBet, int newBet, int act, int &miniRaise, int n, int who){
    if(act == 1)
        foldPlayer(seats[who]);
    if(act == 2)
        seats[who].act = true;
    if(act == 3)
        callBet(seats[who], pot ,currentBet);
    if(act == 4){
        raiseBet(seats[who], pot, currentBet, newBet, miniRaise);
        afterRaise(seats, n, who);
    }
}

//跑圈
void bettingRound(Player seats[], int &pot, int start, int n, int round, Card board[]){
    int who = start;
    if(who == -1)
        return;
    int newBet = 0;
    int currentBet = 0;
    if(round == 0)
        currentBet = 10;
    int miniRaise = 10;//等于大盲注
    while(!bettingOver(seats, n, currentBet)){
        std::cout<<"\n";
        std::cout<<"player"<<who<<" action:"<<std::endl;
        int nBoard = 0;
        if(round > 0)
            nBoard = round + 2;

        printActionInfo(seats[who], pot, currentBet, miniRaise, nBoard, board);
        int act = readAction(seats[who], currentBet, newBet, miniRaise);
        printAction(act);
        doAction(seats, pot, currentBet, newBet, act, miniRaise, n, who);
        who = nextActive(seats, n, who);
        if(who < 0)
            return;
        if(countActivePlayer(seats, n) == 1)
            break;
    }
}


//盲注
void payBlind(Player &p, int &pot, int amt){
    if(amt > p.chips)
        amt = p.chips;
    payToPot(p, pot, amt);
    p.bet = amt;
}

//小盲
int sbIndex(Player seats[], int BTN, int n){
    if(countActivePlayer(seats, n) == 2)
        return BTN;
    return nextActive(seats, n, BTN);
}

//大盲
int bbIndex(Player seats[], int BTN, int n){
    int sb = sbIndex(seats, BTN, n);
    return nextActive(seats, n, sb);
}

//下一位
int nextActive(Player seats[], int n, int from){
    for(int i=1;i<=n;i++){
        int tmp = (from + i) % n;
        if(!seats[tmp].folded && !seats[tmp].allin)
            return tmp;
    }
    return -1;
}

//除翻牌前第一个行动的玩家
int firstAvtivePlayer(Player seats[], int n,int from){
    return nextActive(seats, n, from);
}

//判断是否所有人都下注额是否一致
bool allCaughtUp(Player seats[], int n, int currentBet){
    for(int i=0;i<n;i++){
        if(seats[i].bet != currentBet && !seats[i].folded && !seats[i].allin)
            return false;
    }
    return true;
}

//判断是否所有人行动结束
bool allHaveActive(Player seats[], int n){
    for(int i=0;i<n;i++){
        if(!seats[i].act && !seats[i].folded && !seats[i].allin)
            return false;
    }
    return true;
}

//当前轮次是否结束
bool bettingOver(Player seats[], int n,int currentBet){
    if(countActivePlayer(seats, n)<=1)
        return true;
    return (allCaughtUp(seats, n, currentBet) && allHaveActive(seats, n));
}


//重置当前轮次的各玩家加注额
void resetBets(Player seats[],int n){
    for(int i=0;i<n;i++){
        seats[i].bet = 0;
        seats[i].act = false;
    }
}

//没有弃牌的玩家数量
int countActivePlayer(Player seats[],int n){
    int cnt = 0;
    for(int i=0;i<n;i++){
        if(seats[i].folded==false)
            cnt++;
    }
    return cnt;
}

//判断由弃牌导致的winner是谁
int lastActive(Player seats[], int n){
    if(countActivePlayer(seats, n)==1){
        for(int i=0;i<n;i++){
            if(seats[i].folded==false)
                return i;
        }
    }
    return -1;
}

//记录摊牌winner以及数量
int showDownWinners(BestHand p[], Player seats[],int n, int win[]){
    int cnt = 0;
    int max = -1;
    for(int i=0;i<n;i++){
        if(max<p[i].score && seats[i].folded==false)
            max = p[i].score;
    }
    for(int i=0;i<n;i++){
        if(max==p[i].score && seats[i].folded==false)
            win[cnt++] = i;
    }
    return cnt;
}


//发一张牌
Card dealcard(Card deck[],int &top){
    Card c = deck[top];
    top++;
    return c;
}

//给所有玩家发送起始手牌
void dealHole(Card deck[], int &top,Player player[], int n){
    for(int i=0;i<2;i++){
        for(int j=0;j<n;j++){
            player[j].hole[i] = dealcard(deck,top);
        }
    }
}

//打印
void printcard(Card c){
    const char *suits[4] = {"♥", "♦", "♠", "♣"};
    const char *ranks[] = {" "," ","2","3","4","5","6","7","8","9","T","J","Q","K","A"};
    std::cout<<suits[c.suit]<<" "<<ranks[c.rank]<<std::endl;
}

std::string cardName(Card c){
    const char *suits[4] = {"♥", "♦", "♠", "♣"};
    const char *ranks[] = {" "," ","2","3","4","5","6","7","8","9","T","J","Q","K","A"};
    return std::string(suits[c.suit]) + " " + ranks[c.rank];
}

//翻牌
void dealflop(Card deck[], int &top, Card board[]){
    dealcard(deck, top);//切一张
    for(int i=0;i<3;i++){
        Card c = dealcard(deck, top);
        board[i] = c;
        //printcard(c);
    }
}

//转牌
void dealturn(Card deck[], int &top, Card board[]){
    dealcard(deck, top);//切一张
    Card c = dealcard(deck, top);
    board[3] = c;
    //printcard(c);
}

//河牌
void dealriver(Card deck[], int &top, Card board[]){
    dealcard(deck, top);//切一张
    Card c = dealcard(deck, top);
    board[4] = c;
    //printcard(c);
}

//判断对子
bool hasPair(Card c[]){
    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;
    for(int i=2;i<15;i++){
        if(count[i]>=2)
            return true;
    }
    return false;
}

//判断两对
bool hasTwoPairs(Card c[]){
    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;

    int pairs = 0;
    for(int i=2;i<15;i++){
        if(count[i]>=2)
            pairs++;
    }
    if(pairs>=2)
        return true;
    else
        return false;
}

//判断三条
bool hasThreeOfAKind(Card c[]){
    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;
    for(int i=2;i<15;i++){
        if(count[i]>=3)
            return true;
    }
    return false;
}

//顺子
bool hasStraight(Card c[]){
    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;

    if(count[14] && count[2] && count[3] && count[4] && count[5])
        return true;

    for(int i=2;i<11;i++){
        if(count[i] && count[i+1] && count[i+2] && count[i+3] && count[i+4])
            return true;
    }
    return false;
}

//同花
bool hasFlush(Card c[]){
    int count[4]={0};
    for(int i=0;i<5;i++)
        count[c[i].suit]++;
    for(int i=0;i<4;i++){
        if(count[i]>=5)
            return true;
    }
    return false;
}


//葫芦
bool hasFullHouse(Card c[]){
    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;

    bool three = false;
    bool two = false;
    for(int i=2;i<15;i++){
        if(count[i]>=3)
            three = true;
        else if(count[i]>=2)
            two = true;
    }
    if(three && two)
        return true;
    return false;
}

//四条
bool hasFourOfAKind(Card c[]){
    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;

    for(int i=2;i<15;i++){
        if(count[i]>=4)
            return true;
    }
    return false;
}

//同花顺
bool hasFlushStraight(Card c[]){
    if(hasFlush(c) && hasStraight(c))
        return true;
    return false;
}

//判断牌型
int handRank(Card c[]){
    if(hasFlushStraight(c))
        return 8;
    if(hasFourOfAKind(c))
        return 7;
    if(hasFullHouse(c))
        return 6;
    if(hasFlush(c))
        return 5;
    if(hasStraight(c))
        return 4;
    if(hasThreeOfAKind(c))
        return 3;
    if(hasTwoPairs(c))
        return 2;
    if(hasPair(c))
        return 1;
    return 0;
}

//牌面分
int pack(int cat, int a, int b, int c, int d, int e) {
    return ((((cat * 15 + a) * 15 + b) * 15 + c) * 15 + d) * 15 + e;
}

//计算牌面分
int handscore(Card c[]){
    int cat = handRank(c);
    int v[5]={0};

    int count[15]={0};
    for(int i=0;i<5;i++)
        count[c[i].rank]++;

    int n=0;

    for(int f=4;f>0;f--){
        for(int i=14;i>=2;i--){
            if(count[i]==f){
                v[n++] = i;
            }
        }
    }
    if ((cat == 4 || cat == 8) && v[0] == 14 && v[1] == 5)
        return pack(cat, 5, 0, 0, 0, 0);
    return pack(cat, v[0], v[1], v[2], v[3], v[4]);
}

//最佳组合手牌得分
void bestScore(Card c[], Card board[], BestHand &p){
    Card seven[7];
    for(int i=0;i<5;i++)
        seven[i] = board[i];
    for(int i=0;i<2;i++)
        seven[i+5] = c[i];

    p.score = 0;

    for(int i=0;i<7;i++){
        for(int j=i+1;j<7;j++){
            Card five[5];
            int n = 0;
            int src[5];
            for(int k=0;k<7;k++){
                if(k!=i && k!=j){
                    five[n] = seven[k];
                    src[n] = k;
                    n++;
                }
            }
            if(p.score < handscore(five)){
                p.score = handscore(five);
                for(int k=0;k<5;k++){
                    p.five[k] = five[k];
                    p.src[k] = src[k];
                }
             }
        }
    }
}

//打印牌型
void printHandRank(BestHand p){
    int cat = handRank(p.five);
    if(cat == 8)
        std::cout<<"FlushStraight"<<std::endl;
    if(cat == 7)
        std::cout<<"Four Of A Kind"<<std::endl;
    if(cat == 6)
        std::cout<<"Full House"<<std::endl;
    if(cat == 5)
        std::cout<<"Flush"<<std::endl;
    if(cat == 4)
        std::cout<<"Straight"<<std::endl;
    if(cat == 3)
        std::cout<<"Three Of A Kind"<<std::endl;
    if(cat == 2)
        std::cout<<"Two pairs"<<std::endl;
    if(cat == 1)
        std::cout<<"One Pair"<<std::endl;
    if(cat == 0)
        std::cout<<"High Cards"<<std::endl;
}

std::string HandrankName(BestHand p){
    int cat = handRank(p.five);
    if(cat == 8)
        return std::string("FlushStraight");
    if(cat == 7)
        return std::string("Four Of A Kind");
    if(cat == 6)
        return std::string("Full House");
    if(cat == 5)
        return std::string("Flush");
    if(cat == 4)
        return std::string("Straight");
    if(cat == 3)
        return std::string("Three Of A Kind");
    if(cat == 2)
        return std::string("Two pairs");
    if(cat == 1)
        return std::string("One pair");
    if(cat == 0)
        return std::string("High Cards");
    return std::string("");
}

//打印动作
void printAction(int i){
    if(i==1){
        std::cout<<"fold"<<std::endl;
    }
    if(i==2){
        std::cout<<"check"<<std::endl;
    }
    if(i==3){
        std::cout<<"call"<<std::endl;
    }
    if(i==4){
        std::cout<<"raise"<<std::endl;
    }
}

//轮次信息
void printActionInfo(Player p, int pot, int currentBet, int miniRaise, int nBoard, Card board[]){
    int toCall = currentBet - p.bet;
    std::cout << "your hand:" << std::endl;
    printcard(p.hole[0]);
    printcard(p.hole[1]);
    std::cout << "board:" << std::endl;
    for(int i=0;i<nBoard;i++)
        printcard(board[i]);
    std::cout << "pot=" << pot
              << " currentBet=" << currentBet << std::endl;
    std::cout << "yourChips=" << p.chips
              << " yourBet=" << p.bet << std::endl;
    std::cout << "canCheck=" << canCheck(p, currentBet)
              << " toCall=" << toCall << std::endl;
    std::cout << "minRaiseTo=" << currentBet + miniRaise
              << " maxRaiseTo=" << p.bet + p.chips << std::endl;
}


//玩家人数
int readPlayerNum(){
    int n = 0;
    std::cout<<"Please enter the number of players(2~9):"<<std::endl;
    while(1){
        std::cin>>n;
        if(n<2 || n>9){
            std::cout<<"Invalid number of player, please re-enter(2~9):"<<std::endl;
        }
        else
            break;
    }
    return n;
}

//游戏局数
int readTotalHands(){
    int n=0;
    std::cout<<"Please set the number of games:" << std::endl;
    while(1){
        std::cin>>n;
        if(n < 1){
            std::cout<<"Invalid number of games, please re-enter:"<<std::endl;
        }
        else
            break;
    }
    return n;
}

int readStartChips(){
    int n = 1000;
    std::cout << "Please set the starting chips(Default 1000.>=10):" << std::endl;
    while(1){
        std::cin >> n;
        if(n < 10){
            std::cout << "Invalid chips, please re-enter(>=10):" << std::endl;
        }
        else
            break;
    }
    return n;
}

//玩家动作
int readAction(Player p, int currentBet, int &newBet, int miniRaise){
    int a = 0;
    while(1){
        std::cout << "1=fold  2=check  3=call  4=raise" << std::endl;
        std::cin>>a;
        if(a<1 || a>4){
            std::cout << "Invalid action, re-enter:" << std::endl;
        }
        else if(a == 2 && !canCheck(p, currentBet))
            std::cout << "Cannot check, re-enter:" << std::endl;
        else if(a == 4){
            std::cout << "please enter-raise:";
            std::cin >> newBet;
            if(newBet - p.bet > p.chips)
                newBet = p.bet + p.chips;
            if(newBet <= currentBet){
                if(p.chips < currentBet - p.bet){
                    a = 3;
                    break;
                }
                std::cout << "Invalid raise, re-enter:" << std::endl;
            }
            else if(newBet - p.bet < p.chips && newBet - currentBet < miniRaise)
                std::cout << "Invalid raise, re-enter:" << std::endl;
            else
                break;
        }
        else
            break;
    }
    return a;
}

//计算当前最小边池
int minPositive(int remain[], int n){
    int min = 0;
    for(int i=0;i<n;i++){
        if(remain[i] > 0 && ( min == 0 || min > remain[i]))
            min = remain[i];
    }
    return min;
}

//边池
int peelLayer(int remain[], int n){
    int peel = minPositive(remain, n);
    int amt = 0;
    if(peel == 0)
        return 0;
    for(int i=0;i<n;i++){
        if(remain[i]>0){
            remain[i] = remain[i] - peel;
            amt += peel;
        }
    }
    return amt;
}

//边池名单
int layerElig(Player seats[], int remain[], int n, int elig[]){
    int cnt = 0;
    for(int i=0;i<n;i++){
        if(remain[i] > 0 && !seats[i].folded)
            elig[cnt++] = i;
    }
    return cnt;
}

//边池赢家
int showDownLayer(BestHand p[], int elig[], int ecnt, int win[]){
    int cnt = 0;
    int max = -1;
    for(int i=0;i<ecnt;i++){
        if(p[elig[i]].score > max){
            max = p[elig[i]].score;
        }
    }
    for(int i=0;i<ecnt;i++){
        if(max == p[elig[i]].score)
            win[cnt++] = elig[i];
    }
    return cnt;
}