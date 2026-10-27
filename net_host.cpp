#include "net.h"
#include "card.h"
int runServer(int port, int startChips, int bigBlind, int maxPlayers, int handNum){
    signal(SIGPIPE, SIG_IGN);
    //int startChips = readStartChips();
    //std::cout << "start chips=" << startChips << std::endl;

    int fd = hostListen(port);
    if(fd < 0)
        return 1;
    std::cout << "listening on " << port << std::endl;

    int cfd[9];
    std::string nicks[9];
    int ready[9] = {0};  
    int nclient = 0;
    while(1){
        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        int maxfd = fd;
        for(int i = 0; i < nclient; i++){      // ★ 每个客户端都放进 select
            FD_SET(cfd[i], &set);
            if(cfd[i] > maxfd)
                maxfd = cfd[i];
        }
        if(select(maxfd + 1, &set, NULL, NULL, NULL) < 0){
            close(fd);
            return 1;
        }
        if(FD_ISSET(fd, &set)){
            if(nclient >= maxPlayers){
                int extra = hostAccept(fd);
                close(extra);
                std::cout << "room full" << std::endl;
            }
            else{
                cfd[nclient] = hostAccept(fd);
                if(cfd[nclient] < 0){
                    close(fd);
                    return 1;
                }
                int cmd = 0;
                if(recvInt(cfd[nclient], cmd) < 0 || cmd != 22){
                    std::cout << "bad join, kick" << std::endl;
                    close(cfd[nclient]);
                }
                else if(recvStr(cfd[nclient], nicks[nclient]) < 0){
                    close(cfd[nclient]);
                }
                else{
                    std::cout << "player joined: " << nicks[nclient] << std::endl;
                    int step30 = 30;                     // 告诉本人：你是几号
                    sendInt(cfd[nclient], step30);
                    sendInt(cfd[nclient], nclient);
                    nclient++;
                    sendRoster(cfd, nicks, ready, nclient);
                }
            }
            continue;
        }

        // 2) 客户端来信：准备切换 / 房主开始 / 断开
        bool started = false;
        for(int i = 0; i < nclient; i++){
            if(!FD_ISSET(cfd[i], &set))
                continue;
            int cmd = 0;
            if(recvInt(cfd[i], cmd) < 0){
                if(i == 0){                              // 房主断了：散场
                    std::cout << "host disconnected" << std::endl;
                    close(fd);
                    return 1;
                }
                std::cout << "player" << i << " left lobby" << std::endl;
                close(cfd[i]);                           // 玩家断了：踢出并左移数组
                for(int k = i; k < nclient - 1; k++){
                    cfd[k] = cfd[k+1];
                    nicks[k] = nicks[k+1];
                    ready[k] = ready[k+1];
                }
                nclient--;
                sendRoster(cfd, nicks, ready, nclient);
                continue;
            }
            if(cmd == 21){                               // 切换准备
                ready[i] = 1 - ready[i];
                sendRoster(cfd, nicks, ready, nclient);
                continue;
            }
            if(cmd == 20 && i == 0){                     // 房主开始：查条件
                bool ok = (nclient >= 2);
                for(int j = 1; ok && j < nclient; j++)
                    if(!ready[j])
                        ok = false;
                if(ok){
                    std::cout << "game start" << std::endl;
                    started = true;
                    break;
                }
                std::cout << "not all ready" << std::endl;
            }
        }
        if(started)
            break;
        
    }

    int player_num = nclient;
    for(int i=0;i<player_num;i++){
        int step = 10;
        if(sendInt(cfd[i], step) < 0)
            std::cout << "send failed" << std::endl;

        int num = i;
        if(sendInt(cfd[i], num) < 0)
            std::cout << "send failed" << std::endl;
        if(sendInt(cfd[i], player_num) < 0)
            std::cout << "send failed" << std::endl;
        for(int j=0;j<player_num;j++){
            if(sendStr(cfd[i], nicks[j]) < 0)
                std::cout << "send failed" << std::endl;
        }
    }

    int top = 0;
    
    Player player[player_num];
    initPlayers(player, player_num, startChips);
    int BTN = player_num - 1;
    int pot = 0;

    //int handNum = 3;
    srand(time(NULL));

    for(int hand=0;hand<handNum;hand++){


        Card deck[52];
        Card hole[player_num][2];
        Card board[5];
        initDeck(deck);
        shufflecard(deck);

        for(int j=0;j<2;j++){
            for(int i=0;i<player_num;i++)
                hole[i][j] = dealcard(deck, top);
        }

        dealflop(deck, top, board);
        dealturn(deck, top, board);
        dealriver(deck, top, board);

        std::cout << "\nhand " << hand + 1 << std::endl;
        if(hand > 0)
            resetHand(player, player_num);

        std::cout << "\n";
        std::cout << "your cards:" << std::endl;
        printcard(hole[0][0]);
        printcard(hole[0][1]);

        for(int i=0;i<player_num;i++){
            int step = 11;
            if(sendInt(cfd[i], step) < 0)
                std::cout << "send failed" << std::endl;
            for(int j=0;j<2;j++){
                if(sendInt(cfd[i], hole[i][j].suit) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[i], hole[i][j].rank) < 0)
                    std::cout << "send failed" << std::endl;
            }
        }

        for(int i=0;i<player_num;i++){
            player[i].hole[0] = hole[i][0];
            player[i].hole[1] = hole[i][1];  
        }

        int sb = sbIndex(player, BTN, player_num);
        int bb = bbIndex(player, BTN, player_num);


        bool lastwin = false;
        for(int round=0;round<4;round++){
            
            int currentBet = bigBlind;
            int miniRaise = bigBlind;

            int n = 0;
            int from = 0;
            if(round == 0){
                payBlind(player[sb], pot, bigBlind/2);
                payBlind(player[bb], pot, bigBlind);
                for(int i = 0; i < player_num ; i++){
                    int step = 14;
                    if(sendInt(cfd[i], step) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], sb) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], bb) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], bigBlind) < 0)
                        std::cout << "send failed" << std::endl;
                    for(int j = 0; j < player_num; j++){
                        if(sendInt(cfd[i], player[j].chips) < 0)
                            std::cout << "send failed" << std::endl;
                    }
                    if(sendInt(cfd[i], pot) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], currentBet) < 0)
                        std::cout << "send failed" << std::endl;

                    
                }
                
            }
            if(round == 1){
                n = 3;
                from = 0;
            }
            if(round == 2){
                n = 1;
                from = 3;
            }
            if(round == 3){
                n = 1;
                from = 4;
            }
            if(round > 0){
                resetBets(player, player_num);  
                currentBet = 0;
            }
        
            for(int i=0;i<player_num;i++){
                int step = 12;
                if(sendInt(cfd[i], step) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[i], n) < 0)
                    std::cout << "send failed" << std::endl;
                for(int k=0;k<n;k++){
                    if(sendInt(cfd[i], board[from+k].suit) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], board[from+k].rank) < 0)
                        std::cout << "send failed" << std::endl;
                }   
            }

            std::cout << "\n";
            std::cout << "board:" << std::endl;
            for(int k=0;k<n+from;k++)
                printcard(board[k]);

            int start = firstAvtivePlayer(player, player_num, BTN);
            if(round == 0)
                start = firstAvtivePlayer(player, player_num, bb);

            int who = start;

            while(who >= 0 && !bettingOver(player, player_num, currentBet)){
                int newBet = 0;
                int act = 0;

                
                int step = 15;
                for(int i=0;i<player_num;i++){
                    if(sendInt(cfd[i], step) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], who) < 0)
                        std::cout << "send failed" << std::endl;
                }
                

                int code = 1;
                if(sendInt(cfd[who], code) < 0)
                    std::cout << "send failed" << std::endl;

                int tocall = currentBet - player[who].bet;
                if(tocall < 0)
                    tocall = 0;
                if(sendInt(cfd[who], pot) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[who], currentBet) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[who], tocall) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[who], player[who].chips) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[who], player[who].bet) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[who], miniRaise) < 0)
                    std::cout << "send failed" << std::endl;

                if(recvInt(cfd[who], act) < 0)
                    std::cout << "recv failed" << std::endl;
                if(act == 4){
                    if(recvInt(cfd[who], newBet) < 0)
                        std::cout << "recv failed" << std::endl;
                    if(newBet - player[who].bet > player[who].chips)
                        newBet = player[who].bet + player[who].chips;
                    if(newBet <= currentBet)
                        act = 3;
                    else if(newBet - player[who].bet < player[who].chips
                            && newBet - currentBet < miniRaise)
                        act = 3;
                }
                std::cout << "\n";
                std::cout << "player" << who << " " << std::endl;
                printAction(act);
                if(act == 4)
                    std::cout << "to " << newBet <<std::endl;
                
                doAction(player, pot, currentBet, newBet, act, miniRaise, player_num, who);
                for(int i=0;i<player_num;i++){
                    //if(i == who - 1)
                        //continue;
                    int info = 2;
                    if(sendInt(cfd[i], info) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], who) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], act) < 0)
                        std::cout << "send failed" << std::endl;
                    if(act == 3)
                        newBet = player[who].bet;
                    if(sendInt(cfd[i], newBet) < 0)
                        std::cout << "send failed" << std::endl;

                    if(sendInt(cfd[i], player[who].chips) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], pot) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], currentBet) < 0)
                        std::cout << "send failed" << std::endl;
                    if(sendInt(cfd[i], miniRaise) < 0)
                        std::cout << "send failed" << std::endl;

                    if(sendInt(cfd[i], player[who].bet) < 0)
                        std::cout << "send failed" << std::endl;
                }
                who = nextActive(player, player_num, who);
                
            }

            usleep(3000000); 

            for(int i = 0; i < player_num; i++){
                int code = 0;
                if(sendInt(cfd[i], code) < 0)
                    std::cout << "send failed" << std::endl;
            }

            if(countActivePlayer(player, player_num) == 1){
                lastwin = true;
                if(round < 3){
                    int stop = -1;
                    for(int i=0;i<player_num;i++){
                        if(sendInt(cfd[i], stop) < 0)
                            std::cout << "send failed" << std::endl;
                    }
                }
                break;
            }
        }

        int winnerArr[player_num] = {0};
        BestHand p[player_num];
        if(lastwin){
            int win = lastActive(player, player_num);
            player[win].chips += pot;
            winnerArr[win] = 1;
            //std::cout << "winner seat " << win << std::endl;
        }
        else{
            for(int i=0;i<player_num;i++)
                bestScore(player[i].hole, board, p[i]);

            int remain[player_num] = {0};
            for(int i=0;i<player_num;i++)
                remain[i] = player[i].put;
            
            for(int r = 0; r < player_num; r++){
                int elig[player_num];
                int win[player_num];
                int ecnt = layerElig(player, remain, player_num, elig);
                if(ecnt == 0){
                    for(int i = 0; i < player_num; i++){
                        player[i].chips += remain[i];
                        remain[i] = 0;
                    }
                    break;
                }
                int peel = peelLayer(remain, player_num);
                if(peel == 0)
                    break;
                int wincnt = showDownLayer(p, elig, ecnt, win);
                if(wincnt == 0)
                    break;
                int potcut = peel / wincnt;
                int lastpot = peel % wincnt;
                for(int i = 0; i < wincnt; i++){
                    std::cout << "player" << win[i] << std::endl;
                    printHandRank(p[win[i]]);
                    player[win[i]].chips += potcut;
                    if(r == 0)                 // 只有第一层是主池
                        winnerArr[win[i]] = 1;
                }
                player[win[0]].chips += lastpot;
            }
            for(int i = 0; i < player_num; i++)
                player[i].chips += remain[i];
        }

        int step = 13;
        for(int i=0;i<player_num;i++){
            if(sendInt(cfd[i], step) < 0)
                std::cout << "send failed" << std::endl;
        }


        int show = 0;
        if(!lastwin)
            show = 1;
        for(int i=0;i<player_num;i++){
            if(sendInt(cfd[i], show) < 0)
                std::cout << "send failed" << std::endl;
            if(show == 0)
                continue;
            for(int j=0;j<player_num;j++){
                int folded = 0;
                if(player[j].folded)
                    folded = 1;
                if(sendInt(cfd[i], folded) < 0)
                    std::cout << "send failed" << std::endl;
                if(folded == 1)
                    continue;
                
                if(sendInt(cfd[i], player[j].hole[0].suit) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[i], player[j].hole[0].rank) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[i], player[j].hole[1].suit) < 0)
                    std::cout << "send failed" << std::endl;
                if(sendInt(cfd[i], player[j].hole[1].rank) < 0)
                    std::cout << "send failed" << std::endl;

                std::cout << "\n";
                printHandRank(p[i]);
                for(int j=0;j<5;j++)
                    printcard(p[i].five[j]);
            }
        }

        std::cout << "\n";
        for(int i=0;i<player_num;i++)
            std::cout << "player" << i << " chips=" << player[i].chips << std::endl;
        
        for(int i=0;i<player_num;i++){
            for(int j=0;j<player_num;j++){
                if(sendInt(cfd[i], player[j].chips) < 0)
                    std::cout << "send failed" << std::endl;
            }
        }

        std::cout << "\n";
        int again = 0;
        if(hand != handNum - 1)
            again = 1;
        for(int i=0;i<player_num;i++){
            if(sendInt(cfd[i], again) < 0)
                std::cout << "send failed" << std::endl;
        }
        std::cout << "again=" << again << std::endl;

        for(int i=0;i<player_num;i++){
            for(int j=0;j<player_num;j++){
                if(sendInt(cfd[i], winnerArr[j]) < 0)
                    std::cout << "send failed" << std::endl;
            }
        }

        
        pot = 0;
        top = 0;
        BTN = (BTN + 1)%player_num;

        if(again == 1)
            usleep(10000000); 
    }

    for(int i = 0; i < player_num; i++)
        close(cfd[i]);
    close(fd);
    return 0;
}

// 控制台版服务器入口：开发调试用
#ifndef EMBEDDED_SERVER

int main(){
     return runServer(8888, 1000, 10, 9, 3);
}
#endif
