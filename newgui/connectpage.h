#ifndef CONNECTPAGE_H
#define CONNECTPAGE_H

#include <QWidget>
#include <QString>

#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>

#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>


class ConnectPage:public QWidget{

    Q_OBJECT

public:
    ConnectPage(QWidget *parent = nullptr);


    // 给 MainWindow 用：以后连接失败时写提示文案到状态栏
    void setStatus(const QString &text);

private:



signals:

    // 点击"创建房间"：昵称、端口、初始筹码、大盲、最大人数
    void createRequested(QString nick, int port, int startchips, int bigblind, int maxplayer, int handnum);

    // 点击"加入房间"：昵称、房主IP、端口
    void joinRequested(QString nick, QString ip, int port);

private:

    // ---- 创建房间表单 ----
    QLineEdit *createNick;
    QSpinBox  *portSpin;      // 监听端口
    QSpinBox  *chipsSpin;     // 初始筹码
    QSpinBox  *bbSpin;        // 大盲
    QSpinBox  *maxSpin;       // 最大人数 2~9
    QSpinBox  *hNumSpin;       // 局数

    // ---- 加入房间表单 ----
    QLineEdit *joinNick;
    QLineEdit *ipEdit;        // 房主 IP
    QSpinBox  *joinPortSpin;

    QLabel *statusLabel;      // 底部状态提示

    QLineEdit *ipShow;          // 本机 IP（只读展示）
    static QString localIP();   // 枚举网卡找局域网 IP



    void oneCreateClicked();
    void oneJoinClicked();


};

#endif // CONNECTPAGE_H
