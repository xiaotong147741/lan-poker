#ifndef WAITPAGE_H
#define WAITPAGE_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>
#include <QVector>

class WaitPage : public QWidget
{
    Q_OBJECT
public:

    WaitPage(QWidget *parent = nullptr);
    // 设置第 idx 行的名单文字（下一步由 31 消息驱动）
    void updateRoster(int count, const QStringList &nicks, const QVector<int> &ready, int hostIdx);
    // 你是几号：0=房主（显示开始按钮），其他=玩家（显示准备按钮）
    void setMyIndex(int idx);
    // 房主开始按钮是否可用（人数够 + 全员已准备）
    void setCanStart(bool ok);

signals:

    void readyClicked();    // 点了"准备/取消准备"
    void startClicked();    // 房主点了"开始游戏"

private:

    QLabel *roster[9];
    QPushButton *readyBtn;
    QPushButton *startBtn;




};

#endif // WAITPAGE_H
