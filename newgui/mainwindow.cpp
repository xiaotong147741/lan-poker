#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectpage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sock(nullptr)

{
    ui->setupUi(this);

    pages = new QStackedWidget(this);//主窗口的中心部件换成堆栈
    setCentralWidget(pages);
    QCoreApplication::removePostedEvents(ui->centralwidget, QEvent::DeferredDelete);

    ConnectPage *cp = new ConnectPage(this);
    wait = new WaitPage(this);
    pages->addWidget(cp);
    pages->addWidget(wait);
    pages->addWidget(ui->centralwidget);//addWidget 的顺序就是页码：先加的大厅是 0 页，后加的牌桌是 1 页；
    pages->setCurrentIndex(PageConnect);      // 先显示连接页

    connect(cp, &ConnectPage::createRequested, this,[this, cp](QString nick, int port, int chips, int bb, int maxN, int hNum){
        if(g.fd >= 0){
            cp->setStatus("已经连接了房间");
            return ;
        }

        std::thread t(runServer, port, chips, bb, maxN, hNum);
        t.detach();

        for(int i=0;i<20&&g.fd<0;i++){
            connectHost("127.0.0.1", port, nick);
            if(g.fd < 0)
                usleep(50000);
        }

        if(g.fd < 0)
            cp->setStatus("创建失败：端口可能被占用");
        else{
            cp->setStatus("创建并链接房间成功");
            pages->setCurrentIndex(PageWait);
        }

    });
    connect(cp, &ConnectPage::joinRequested, this,[this, cp](QString nick, QString ip, int port){
        if(g.fd >= 0){
            cp->setStatus("已经连接过了");
            return ;
        }
        connectHost(ip, port, nick);
        if(g.fd < 0)
            cp->setStatus("连接失败，请检查 IP 和端口");
        else{
            cp->setStatus("已连接，等待房主开始游戏");
            pages->setCurrentIndex(PageWait);
        }
    });


    connect(wait, &WaitPage::readyClicked, this,[this]{
        int cmd = 21;
        sendInt(g.fd, cmd);
    });
    connect(wait, &WaitPage::startClicked, this, [this]{
        int cmd = 20;
        sendInt(g.fd, cmd);
    });

    g.fd = -1;
    setButtons(false, false);
    g.seat = -1;
    g.round = 0;
    //connectHost();
    initSeat();
    ui->miniraise->setText("10");
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::initSeat(){
    seats[0] = ui->seat0;
    seats[1] = ui->seat1;
    seats[2] = ui->seat2;
    seats[3] = ui->seat3;
    seats[4] = ui->seat4;
    seats[5] = ui->seat5;
    seats[6] = ui->seat6;
    seats[7] = ui->seat7;
    seats[8] = ui->seat8;

    acts[0] = ui->act0;
    acts[1] = ui->act1;
    acts[2] = ui->act2;
    acts[3] = ui->act3;
    acts[4] = ui->act4;
    acts[5] = ui->act5;
    acts[6] = ui->act6;
    acts[7] = ui->act7;
    acts[8] = ui->act8;

    chips[0] = ui->chips0;
    chips[1] = ui->chips1;
    chips[2] = ui->chips2;
    chips[3] = ui->chips3;
    chips[4] = ui->chips4;
    chips[5] = ui->chips5;
    chips[6] = ui->chips6;
    chips[7] = ui->chips7;
    chips[8] = ui->chips8;

    ha[0] = ui->h0a;
    ha[1] = ui->h1a;
    ha[2] = ui->h2a;
    ha[3] = ui->h3a;
    ha[4] = ui->h4a;
    ha[5] = ui->h5a;
    ha[6] = ui->h6a;
    ha[7] = ui->h7a;
    ha[8] = ui->h8a;

    hb[0] = ui->h0b;
    hb[1] = ui->h1b;
    hb[2] = ui->h2b;
    hb[3] = ui->h3b;
    hb[4] = ui->h4b;
    hb[5] = ui->h5b;
    hb[6] = ui->h6b;
    hb[7] = ui->h7b;
    hb[8] = ui->h8b;

    hr[0] = ui->handrank0;
    hr[1] = ui->handrank1;
    hr[2] = ui->handrank2;
    hr[3] = ui->handrank3;
    hr[4] = ui->handrank4;
    hr[5] = ui->handrank5;
    hr[6] = ui->handrank6;
    hr[7] = ui->handrank7;
    hr[8] = ui->handrank8;
}

void MainWindow::showFlop(){
    ui->b0->setText(QString::fromStdString(cardName(g.board[0])));
    ui->b1->setText(QString::fromStdString(cardName(g.board[1])));
    ui->b2->setText(QString::fromStdString(cardName(g.board[2])));
    ui->info->setText("翻牌");
}

void MainWindow::showTurn(){
    ui->b3->setText(QString::fromStdString(cardName(g.board[3])));
    ui->info->setText("转牌");
}

void MainWindow::showRiver(){
    ui->b4->setText(QString::fromStdString(cardName(g.board[4])));
    ui->info->setText("河牌");
}

void MainWindow::nextStreet(){
    if(g.round == 0)
        showFlop();
    else if(g.round == 1)
        showTurn();
    else if(g.round == 2)
        showRiver();
    else
        return ;

    g.round++;
    g.currentBet = 0;
    g.miniRaise = 10;
    resetBets(&g.me, 1);
    ui->curNum->setText("0");
    ui->myBet->setText("0");
    ui->miniraise->setText("10");


    if(!g.me.folded)
        ui->actText->setText("暂无");

    for(int i = 0; i < g.player_num; i++){
        if(acts[i]->text() != QString("弃牌") && !acts[i]->text().startsWith("All in"))
            acts[i]->setText("");
    }
}

void MainWindow::readJoin(){
    if(recvInt(g.fd, g.seat) < 0)
        return ;
    if(recvInt(g.fd, g.player_num) < 0)
        return ;
    for(int i = 0; i < 9; i++)
        seats[i]->parentWidget()->setVisible(i < g.player_num);

    for(int i=0;i<g.player_num;i++){
        std::string s;
        if(recvStr(g.fd, s) < 0)
            return ;
        g.nicks[i] = QString::fromStdString(s);
        seats[i]->setText(g.nicks[i]);
    }
}

void MainWindow::readHole(){
    clearHole();
    for(int i=0;i<2;i++){
        if(recvInt(g.fd, g.me.hole[i].suit) < 0)
            return ;
        if(recvInt(g.fd, g.me.hole[i].rank) < 0)
            return ;
    }

    g.round = 0;
    g.me.folded = false;
    ui->info->setText("翻牌前");
    ui->actText->setText("");
    ui->myBet->setText("0");
    ui->b0->setText("");
    ui->b1->setText("");
    ui->b2->setText("");
    ui->b3->setText("");
    ui->b4->setText("");
    for(int i = 0; i < 9; i++)
        acts[i]->setText("");

    ui->hole1->setText(QString::fromStdString(cardName(g.me.hole[0])));
    ui->hole2->setText(QString::fromStdString(cardName(g.me.hole[1])));

    for(int i=0;i<9;i++)
        hr[i]->setText("");

}

void MainWindow::readStreet(){
    int n = 0;
    if(recvInt(g.fd, n) < 0)
        return ;
    int from = 0;
    if(g.round == 1)
        from = 3;
    if(g.round == 2)
        from = 4;

    for(int i=0;i<n;i++){
        if(recvInt(g.fd, g.board[from+i].suit) < 0)
            return ;
        if(recvInt(g.fd, g.board[from+i].rank) < 0)
            return ;
    }
    if(n > 0)
        nextStreet();
}

void MainWindow::readTurnInfo(){
    int tocall = 0;
    if(recvInt(g.fd, g.pot) < 0)
        return;
    if(recvInt(g.fd, g.currentBet) < 0)
        return;
    if(recvInt(g.fd, tocall) < 0)
        return;
    if(recvInt(g.fd, g.me.chips) < 0)
        return;
    if(recvInt(g.fd, g.me.bet) < 0)
        return;
    if(recvInt(g.fd, g.miniRaise) < 0)
        return;
    ui->potNum->setText(QString::number(g.pot));
    ui->curNum->setText(QString::number(g.currentBet));
    ui->myChips->setText(QString::number(g.me.chips));
    ui->myBet->setText(QString::number(g.me.bet));
    ui->miniraise->setText(QString::number(g.miniRaise));
    setButtons(true, tocall == 0);
    showActivePlayer(g.seat);
}

QString MainWindow::actWords(int act, int newBet){
    if(act == 1)
        return "弃牌";
    if(act == 2)
        return "过牌";
    if(act == 3)
        return "跟注" + QString::number(newBet);
    if(act == 4)
        return "加注" + QString::number(newBet);
    return "";
}

void MainWindow::readOther(){
    int who;
    int act;
    int newBet;

    if(recvInt(g.fd, who) < 0)
        return ;
    if(recvInt(g.fd, act) < 0)
        return ;
    if(recvInt(g.fd, newBet) < 0)
        return ;


    int left = 0;
    int potNow = 0;
    int cur = 0;
    int mnr = 0;
    if(recvInt(g.fd, left) < 0)
        return;
    if(recvInt(g.fd, potNow) < 0)
        return;
    if(recvInt(g.fd, cur) < 0)
        return ;
    if(recvInt(g.fd, mnr) < 0)
        return ;
    chips[who]->setText(QString::number(left));
    g.pot = potNow;
    ui->potNum->setText(QString::number(potNow));
    g.currentBet = cur;
    ui->curNum->setText(QString::number(cur));
    g.miniRaise = mnr;
    ui->miniraise->setText(QString::number(mnr));

    int mb = 0;
    if(recvInt(g.fd, mb) < 0)
        return ;


    if(who == g.seat){
        g.me.bet = mb;
        g.me.chips = left;
        ui->myBet->setText(QString::number(mb));
        ui->myChips->setText(QString::number(left));

        if(left == 0)
            ui->actText->setText("All in " + QString::number(mb));
        else
            ui->actText->setText(actWords(act, newBet));
    }

    if(left == 0)
        acts[who]->setText("All in " + QString::number(mb));
    else
        acts[who]->setText(actWords(act, newBet));
    //showActivePlayer(who);
}

void MainWindow::readShow(){
    int show = 0;
    if(recvInt(g.fd, show) < 0)
        return ;
    if(show == 1){
        for(int i=0;i<g.player_num;i++){
            int folded = 0;
            if(recvInt(g.fd, folded) < 0)
                return ;
            if(folded == 1)
                continue;
            Card c[2];
            for(int j=0;j<2;j++){
                if(recvInt(g.fd, c[j].suit) < 0)
                    return ;
                if(recvInt(g.fd, c[j].rank) < 0)
                    return ;
            }
            ha[i]->setText(QString::fromStdString(cardName(c[0])));
            hb[i]->setText(QString::fromStdString(cardName(c[1])));

            BestHand p;
            bestScore(c, g.board, p);
            hr[i]->setText(QString::fromStdString(HandrankName(p)));
        }
    }
    for(int j = 0; j < g.player_num; j++){
            int left = 0;
            if(recvInt(g.fd, left) < 0)
                return;
            chips[j]->setText(QString::number(left));
            if(j == g.seat){
                g.me.chips = left;
                ui->myChips->setText(QString::number(g.me.chips));
            }
        }
        int again = 0;
        if(recvInt(g.fd, again) < 0)
            return;
        showActivePlayer(-1);
        for(int j = 0; j < g.player_num; j++){
            int won = 0;
            if(recvInt(g.fd, won) < 0)
                return ;
            if(won == 1)
                seats[j]->setStyleSheet("background-color:#f1c40f;color:#000000;");
        }
}

void MainWindow::readChips(){
    int sb = 0;
    int bb = 0;
    int bigblind = 0;
    if(recvInt(g.fd, sb) < 0)
        return;
    if(recvInt(g.fd, bb) < 0)
        return;
    if(recvInt(g.fd, bigblind) < 0)
        return;
    acts[sb]->setText("小盲注 " + QString::number(bigblind/2));
    acts[bb]->setText("大盲注 " + QString::number(bigblind));
    for(int j = 0; j < g.player_num; j++)
        seats[j]->setText(g.nicks[j]);
    if(sb >= 0 && sb < g.player_num)
        seats[sb]->setText(seats[sb]->text() + " 小盲");
    if(bb >= 0 && bb < g.player_num)
        seats[bb]->setText(seats[bb]->text() + " 大盲");

    for(int i=0;i<g.player_num;i++){
        int left = 0;
        if(recvInt(g.fd, left) < 0)
            return ;
        chips[i]->setText(QString::number(left));
        if(i == g.seat){
            g.me.chips = left;
            ui->myChips->setText(QString::number(left));
        }
    }

    int potNow = 0;
    int cur = 0;

    if(recvInt(g.fd, potNow) < 0)
        return;
    if(recvInt(g.fd, cur) < 0)
        return;

    g.pot = potNow;
    ui->potNum->setText(QString::number(potNow));
    g.currentBet = cur;
    ui->curNum->setText(QString::number(cur));
    if(g.seat == sb){
        g.me.bet = bigblind / 2;
        ui->myBet->setText(QString::number(bigblind/2));
    }
    if(g.seat == bb){
        g.me.bet = bigblind;
        ui->myBet->setText(QString::number(bigblind));
    }
}

void MainWindow::connectHost(QString ip, int port, QString nick){
    g.fd = clientConnection(ip.toStdString().c_str(), port);
    if(g.fd < 0)
        return ;

    int cmd = 22;
    if(sendInt(g.fd, cmd) < 0)
        return ;
    if(sendStr(g.fd, nick.toStdString()) < 0)
        return ;

    sock = new QSocketNotifier(g.fd, QSocketNotifier::Read, this);
    connect(sock, &QSocketNotifier::activated, this, &MainWindow::onSock);

}

void MainWindow::readActivePlayer(){
    int who = 0;
    if(recvInt(g.fd, who) < 0)
        return ;
    showActivePlayer(who);
}

void MainWindow::readRoster(){
    int cnt = 0;
    if(recvInt(g.fd, cnt) < 0)
        return ;
    QStringList nicks;
    QVector<int> ready;
    for(int i = 0; i < cnt; i++){
        std::string s;
        int r = 0;
        if(recvStr(g.fd, s) < 0)
            return ;
        if(recvInt(g.fd, r) < 0)
            return ;
        nicks << QString::fromStdString(s);
        ready << r;
    }
    int hostIdx = 0;
    if(recvInt(g.fd, hostIdx) < 0)
        return ;
    wait->updateRoster(cnt, nicks, ready, hostIdx);

    bool ok = (g.seat == 0 && cnt >= 2);     // 房主视角：人数够才谈开始
    for(int i = 1; ok && i < cnt; i++)       // 除房主外全员已准备
        if(!ready[i])
            ok = false;
    wait->setCanStart(ok);
}

void MainWindow::onSock(){
    sock->setEnabled(false);
    int step = 0;
    if(recvInt(g.fd, step) < 0)
        return ;
    if(step == 10){
        readJoin();
        pages->setCurrentIndex(PageTable);
        if(g.seat >= 0)
            ui->mySeat->setText(g.nicks[g.seat]);
    }
    else if(step == 11){
        readHole();
    }
    else if(step == 12){
        readStreet();
    }
    else if(step == 1){
        readTurnInfo();
    }
    else if(step == 2){
        readOther();
    }
    else if(step == 13){
        readShow();
    }
    else if(step == 14){
        readChips();
    }
    else if(step == 15){
        readActivePlayer();
    }
    else if(step == 30){
        if(recvInt(g.fd, g.seat) < 0)
            return ;
        wait->setMyIndex(g.seat);   // 把"我是几号"传给等待室
    }
    else if(step == 31){
        readRoster();
    }

    sock->setEnabled(true);
}


void MainWindow::setButtons(bool myTurn, bool canCheck){
    ui->fold->setEnabled(myTurn);
    ui->check->setEnabled(myTurn && canCheck);
    ui->call->setEnabled(myTurn);
    ui->raise->setEnabled(myTurn);
    ui->raiseTo->setEnabled(myTurn);
}

void MainWindow::clearHole(){
    for(int i=0;i<9;i++){
        ha[i]->setText("");
        hb[i]->setText("");
    }
}

void MainWindow::showActivePlayer(int who){
    for(int i=0;i<9;i++)
        seats[i]->setStyleSheet("");
    if(who < 0 || who >= g.player_num)
            return ;
    seats[who]->setStyleSheet("background-color:#2ecc71;color:#ffffff;");
}

void MainWindow::on_fold_clicked(){
    int act = 1;
    if(sendInt(g.fd, act) < 0)
        return ;
    setButtons(false, false);
}

void MainWindow::on_check_clicked(){
    int act = 2;
    if(sendInt(g.fd, act) < 0)
        return ;
    setButtons(false, false);
}

void MainWindow::on_call_clicked(){
    int act = 3;
    if(sendInt(g.fd, act) < 0)
        return ;
    setButtons(false, false);
}

void MainWindow::on_raise_clicked(){
    int newBet = ui->raiseTo->text().toInt();
    bool allin = (newBet - g.me.bet == g.me.chips);
    if(!allin && (newBet < g.currentBet || newBet - g.currentBet < g.miniRaise )){
        QMessageBox::warning(this, "无效加注额",
                             "请满足最小加注尺度并且大于当前注额。");
        ui->raiseTo->setText("");
        return;
    }
    int act = 4;
    if(sendInt(g.fd, act) < 0)
        return ;
    if(sendInt(g.fd, newBet) < 0)
        return ;
    setButtons(false, false);
}



































