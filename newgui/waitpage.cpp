#include "waitpage.h"

WaitPage::WaitPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("等待室", this);
    layout->addWidget(title);

    for(int i=0;i<9;i++){
        roster[i] = new QLabel("", this);
        layout->addWidget(roster[i]);
    }

    readyBtn = new QPushButton("准备", this);
    layout->addWidget(readyBtn);
    connect(readyBtn, &QPushButton::clicked, this, &WaitPage::readyClicked);

    startBtn = new QPushButton("开始", this);
    layout->addWidget(startBtn);
    connect(startBtn, &QPushButton::clicked, this, &WaitPage::startClicked);


}

void WaitPage::updateRoster(int count, const QStringList &nicks, const QVector<int> &ready, int hostIdx){
    for(int i = 0; i < 9; i++){
        if(i < count){
            QString line = nicks[i];
            if(i == hostIdx)
                line += "  [房主]";
            line += ready[i] ? "  ✓已准备" : "  ✗未准备";
            roster[i]->setText(line);
        }
        else
            roster[i]->setText("");
    }
}

void WaitPage::setMyIndex(int idx){
    if(idx == 0){          // 我是房主：用开始按钮
        readyBtn->hide();
        startBtn->show();
    }
    else{                  // 我是玩家：用准备按钮
        readyBtn->show();
        startBtn->hide();
    }
}

void WaitPage::setCanStart(bool ok){
    startBtn->setEnabled(ok);
}
































