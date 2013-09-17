#ifndef EDITOR_H
#define EDITOR_H

#include <QtGui/QMainWindow>
#include <iostream>
#include <math.h>

using namespace std;
using namespace Qt;

const int tilesize=16;

#define fr(a,b) for (int a=0; a<b; a++)

struct tile
{
    qint16 type;
    char color[8];
    qint16 durability;
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

namespace Ui {
    class editor;
}

class editor : public QMainWindow
{
    Q_OBJECT

    Ui::editor *ui;

    QString mapFilename;

    _map level;

    int mode;
    char selectedColor[8];  // "#rrggbb\0"
    int selectedDurability;

    bool pressedL;
    bool pressedR;
    double xMouseReal;
    double yMouseReal;
    double xCorner;
    double yCorner;
    bool levelLoaded;

    QPixmap *pixmap;

    QPixmap sprite_tile;
    QPixmap sprite_border;

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void listenToMouse();
    void render();
    void renderTile(int x, int y);
    void updateTile(int x, int y);

    void setTile(int x, int y, int type, char color[]="#ffffff", double durability=70);
    int tileConnectionType(int x, int y);
    bool tileSolid(int x, int y);
    qint16 &tileType(int x, int y);
    char *tileColor(int x, int y);

    void readLevel(QString _file);
    void writeLevel(QString _file);

public slots:
    void actNew();
    void actOpen();
    void actSave();
    void actSaveAs();
    void actColor();
    void actSmoother();

    void actTileAir();
    void actTileNormal();
    void actTileAdamantine();

public:
    editor(QWidget *parent = 0);
    ~editor();
};

QDataStream &operator<<(QDataStream & s, const char &c);
QDataStream &operator>>(QDataStream & s, char &c);

QDataStream &operator<<(QDataStream & s, const tile &t);
QDataStream &operator>>(QDataStream & s, tile &t);

QDataStream &operator<<(QDataStream & s, _map &m);
QDataStream &operator>>(QDataStream & s, _map &m);

#endif // EDITOR_H
