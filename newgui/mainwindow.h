#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSocketNotifier>
#include <QLabel>
#include <QMessageBox>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QDebug>
#include <QStackedWidget>
#include <thread>
#include "../card.h"
#include "../net.h"
#include "waitpage.h"

struct Game {

    Card board[5];
    int pot;
    int currentBet;
    int miniRaise;
    int round;
    Player me;

    int fd;
    int seat;
    int player_num;

    QString nicks[9];

};



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Game g;
    QSocketNotifier *sock;
    QLabel *seats[9];
    QLabel *acts[9];
    QLabel *chips[9];
    QLabel *ha[9];
    QLabel *hb[9];
    QLabel *hr[9];

    QStackedWidget *pages;
    WaitPage *wait;

    enum Page { PageConnect = 0, PageWait = 1, PageTable = 2 };

    void showFlop();
    void showTurn();
    void showRiver();

    void nextStreet();
    void setButtons(bool myTurn, bool canCheck);
    //void onNewCards(int n);


    void readJoin();
    void readHole();
    void readStreet();
    void readTurnInfo();
    void readOther();
    void readShow();
    void readChips();
    void connectHost(QString ip, int port, QString nick);

    void initSeat();
    QString actWords(int act, int newBet);

    void clearHole();
    void showActivePlayer(int who);
    void readActivePlayer();

    void readRoster();

private slots:
    void on_fold_clicked();
    void on_check_clicked();
    void on_call_clicked();
    void on_raise_clicked();

    void onSock();
};
#endif // MAINWINDOW_H
