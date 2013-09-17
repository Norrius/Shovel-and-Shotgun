#ifndef DECLARATIONS_H
#define DECLARATIONS_H

/****************************************************************
 * This file contains basic elements of the project.
 * Lots of these definitions are needed for each module.
 * This header should be included into every file
 * before any other includes and code
 ****************************************************************/

#include <QApplication>     // Qt applicaton
#include <QWidget>          // Qt widgets
#include <QtDebug>          // allowes using qDebug() as a stream
//#include <QFileDialog>      // load & save
//#include <QBitmap>
//#include <QTime>
//#include <QFile>
//#include <QTextStream>    // old file processing tools
//#include <QDataStream>
//#include <QUdpSocket>       // networking
//#include <fstream>          // file i/o
#include <iostream>         // console i/o
//#include <cstdlib>
//#include <cstring>
//#include <ctime>
//#include <unistd.h>
#include <math.h>           // calculations
//#include <QtCore/qmath.h>

//#define SNS_UI_ONLY
#define SNS_NO_TCP_BUFFERING
//#define RENDER_BACKGROUND
//#define DRAW_AABB
//#define LOG_EXCEPTION_TILE_IS_NOT_IN_RANGE

#ifdef SNS_NO_TCP_BUFFERING
#define IPPROTO_TCP 6
#define TCP_NODELAY 1
#if defined(Q_OS_UNIX)
#include <sys/types.h>  // for compatibility reasons
#include <sys/socket.h>
#elif defined(Q_OS_WIN)
#include <winsock2.h>
#endif
#endif

#define SNS_PREALPHA
const quint32 S_magic = 0x534E5338;
extern const quint32 version;       // SMmmppe (stage-major-minor-patch-extra)
extern const char *versionString;   // human-readable

typedef unsigned char byte;
const double pi = 3.141593;

#define fr(a,b) for (int a=0; a<b; a++)
#define forlist(i,list) for (int i=0; i<list.size(); i++)
#define foreachtile(level,i,j) for (int i=0; i<level.w; i++) for (int j=0; j<level.h; j++)
//#define foreachtile(i, j) for (int i=mlevel[0][0].x*16; i<mlevel[mlevel.m()-1][0].x*16+16; i++) for (int j=mlevel[0][0].y*16; j<mlevel[0][mlevel.n()-1].y*16+16; j++)
//#define c(a,b) a##b

using namespace std;
using namespace Qt;

const int tilesize=16;

class gameCore;
class gameUI;
class entity;
class player;
class weapon;
class projectile;

struct tile
{
    qint16 type;
    char color[8];
    qint16 durability;
//    int metadata;
    tile(qint16 type=0, const char *color="#ffffff", qint16 durability=0)
    {
        this->type = type;
        this->durability = durability;
        qstrcpy(this->color, color);
    }
};

struct tilePoint       // one does not simply use that class
{
    qint16 x;
    qint16 y;
    tilePoint(int _x=0, int _y=0){x=_x; y=_y;}
    bool operator==(tilePoint r){return (x==r.x && y==r.y);}
    bool operator<(tilePoint r)const{return y<r.y;}
};

struct renderfragment
{
    int x;
    int y;
    QPixmap data;
    renderfragment(int _x=0, int _y=0){x=_x; y=_y;data=QPixmap(256,256);data.fill(transparent);}
    bool operator==(renderfragment r){return (x==r.x && y==r.y);}
};

struct _map
{
    quint16 w;
    quint16 h;
    tile **p;
    _map(){w=h=a=0;}
    void allocate(){if(!a){a=true;p=new tile*[w];fr(i,w)p[i]=new tile[h];}}
    void clear(){if(a){fr(i,w)delete p[i];delete p;a=false;}}
    tile *operator[](int i){return p[i];}
private:
    bool a; // allocated
};

enum items
{
    itemShovel,
    itemBlock,
    itemSmg,
    itemShotgun,
    itemSniperRifle
};
const int globalItemsCount = 5;

/* Damage types:
 *
 * physical
 * projectile
 * shovel
 * fire
 * blast
 */

#endif // DECLARATIONS_H
