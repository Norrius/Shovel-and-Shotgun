#include "thread.h"
#include "global.h"
#include "gamecore.h"
#include "player.h"
#include "projectile.h"
#include "io.h"

SnsThread::SnsThread(gameCore *parent) :
    QThread(parent)
{
    //qDebug() << "Created a thread";
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    core = parent;
    ready = false;
}

bool SnsThread::grabConnection(int socketDescriptor, int _id)
{
    tcpSocket = new QTcpSocket;
    if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "TCP socket descriptor setting failed";
        return false;
    }

    QDataStream stream(tcpSocket);

    // FIXME: anisochronous method, deadlock probabilty
    if (tcpSocket->waitForReadyRead(10000))
    {
        quint8 packetID;
        stream >> packetID;
        if (packetID != 0x00)
        {
            qDebug().nospace() << "Client "
                               << tcpSocket->peerAddress().toString()
                               << ":" << tcpSocket->peerPort()
                               << " failed to join: wrong join packet";
            return false;
        }
        quint32 magic;
        stream >> magic;
        if (magic != S_magic)
        {
            qDebug().nospace() << "Client "
                               << tcpSocket->peerAddress().toString()
                               << ":" << tcpSocket->peerPort()
                               << " failed to join: incorrect magic number";
            return false;
        }
        quint32 ver;
        stream >> ver;
        if (ver != version)
        {
            qDebug().nospace() << "Client "
                               << tcpSocket->peerAddress().toString()
                               << ":" << tcpSocket->peerPort()
                               << " failed to join: incorrect version";
            stream << (quint8)0x00 << S_magic;
            stream << (qint8)(ver<version ? 0x01 : 0x02);
            return false;
        }
        stream << (quint8)0x00 << S_magic << (qint8)0x00;
    } else
    {
        qDebug().nospace() << "Client "
                           << tcpSocket->peerAddress().toString()
                           << ":" << tcpSocket->peerPort()
                           << " failed to join: no data on join";
        return false;
    }

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readPacket()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(clientLostConnection()));

    id = _id;
    p = new player(id, core);
    stream >> p->name;
    qDebug().nospace() << p->name << " ["
                       << tcpSocket->peerAddress().toString()
                       << ":" << tcpSocket->peerPort()
                       << "] joined the game";

    p->spawn(myRand(core->level.w*0.1, core->level.w*0.9), core->level.h*0.6);
    pressedW = pressedA = pressedS = pressedD = pressedL = pressedR = false;

    sendLevelData();

#ifdef SNS_NO_TCP_BUFFERING
    //qDebug() << tcpSocket->state();
    int on = 1;
//    qDebug() << "Thread set sock opt:" <<
                setsockopt(tcpSocket->socketDescriptor(),
                           IPPROTO_TCP, TCP_NODELAY,
                           (char *) &on, sizeof(int));
#endif

    ready = true;
    return true;
}

SnsThread::~SnsThread()
{
    if (core->threads.contains(this))
        core->threads.removeOne(this);
    /*try {
    if (isRunning())
    {
        qDebug() << "Thread is running. Terminating...";
        terminate();
        wait();
    }
    } catch (...)
    {
        cerr << "Exception caught\n";
    }*/
    //qDebug() << "Destroyed a thread";
}

bool SnsThread::operator==(SnsThread r)
{
    return id == r.id;
}

void SnsThread::readPacket()
{
//    cerr << "[" << core->ticks << "] got packet\n";
    QDataStream stream(tcpSocket);
    quint8 packetID;

    while (tcpSocket->bytesAvailable() > 0)
    {
        stream >> packetID;
        switch (packetID)
        {
        /*case 0x01:
        {
            qDebug().nospace() << "Client "
                               << tcpSocket->peerAddress().toString()
                               << ":" << tcpSocket->peerPort()
                               << " leaved the server";
            deleteLater();
            break;
        }*/
        case 0x03:
        {
            stream >> pressedW >> pressedA >> pressedS >> pressedD
                   /*>> pressedL >> pressedR >> xMouse >> yMouse*/;
            break;
        }
        case 0x04:
        {
            quint8 L, R;
            stream >> L >> R;
            if (L == 0x01)
                pressedL = true;
            else if (L == 0x00)
                pressedL = false;
            if (R == 0x01)
                pressedR = true;
            else if (R == 0x00)
                pressedR = false;
            qint16 slot;
            char* color;
            stream >> xMouse >> yMouse
                   >> slot >> color;
            p->currentSlot = slot;
            //p->currentWeapon = p->slot[slot];
            p->setColor(color);
            break;
        }
        case 0xff:
        {
            qint32 t;
            stream >> t;
            QDataStream socket(tcpSocket);
            socket << (quint8)0xff << t;
        }
        }
    }
}

void SnsThread::clientLostConnection()
{
    /*QDataStream stream(tcpSocket);
    stream.setVersion(QDataStream::Qt_4_8);
    stream << (quint8)0x01 << (qint8)0x01;*/

    if (!p->name.isEmpty())
        qDebug().nospace() << p->name << " ["
                           << tcpSocket->peerAddress().toString()
                           << ":" << tcpSocket->peerPort()
                           << "] lost connection";
    else
        qDebug().nospace() << "Client "
                           << tcpSocket->peerAddress().toString()
                           << ":" << tcpSocket->peerPort()
                           << " lost connection";
    deleteLater();
}

void SnsThread::process()
{
    if (pressedW)
        p->jump();
    if (pressedA)
        p->change_vx(-0.13);
    if (pressedS)
        p->change_vy(-0.1);
    if (pressedD)
        p->change_vx(0.13);

    if (pressedL)
        p->useFire(xMouse, yMouse);
    if (pressedR)
        p->useAltFire(xMouse, yMouse);

    p->tick();

//    qDebug() << "Waiting to write" << tcpSocket->bytesToWrite() << "bytes";
//    qDebug() << "Waiting to read" << tcpSocket->bytesAvailable() << "bytes";
//     == 0) {if (core->ticks%30

    //int s = core->updated.size();
//    qDebug() << "Tread is"<<(isRunning()?"":"not")<< "running";
    //    cerr << core->ticks << " data sent (id " << id << ")\n";
}

void SnsThread::sendData()
{
    if (ready)
    {
        sendPlayerData();
        sendProjectilesData();
    }
    sendUpdatedData();
}

void SnsThread::sendLevelData()
{
    qint32 mapSize = sizeofmap(&core->level);
    //qDebug() << "Uploading the map," << double(mapSize)/1024/1024 << "megabytes";
    QDataStream stream(tcpSocket);
    stream << (quint8)0x02
           << mapSize;
    stream << (quint8)0x03
           << core->level;
}

void SnsThread::sendUpdatedData()
{
    if (!core->updated.empty())
    {
        QDataStream stream(tcpSocket);
        stream << (quint8)0x04
               << (quint32)(sizeoftile()*core->updated.size());
//        qDebug() << "sending" << core->updated.size() << "tiles (" << (quint32)((2 + 2 + 12 /*sizeof(tile)*/)*core->updated.size()) << "bytes)";
        foreach (tilePoint t, core->updated)
        {
            stream << t.x << t.y
                   << core->level[t.x][t.y];
        }
    }
}

void SnsThread::sendPlayerData()
{
    int numberOfOtherPlayersAlive = 0;
    foreach (SnsThread *t, core->threads)
        if (t->getid() != id && t->p->alive())
            numberOfOtherPlayersAlive++;

    QDataStream stream(tcpSocket);
    stream << (quint8)0x05
           << *p << *p->slot[0] << *p->slot[1] << *p->slot[2]
           << (quint32)(sizeofplayer()*numberOfOtherPlayersAlive);
    foreach (SnsThread *t, core->threads)
        if (t->getid() != id && t->p->alive())
//        {
            stream << *t->p;
//            clog << "Thread " << id << ": sending player @ " << core->threads.at(i)->p->x() << "; " << core->threads.at(i)->p->y() << "\n";
//        }
}

void SnsThread::sendProjectilesData()
{
    QDataStream stream(tcpSocket);
    stream << (quint8)0x06
           << (quint32)(sizeofprojectile() * core->bullets.size());
//    if (core->bullets.size()) qDebug() << "sending" << core->bullets.size() << "projectiles (" << (quint32)(sizeof(double)*5 * core->bullets.size()) << "bytes)";
    foreach (projectile *t, core->bullets)
        stream << *t;
}

int SnsThread::getid(){return id;}
