#ifndef THREAD_H
#define THREAD_H

#include "declarations.h"
#include <QThread>
#include <QTcpSocket>
#include <QUdpSocket>
//#include "player.h"

class SnsThread : public QThread
{
    Q_OBJECT

    QTcpSocket *tcpSocket;

    int id;

    gameCore *core;

public:
    int socketDescriptor;
    bool ready;

    player *p;
    double xMouse;
    double yMouse;
    bool pressedW, pressedA, pressedS, pressedD, pressedL, pressedR;

    explicit SnsThread(gameCore *parent);
    bool grabConnection(int socketDescriptor, int _id);
    ~SnsThread();
    bool operator==(SnsThread r);

    void process();

    void sendData();
    void sendPlayerData();
    void sendUpdatedData();
    void sendLevelData();
    void sendProjectilesData();

    int getid();

private slots:
    void readPacket();
    void clientLostConnection();
};

#endif // THREAD_H
