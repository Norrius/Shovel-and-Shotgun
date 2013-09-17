#ifndef GAMECORE_H
#define GAMECORE_H

#include <QTcpServer>
#include <QMutex>
#include "declarations.h"
#include "thread.h"

class gameCore : public QTcpServer
{
    Q_OBJECT

//    gameUI *connectedUI;
    //    QList<QSet<char> > pressedKeys;
    int nextID;     // first unused user ID, only increase

    QTimer *timer;

    QString mapName;

    void incomingConnection(int socketDescriptor);  // overloaded QTcpServer method

private slots:
    void tick();

public:
    QList<SnsThread*> threads;
    QList<projectile*> bullets;

    _map level;
    QList<tilePoint> updated;
    QList<tilePoint> maybeFalling; // contains all tiles which maybe should fall this tick
    QList<tilePoint> alreadyFallen;// tiles which already fallen this tick

    unsigned int ticks;

    explicit gameCore(QObject *parent = 0);
    ~gameCore();

    void updateTile(int x, int y);

    void checkForFall(int x, int y);
    void assignCluster(int x, int y, QList<tilePoint> *cluster);

    void createBullet(int ownerID, double _x, double _y, double _angle, double _energy);

};

#endif // GAMECORE_H
