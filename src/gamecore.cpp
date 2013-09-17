#include "declarations.h"
#include "gamecore.h"
#include "global.h"
#include "tile.h"
#include "player.h"
#include "projectile.h"
#include <QTcpSocket>       // networking
#include "io.h"             // custom QDataStream
#include <QFile>            // file manipulation
#include <QTimer>           // for main timer
#include <QTime>

gameCore::gameCore(QObject *parent) :
    QTcpServer(parent)
{
//    qDebug() << "Created SnS server";
    clog << "Started SnS server\nversion " << versionString << "\n";

    /*
    // ~~~ generating flat map ~~~
    QFile f("maps/flat2.snsmap");
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    _map tmp;
    tmp.w=1024;
    tmp.h=128;
    tmp.allocate();
    for (int i=0; i<1024; i++)
        for (int j=0; j<128; j++)
            if (j<64) {
                tmp[i][j].type = 1;
                qstrcpy(tmp[i][j].color, "#cccccc");
                tmp[i][j].durability = 70;
            } else if (j==64) {
                tmp[i][j].type = 1;
                qstrcpy(tmp[i][j].color, "#00a000");
                tmp[i][j].durability = 50;
            } else {
                tmp[i][j].type = 0;
                qstrcpy(tmp[i][j].color, "#ffffff");
                tmp[i][j].durability = 0;
            }
    out << tmp;*/

    // ~~~ loading the map ~~~
    mapName = "fall";
    QFile file("maps/"+mapName+".snsmap");
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    stream >> level;
    file.close();

    for (int i=0; i<level.w; i++)
        for (int j=0; j<level.h; j++)
            maybeFalling.append(tilePoint(i, j));
    while (!maybeFalling.empty())
        foreach (tilePoint t, maybeFalling)
            checkForFall(t.x, t.y);
    clog << "Map loaded\n";

    // ~~~ startng the server ~~~
    if (!this->listen(QHostAddress::Any, 32265)) {
        cerr << "Unable to start listening on TCP port\n";
        exit(1);
    } else
        cerr << "Listening on TCP port " << serverPort() << "\n";

    // ~~~ initializing some variables ~~~
    nextID = 0;
    ticks = 0;

    // ~~~ setting up the main game timer ~~~
    QTimer *timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
    timer->start(33);
}

gameCore::~gameCore()
{
    qDebug() << "Destroyed SnS server";
}

void gameCore::incomingConnection(int socketDescriptor)
{ // called then new player connects
    SnsThread *thread = new SnsThread(this);
    if (thread->grabConnection(socketDescriptor, nextID++))
    {
//        cout << "Client joined\n";
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        threads.append(thread);
//        threads.last()->start();
    } else
    {
        delete thread;
    }
//    cerr << "Thread count: " << threads.size() << "\n";
}

void gameCore::updateTile(int x, int y)
{
    maybeFalling.append(tilePoint(x, y));
    maybeFalling.append(tilePoint(x-1, y));
    maybeFalling.append(tilePoint(x+1, y));
    maybeFalling.append(tilePoint(x, y-1));
    maybeFalling.append(tilePoint(x, y+1));
    updated.append(tilePoint(x, y));
}

void gameCore::checkForFall(int x, int y)
{
    QList<tilePoint> cluster;
    try
    {
        assignCluster(x, y, &cluster);
    } catch (const char *s)
    {
        if (!qstrcmp(s, "NoFall")) // this tile will not fall
        {
            maybeFalling.removeAll(tilePoint(x, y));
            return;
        } else if (!qstrcmp(s, "Exception: tile does not exist.")) // this tile simply does not exist
        {
            maybeFalling.removeAll(tilePoint(x, y));
            return;
        } else
        {
            Q_ASSERT_X(qstrcmp(s, "NoFall"), "exception", "trowed unknown exception");
            throw s;
        }
    }

    if (!cluster.empty())
    {
        qSort(cluster.begin(), cluster.end());

        foreach(tilePoint t, cluster)
        { // need improvement here
            tile newTile; // null (air) tile by default
            int deltaDurability = tileDurability(level, t.x, t.y) * myRand(0.1, 0.2);
            cap(deltaDurability, 2, 6);
            qint16 newDurability = tileDurability(level, t.x, t.y) - deltaDurability;
            if (newDurability > 0)
                newTile = tile(tileType(level, t.x, t.y), tileColor(level, t.x, t.y), newDurability);
            try {
            setTile(level, t.x, t.y-1, newTile);
            setTile(level, t.x, t.y, tile());   // empty (air) tile
            updated.append(tilePoint(t.x, t.y));
            updated.append(tilePoint(t.x, t.y-1));
            maybeFalling.append(tilePoint(t.x, t.y-1));
            maybeFalling.append(tilePoint(t.x, t.y));
            alreadyFallen.append(tilePoint(t.x, t.y-1));
            alreadyFallen.append(tilePoint(t.x, t.y));
            } catch (...) {}
        }

    } else
    {
        maybeFalling.removeAll(tilePoint(x, y));
    }
}

void gameCore::assignCluster(int x, int y, QList<tilePoint> *cluster)
{
    //cerr << "Checking point " << x << "; "<< y<<" for being cluster element\n";
    if (tileType(level, x, y) == 2 || (tileSolid(level, x, y) && y == 0))
        throw "NoFall";
    else if (tileType(level, x, y) == 1)
    {
        if (!cluster->contains(tilePoint(x, y)))
        {
            cluster->append(tilePoint(x, y));
            if (!cluster->contains(tilePoint(x, y-1)))
                assignCluster(x, y-1, cluster);
            if (!cluster->contains(tilePoint(x-1, y)))
                assignCluster(x-1, y, cluster);
            if (!cluster->contains(tilePoint(x+1, y)))
                assignCluster(x+1, y, cluster);
            if (!cluster->contains(tilePoint(x, y+1)))
                assignCluster(x, y+1, cluster);
        }
    }
}

void gameCore::createBullet(int ownerID, double _x, double _y, double _angle, double _energy)
{
    bullets.append(new projectile(this, ownerID, _x, _y, _angle, _energy));
}

void gameCore::tick() {
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /*
    cerr << "~~~~~~~~\n";
    //cerr << floor(p->x/16) << "\n";
    //cerr << floor(p->y/16) << "\n";
    for (int i=0; i<level_old.size(); i++)
        cerr << level_old.at(i).x << "; " << level_old.at(i).y << "\n";
    */
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    ticks++;

    // ~~~ processing bullets ~~~
    for (int i=0; i<bullets.size(); i++) {
        bullets[i]->tick();
        if (bullets[i]->remove)
            bullets.removeAt(i);
    }

    // ~~~ processing players ~~~
    /*for (int i=0; i<threads.size(); i++)
    {
        threads[i]->tick();*/

        /*QByteArray data;
        data += 0x01;
        data += players[i].serialize();
        connections[i]->write(data);*/
     // threads[i]->sendPlayerData();
//        threads[i]->sendLevelData();
        /*QString data = QString("PC:")+QString::number(players[i].x())+
        QString(";")+QString::number(players[i].y())+QString("\0");
        connections[i]->write(data.toAscii());*/
//        player tmp = players[i];
//        char buff[sizeof(player)];
//        memcpy(buff, &tmp, sizeof(player));
//        cerr << "server encodes " << sizeof(tmp) << " bytes of data\n";
//        connections[i]->write(buff);
        /*player cont;
        std::string file = "binary_arch.dump";
        std::_Ios_Openmode flags = std::ios::binary;
        std::ofstream ofs(file.c_str(), std::ios::out|flags);
        boost::archive::binary_oarchive oa(ofs);
        oa << boost::serialization::make_nvp("Test_Object", cont);*/
     // threads[i]->sendUpdatedData();
    //}

    foreach (SnsThread* t, threads)
        t->process();

    alreadyFallen.clear();
    foreach (tilePoint t, maybeFalling)
        if (!alreadyFallen.contains(tilePoint(t.x, t.y)))
            checkForFall(t.x, t.y);

    foreach (SnsThread* t, threads)
        t->sendData();

//    cerr << ticks << " clearing\n";
    updated.clear();

 // controls


 // calculating liquids...
    /*processWater();
    if (ticks%2) processLava();*/

 // level manipulation
    //regulateMatrix();
   // regulateLevel();

    /*chunkList.clear();
    for (int i=0; i<level_old.size(); i++) {
        pointerchunk tmp(level_old.at(i).x, level_old.at(i).y, &level_old[i]);
        chunkList.append(tmp);
    }*/

 // calculating lights
    //calculateLightMap();

 // updating some tiles
//    connectedUI->renderUpdated(&updated);

 // player's tick...
    /*for (int i=0; i<players.size(); i++)
        players[i].tick();*/

    //updated.clear();

    /*/ fps counting...
    static double fps=0;
    static QTime time;
    static int frameCnt=0;
    static double timeElapsed=0;
    frameCnt++;
    timeElapsed += time.elapsed();
    time.restart();
    if (timeElapsed >= 500)
    {
        fps = frameCnt * 1000.0 / timeElapsed;
        timeElapsed = 0;
        frameCnt = 0;
        cerr << "Framerate: " << fps << "\n";
    }*/
}
