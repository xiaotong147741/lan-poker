#include "connectpage.h"

ConnectPage::ConnectPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QGroupBox *createBox = new QGroupBox("创建房间", this);
    QFormLayout *createForm = new QFormLayout(createBox);

    createNick = new QLineEdit(createBox);

    portSpin = new QSpinBox(createBox);
    portSpin->setRange(1024, 65535);   // 1024 以下需要管理员权限，避开
    portSpin->setValue(8888);

    chipsSpin = new QSpinBox(createBox);
    chipsSpin->setRange(500, 10000);
    chipsSpin->setValue(1000);

    bbSpin = new  QSpinBox(createBox);
    bbSpin->setRange(2, 1000);         // 小盲自动取一半，后面实现
    bbSpin->setValue(10);

    maxSpin = new QSpinBox(createBox);
    maxSpin->setRange(2, 9);
    maxSpin->setValue(9);

    hNumSpin = new QSpinBox(createBox);
    hNumSpin->setRange(1,1000);
    hNumSpin->setValue(20);

    ipShow = new QLineEdit(createBox);
    ipShow->setReadOnly(true);          // 只读，不能改
    ipShow->setText(localIP());

    QPushButton *createBTN = new QPushButton("创建房间", createBox);

    // addRow("文字", 控件) 会自动生成左边的说明标签
    createForm->addRow("昵称", createNick);
    createForm->addRow("端口", portSpin);
    createForm->addRow("初始筹码", chipsSpin);
    createForm->addRow("大盲注", bbSpin);
    createForm->addRow("最大人数", maxSpin);
    createForm->addRow("局数", hNumSpin);
    createForm->addRow(createBTN);      // 单个控件占整行

    createForm->addRow("本机 IP", ipShow);

    connect(createBTN, &QPushButton::clicked, this, &ConnectPage::oneCreateClicked);

    layout->addWidget(createBox);

    statusLabel = new QLabel(this);
    layout->addWidget(statusLabel);



    QGroupBox *joinBox = new QGroupBox("加入房间", this);
    QFormLayout *joinForm = new QFormLayout(joinBox);

    joinNick = new QLineEdit(joinBox);

    ipEdit = new QLineEdit(joinBox);
    ipEdit->setText("127.0.0.1");

    joinPortSpin = new QSpinBox(joinBox);
    joinPortSpin->setRange(1024, 65535);
    joinPortSpin->setValue(8888);

    QPushButton *joinBTN = new QPushButton("加入房间", joinBox);

    joinForm->addRow("昵称", joinNick);
    joinForm->addRow("房间 ip", ipEdit);
    joinForm->addRow("端口号", joinPortSpin);
    joinForm->addRow(joinBTN);

    connect(joinBTN, &QPushButton::clicked, this, &ConnectPage::oneJoinClicked);

    layout->addWidget(joinBox);



}

void ConnectPage::setStatus(const QString &text){
    statusLabel->setText(text);
}

void ConnectPage::oneCreateClicked(){
    QString nick = createNick->text().trimmed();
    if(nick.isEmpty()){
        setStatus("昵称不能为空");
        return ;
    }
    setStatus("");
    emit createRequested(nick, portSpin->value(), chipsSpin->value(), bbSpin->value(), maxSpin->value(), hNumSpin->value());
}

void ConnectPage::oneJoinClicked(){
    QString nick = joinNick->text().trimmed();
    QString ip   = ipEdit->text().trimmed();
    if(nick.isEmpty()){
        setStatus("昵称不能为空");
        return ;
    }
    if(ip.isEmpty()){
        setStatus("房主 IP 不能为空");
        return ;
    }
    setStatus("");
    emit joinRequested(nick, ip, joinPortSpin->value());
}

QString ConnectPage::localIP(){
    const QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for(const QNetworkInterface &iface : ifaces){
        if(!iface.flags().testFlag(QNetworkInterface::IsUp))
            continue;
        if(iface.flags().testFlag(QNetworkInterface::IsLoopBack))
            continue;
        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for(const QNetworkAddressEntry &entry : entries){
            QHostAddress ip = entry.ip();
            if(ip.protocol() == QAbstractSocket::IPv4Protocol)
                return ip.toString();
        }
    }
    return QString("未找到");
}

























