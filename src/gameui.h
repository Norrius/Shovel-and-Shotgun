#ifndef GAMEUI_H
#define GAMEUI_H

#include "declarations.h"
#include <QTcpSocket>
#include <QTime>
#include "player.h"
#include "projectile.h"
#include <QtGui/QMainWindow>

/*namespace Ui {
class gameUI;
}*/

class gameUI : public QMainWindow
{
    Q_OBJECT

    //Ui::gameUI *ui;

    QTcpSocket *tcpSocket;

    QPixmap textureTileMain;
    QPixmap textureTileBorder;
    QPixmap texturePlayer;           // player textures
    QPixmap textureBullet;

    int xPixmap, yPixmap;       // map layer LD corner coords [in-game]
    QList<renderfragment> lpixmap; // map layer
    QPixmap pixmap_player;
    QPixmap pixmap_projectiles;
    QPixmap pixmap_hover;       // hightlighting layer
    QPixmap pixmap_hud;         // interface layer
    static const int hudHeight = 48;   // height of interface bar [widget]
//    QPixmap pixmap_background;
    bool overlay;               // display info?

    bool pressedW, pressedA, pressedS, pressedD; // it appears to be the best way to store pressed buttons...
    bool pressedWPrev, pressedAPrev, pressedSPrev, pressedDPrev; // last tick state
    double xMouse, yMouse;          // mouse pointer [in-game]
    double xMousePrev, yMousePrev;  // last tick state [in-game]
    double xMouseReal, yMouseReal;  // mouse pointer [widget]
    double xCorner, yCorner;        // screen corner [in-game]

    double w, h;                // how many tiles wide and high the screen is
//    QString mapName;

    // flags for data reviever
    bool waitingForMoreData;

    bool gameGoes;
    unsigned int ticks;
    QTime globalTime;
    int timerID;
    double fps;
    double ping; // used for ping

    QString playerName;
    QString serverAddress;

    _map levelCashe;
    QVector<tilePoint> rerender;
    player playerCashe;
    QList<player> players;
    QList<projectile> bulletsCashe;

    void startGame();
    void stopGame();

    void timerEvent(QTimerEvent *);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent (QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);

    void regulateRenderFragmets();
    void renderFragment(renderfragment *pointer);
    void renderTile(int x, int y, QPixmap *pixmap);
    void renderTileAbsolute(int x, int y);
    int tileConnectionType(int x, int y);

    void paintPlayer(player p);
    void paintBullet(projectile bullet);
    void paintHover();
    void paintSky();
    void paintHud();

    void calcCorner(player p);

    void menuVisible(bool b);

private slots:
    void readPacket();

    void joinServer();

    void displayError(QAbstractSocket::SocketError error);

public:
    explicit gameUI(QWidget *parent = 0);
    ~gameUI();

};

#endif // GAMEUI_H
