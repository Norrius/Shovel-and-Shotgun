#include "declarations.h"
#include "gameui.h"
//#include "ui_gamewindow.h"
#include "global.h"
#include "tile.h"
#include "gamecore.h"
#include "player.h"
#include "projectile.h"
#include "io.h"             // custom QDataStream
#include <QDesktopWidget>   // for gathering size of desktop
#include <QPainter>         // Qt painting system
#include <QInputEvent>      // event handler
#include <QTimer>           // render timer
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>

gameUI::gameUI(QWidget *parent) :
    QMainWindow(parent)/*,
    ui(new Ui::gameUI)*/
{
    qDebug() << "Created SnS client";
    gameGoes = false;

    setFixedSize(992, 544);
    QDesktopWidget *d = QApplication::desktop();
    move(d->width()/2-width()/2, d->height()/2-height()/2);

    //ui->setupUi(this);
    playerName = "Player";
    serverAddress = "127.0.0.1";
    QFile options("options");
    if (options.exists() && options.open(QFile::ReadOnly))
    {
        QDataStream stream(&options);
        stream >> playerName;
        stream >> serverAddress;
    }
    /*ui->leditName->setText(playerName);
    ui->leditAddress->setText(serverAddress);
    connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(joinServer()));*/

    tcpSocket = new QTcpSocket(this);
//    w = new gamewindow(&lpixmap);
//    setFixedSize(992, 544);
//    setWindowState(WindowMaximized);
//    resize(992, 544);
    /*cerr << "Width: " << geometry().width() << " px = " << w << " tiles\n"
         << "Height: " << geometry().height() << " px = " << h << " tiles\n";*/
    /*QDesktopWidget *d = QApplication::desktop();            // move window
    move(d->width()/2-width()/2, d->height()/2-height()/2);*/ // to center
//    QDesktopWidget *d = QApplication::desktop();
//    w = d->width()/double(tilesize);
//    h = d->height()/double(tilesize);

    /*QLineEdit txtServerAddress;
    txtServerAddress.move(width()/2-txtServerAddress.width()/2, height()/2-txtServerAddress.height()/2);
    txtServerAddress.resize(10,20);
    txtServerAddress.show();*/

    //setWindowOpacity(0.7);// lol

//    setupGame();

    //centralWidget()->setAttribute(Qt::WA_TransparentForMouseEvents);
    //setMouseTracking(1);    // make system send mouse events even if no button are pressed

    show();
}

gameUI::~gameUI()
{
    //delete ui;
    QFile options("options");
    if (options.open(QFile::ReadWrite))
    {
        QDataStream stream(&options);
        stream << playerName;
        stream << serverAddress;
    }
    qDebug() << "Destroyed SnS client";
}

void gameUI::joinServer()
{    
    /*playerName = ui->leditName->text();
    serverAddress = ui->leditAddress->text();*/

    // ~~~ connecting to the server ~~~
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    waitingForMoreData = false;
    QDataStream stream(tcpSocket);
    tcpSocket->connectToHost(serverAddress, 32265);

    if (!tcpSocket->waitForConnected(10000))
    {
        QMessageBox::critical(this, "Connection failed", "Connection failed: can't establish connection\n");
        return;
    }
    stream << (quint8)0x00 << S_magic << version << playerName;
    if (tcpSocket->waitForReadyRead(10000))
    {
        quint8 packetID;
        stream >> packetID;
        if (packetID != 0x00)
        {
            qDebug() << "Last error:" << tcpSocket->error() << "-" << tcpSocket->errorString();
            tcpSocket->disconnectFromHost();
            QMessageBox::critical(this, "Connection error", "Connection failed: unknown packet\n");
            return;
        }
        quint32 magic;
        stream >> magic;
        if (magic != S_magic)
        {
            tcpSocket->disconnectFromHost();
            QMessageBox::critical(this, "Connection error", "Connection failed: wrong magic\n");
            return;
        }
        qint8 j;
        stream >> j;
        if (j != 0x00)
        {
            switch(j)
            {
            case 0x01:
                tcpSocket->disconnectFromHost();
                QMessageBox::critical(this, "Connection error", "Connection refused: outdated client\n");
                return;
            case 0x02:
                tcpSocket->disconnectFromHost();
                QMessageBox::critical(this, "Connection error", "Connection refused: outdated server\n");
                return;
            case 0x03:
                tcpSocket->disconnectFromHost();
                QMessageBox::critical(this, "Connection error", "Connection refused: server is full\n");
                return;
            case 0x04:
                tcpSocket->disconnectFromHost();
                QMessageBox::critical(this, "Connection error", "Connection refused: you are banned on this server\n");
                return;
            default:
                tcpSocket->disconnectFromHost();
                QMessageBox::critical(this, "Connection error", "Connection refused: unknown reason\n");
                return;
            }
        }
    } else
    {
        tcpSocket->disconnectFromHost();
        QMessageBox::critical(this, "Connection error", "Connection failed: connection timed out\n"); // unknown and mysterious error
        return;
    }

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readPacket()));

    // ~~~ drawing system initialization ~~~
    textureTileMain = QPixmap();
    textureTileMain.load(":/resources/tile.png");
    textureTileBorder = QPixmap();
    textureTileBorder.load(":/resources/border.png");
    texturePlayer = QPixmap();
    texturePlayer.load(":/resources/player.png");
    textureBullet = QPixmap();
    textureBullet.load(":/resources/bullet.png");

#ifdef RENDER_BACKGROUND
    pixmap_background = QPixmap(width(), height());
    pixmap_background.fill();
#endif

    pixmap_player = QPixmap(width(), height());
    pixmap_player.fill(transparent);

    pixmap_projectiles = QPixmap(width(), height());
    pixmap_projectiles.fill(transparent);

    pixmap_hover = QPixmap(width(), height());
    pixmap_hover.fill(transparent);

    pixmap_hud = QPixmap(width(), hudHeight);
    pixmap_hud.fill(white);

    xCorner = yCorner = 0;
    overlay = false;
    ticks = 0;
    fps = 0;
    ping = 0;
    pressedW = pressedA = pressedS = pressedD = false;
    pressedWPrev = pressedAPrev = pressedSPrev = pressedDPrev = false;
    w = width()/tilesize;
    h = height()/tilesize;
}

void gameUI::startGame()
{ // called after level loading

#ifdef SNS_NO_TCP_BUFFERING
    // ~~~ making socket unbuffered ~~~
    //qDebug() << tcpSocket->state();
    int on = 1;
//    qDebug() << "Client set sock opt:" <<
                setsockopt(tcpSocket->socketDescriptor(),
                           IPPROTO_TCP, TCP_NODELAY,
                           (char *) &on, sizeof(int));
#endif

    // ~~~ starting game ~~~
    gameGoes = true;
    menuVisible(false);
    centralWidget()->setAttribute(Qt::WA_TransparentForMouseEvents);
    setMouseTracking(1);    // make system send mouse events even if no button are pressed
    globalTime.start();
    timerID = startTimer(20);
    /*QTimer *timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(renderTick()));
    timer->start(20);*/
}

void gameUI::stopGame()
{
    if (gameGoes)
    {
        killTimer(timerID);
        gameGoes = false;

        disconnect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readPacket()));
        tcpSocket->disconnectFromHost();

        levelCashe.clear();
        lpixmap.clear();
        rerender.clear();
        players.clear();
        bulletsCashe.clear();

        centralWidget()->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        menuVisible(true);
        update();
    }
}

void gameUI::readPacket()
{
//    qDebug() << "Got packet!";
    QDataStream stream(tcpSocket);
    static quint8 packetID=0;
    static quint32 mapSize=0;
    quint32 dataSize;

    do
    {
        //cerr << ticks << " - trying to got data\n";
        if (!waitingForMoreData) // if client is waiting for more data, first byte should not be readed again
            stream >> packetID;

        switch (packetID)
        {
        case 0x02:
        { // server info: map size and name, gamemode, etc.
            stream >> mapSize;
            cerr << "Prepairing to download map: " << mapSize << " bytes\n";
            break;
        }
        case 0x03:
        { // map
          // this additional verification is needed because map can't be sent in single packet
            cerr << "Downloading the map, "<<tcpSocket->bytesAvailable()*100/double(mapSize)<<"%\n";
            if (tcpSocket->bytesAvailable() >= mapSize) {
                waitingForMoreData = false;
                stream >> levelCashe;
                startGame();
            } else {
                waitingForMoreData = true;
                //cerr << "~!~" << tcpSocket->bytesAvailable() << "~!~\n";
            }
            break;
        }
        case 0x04:
        { // updated tiles
            stream >> dataSize;
            //qDebug() << "recieving" << dataSize/sizeoftile() << "tiles (" << dataSize << "bytes)";
            for (uint i=0; i<dataSize; i+=sizeoftile())
            {
                qint16 x, y;
                tile tmp1;
                stream >> x >> y >> tmp1;
                try {
                    setTile(levelCashe, x, y, tmp1);
                } catch (...) {}
//                cerr << "Got updated tile\n";
                tilePoint tmp2(x,y);
                rerender.append(tmp2);
            }
            /*stream >> t;
            while (t != 0xff) {
                stream >> x >> y >> tmp1;
                try {
                    setTile(levelCashe, x, y, tmp1);
                }
                catch (const char *ex) {
                    //cerr << "[" << ticks << "] " << ex << "\n";
                    continue;
                }
                catch (...) {
                    cerr << "[" << ticks << "] Unhandled exception!\n"; continue;
                }
                cerr << "Got updated tile\n";
                tilePoint tmp2(x,y);
                rerender.append(tmp2);
                stream >> t;
            }*/
            break;
        }
        case 0x05:
        { // player
            players.clear();
            stream >> playerCashe
                   >> *playerCashe.slot[0] >> *playerCashe.slot[1] >> *playerCashe.slot[2];
            stream >> dataSize;
            for (uint i=0; i<dataSize; i+=sizeofplayer())
            {
                player tmp;
                stream >> tmp;
//                    clog << "Recieved player @ "<<tmp.x() <<"; " << tmp.y() << "\n";
                players.append(tmp);
            }

            /*while (t != 0xff)
            {
                if (t == 0x01)  // me
                    stream >> playerCashe;
                else            // other player
                {
                    player tmp;
                    stream >> tmp;
//                    clog << "Recieved player @ "<<tmp.x() <<"; " << tmp.y() << "\n";
                    players.append(tmp);
                }
                stream >> t;
            }*/
//            clog << "Have " << players.size() << " player(s) in cashe vector:\n";
            /*foreach(player p, players)
                clog << "Player @ "<<p.x()<<"; "<<p.y()<<"\n";*/
//            cerr << "Got packet from server, ID 05\n";
            break;
        }
        case 0x06:
        { // projectiles
            bulletsCashe.clear();
            stream >> dataSize;
//            if (dataSize) qDebug() << "recieving" << dataSize/(sizeof(double)*5) << "projectiles (" << dataSize << "bytes)";

            for (uint i=0; i<dataSize; i+=sizeofprojectile())
            {
                projectile tmp;
                stream >> tmp;
                bulletsCashe.append(tmp);
            }

            /*while (t != 0xff) {
                projectile tmp;
                stream >> tmp;
                bulletsCashe.append(tmp);
                stream >> t;
            }*/
            break;
        }
        case 0xff:
        {
            qint32 t;
            stream >> t;
            ping = globalTime.elapsed() - t;
            //cerr << "Got ping " << t << " back at " << ticks << " ("<<ticks-t<<" ticks delay)\n";
            break;
        }
        default:
            qDebug() << "Got unknown packet! " << packetID; break;
        } // switch
    } while (!waitingForMoreData && tcpSocket->bytesAvailable() > 0);
}

void gameUI::displayError(QAbstractSocket::SocketError error)
{
    qDebug() << "Error:" << error << "-" << tcpSocket->errorString();
    QMessageBox::critical(this, "Connection error", "Connection error: "+tcpSocket->errorString());
    stopGame();
}

void gameUI::menuVisible(bool b)
{
    /*ui->lblName->setVisible(b);
    ui->lblAddress->setVisible(b);
    ui->leditName->setVisible(b);
    ui->leditAddress->setVisible(b);
    ui->btnConnect->setVisible(b);
    if (!b)
        this->setFocus();*/
}

// ---------- Events ----------

void gameUI::paintEvent(QPaintEvent *)
{ // this event occurs automatically or then update() is called
    if (gameGoes) {
        QPainter painter(this);
//        painter.translate(0, height());
//        painter.scale(1, -1);
    //    painter.setPen(white);
#ifdef RENDER_BACKGROUND
        painter.drawPixmap(0, 0, pixmap_background);
#endif
        /*painter.drawPixmap((xPixmap-xCorner)*tilesize,
                           height() - (pixmap->height() - (yPixmap+yCorner)*tilesize), *pixmap);*/
        forlist(i,lpixmap)
        {
            double xPixmap = lpixmap.at(i).x*16;
            double yPixmap = lpixmap.at(i).y*16;
            //cerr << "[RENDER] Rendered map fragment at "<<xPixmapNew<< "; "<<yPixmapNew<<"\n";
            painter.drawPixmap((xPixmap-xCorner)*tilesize,
                               height() - ((yPixmap-yCorner+16)*tilesize),
                               lpixmap.at(i).data);
        }

        painter.drawPixmap(0, 0, pixmap_player);
        painter.drawPixmap(0, 0, pixmap_projectiles);
        painter.drawPixmap(0, 0, pixmap_hover);
        //painter.drawPixmap(0, height()-hudHeight, pixmap_hud);

//        painter.scale(1, -1);
//        painter.translate(0, -height());

        if (overlay) { // info
            try {
                painter.drawText(2, 14, QString::number(ticks));
                painter.drawText(2, 28, QString::number((int)fps) + " FPS");
                painter.drawText(2, 42, QString::number(ping) + " ms");
                painter.drawText(2, 56, tcpSocket->peerAddress().toString());
                painter.drawText(2, 70, versionString);

                painter.drawText(2, 98, "x: " + QString::number(playerCashe.x()));
                painter.drawText(2, 112, "y: " + QString::number(playerCashe.y()));
//                painter.drawText(2, 84, "f: " + QString::number(playerCashe.getf()));
                painter.drawText(2, 126, "vx: " + QString::number(playerCashe.vx()));
                painter.drawText(2, 140, "vy: " + QString::number(playerCashe.vy()));

                painter.drawText(2, 154, "xc: " + QString::number(xCorner));
                painter.drawText(2, 168, "yc: " + QString::number(yCorner));
                painter.drawText(2, 182, "xm: " + QString::number(xMouse));
                painter.drawText(2, 196, "ym: " + QString::number(yMouse));
                //painter.drawText(2, 158, QString::number(connectedCore->directSunlight(floor(xMouse), floor(yMouse))));
                //painter.drawText(2, 96, QString::number(gameTime.elapsed()/ticks));
                //painter.drawText(2, 210, QString::number(playerCashe.currentSlot));
                //painter.drawText(2, 224, QString::number(playerCashe.selectedWeapon()));
                /*painter.drawText(2, 110, QString::number(p.checkSpaceUp(3)));
                painter.drawText(2, 122, QString::number(p.checkSpaceRight(3)));*/
                //painter.drawText(2, 134, QString::number(tileType_old(p.x(), p.y())));
                //painter.drawText(2, 146, QString::number(p.checkSpaceHor(-3)));
                /*painter.drawText(w*tilesize-35, 50,
                                 QString::number(level_old[int(xMouse/tilesize)][h-int(yMouse/tilesize)-1][0]) + ":" +
                                 QString::number(level_old[int(xMouse/tilesize)][h-int(yMouse/tilesize)-1][1]));*/
                painter.drawText(width()-100, 50,
                                 QString::number(tileType(levelCashe, floor(xMouse), floor(yMouse))) + ":"
                                 + QString::fromUtf8(tileColor(levelCashe, floor(xMouse), floor(yMouse))) + ":"
                                 + QString::number(tileDurability(levelCashe, floor(xMouse), floor(yMouse))));
            }
            catch (const char *ex) {
                //cerr << "[" << ticks << "] " << ex << "\n";
                return;
            }
            catch (...) {
                cerr << "[" << ticks << "] Unhandled exception!\n"; return;
            }
        } // if (overlay)
    }
}

void gameUI::keyPressEvent(QKeyEvent *event)
{
    if (gameGoes) {
        switch (event->key()) { // controls
        case Key_Space:
        case Key_W:
            pressedW = true; break;
        case Key_A:
            pressedA = true; break;
        case Key_S:
            pressedS = true; break;
        case Key_D:
            pressedD = true; break;
        case Key_F:
            /*toggle(connectedCore->p->flying);*/ break;
        case Key_R:
            /*forceRedraw();*/ break;
        case Key_F1:
            toggle(overlay); break;
        /*case Key_Q:
            playerCashe.chooseSlot(-1); break;
        case Key_E:
            playerCashe.chooseSlot(1); break;*/
        case Key_1:
            playerCashe.currentSlot = 0; break;
        case Key_2:
            playerCashe.currentSlot = 1; break;
        case Key_3:
            playerCashe.currentSlot = 2; break;
        /*case Key_4:
            playerCashe.chooseSlot(itemShotgun, true); break;
        case Key_5:
            playerCashe.chooseSlot(itemSniperRifle, true); break;*/
        case Key_X:
            forlist(i,lpixmap)
                lpixmap.at(i).data.save("renderfragments/"+QString::number(lpixmap.at(i).x)+"."+QString::number(lpixmap.at(i).x)+".png");
                cerr << "SAVED\n";
            break;
        case Key_Z:
            break;
        case Key_P:
            break;
        case Key_Escape:
            stopGame();
        }
    }
}

void gameUI::keyReleaseEvent (QKeyEvent *event)
{
    if (gameGoes) {
        switch (event->key()) {
        case Key_Space:
        case Key_W:
            pressedW = false; break;
        case Key_A:
            pressedA = false; break;
        case Key_S:
            pressedS = false; break;
        case Key_D:
            pressedD = false; break;
        }
    }
}

void gameUI::mouseMoveEvent(QMouseEvent *event)
{
    xMouseReal = event->x();
    yMouseReal = event->y();
    if (gameGoes) {
        xMouse = xMouseReal/double(tilesize) + xCorner;
        yMouse = (height()-yMouseReal) / double(tilesize) + yCorner;

        QDataStream stream(tcpSocket);
        stream << (quint8)0x04
               << (quint8)0xff << (quint8)0xff
               << xMouse << yMouse
               << playerCashe.currentSlot
               << playerCashe.getSelectedColor();
    }
}

void gameUI::mousePressEvent(QMouseEvent *event)
{
    if (gameGoes) {
        if (event->button() == MiddleButton)
        {
            playerCashe.setColor(tileColor(levelCashe, xMouse, yMouse));
        }
        else
        {
            QDataStream stream(tcpSocket);
            stream << (quint8)0x04
                   << ((event->button() == LeftButton) ? (quint8)0x01 : (quint8)0xff)
                   << ((event->button() == RightButton) ? (quint8)0x01 : (quint8)0xff)
                   << xMouse << yMouse
                   << playerCashe.currentSlot
                   << playerCashe.getSelectedColor();
        }
    }
}

void gameUI::mouseReleaseEvent(QMouseEvent *event)
{
    if (gameGoes) {
        if (event->button() == LeftButton || event->button() == RightButton)
        {
            QDataStream stream(tcpSocket);
            stream << (quint8)0x04
                   << ((event->button() == LeftButton) ? (quint8)0x00 : (quint8)0xff)
                   << ((event->button() == RightButton) ? (quint8)0x00 : (quint8)0xff)
                   << xMouse << yMouse
                   << playerCashe.currentSlot
                   << playerCashe.getSelectedColor();
        }
    }
}

void gameUI::wheelEvent(QWheelEvent *event)
{
    if (gameGoes) {
        int numSteps = event->delta() / 120;    // number of wheel steps, see Qt Assistant
        playerCashe.currentSlot += numSteps;
        if (playerCashe.currentSlot > 3)
            playerCashe.currentSlot -= 3;
        else if(playerCashe.currentSlot < 1)
            playerCashe.currentSlot += 3;
    }
}

void gameUI::closeEvent(QCloseEvent *)
{
    /*QDataStream stream(tcpSocket);
    stream << (quint8)0x01;
    tcpSocket->disconnectFromHost();*/
}

void gameUI::resizeEvent(QResizeEvent *)
{
    w = width()/double(tilesize);
    h = height()/double(tilesize);
}

// ---------- Display functions ----------

void gameUI::renderFragment(renderfragment *pointer)
{
    for (int i=0; i<16; i++)
        for (int j=0; j<16; j++)
            renderTile(pointer->x*16+i, pointer->y*16+j, &pointer->data);
}

void gameUI::renderTile(int x, int y, QPixmap *pixmap)
{
    int type;
    QColor color;
    int connection;
    try {
        type = tileType(levelCashe, x, y);
        color = QColor(tileColor(levelCashe, x, y));
        connection = tileConnectionType(x, y);
    }catch (const char *ex) {
        //cerr << "[" << ticks << "] " << ex << "\n";
        return;
    }
    catch (...) {
        cerr << "[" << ticks << "] Unhandled exception!\n";
        return;
    }

    //cerr << "Rendering tile ("<<x<<"; "<<y<<") as type="<<type<<"\n";

    double xPixmap = floor(x/16.0)*16; // LD corner
    double yPixmap = floor(y/16.0)*16; // of pixmap

    double xDisplay = (x-xPixmap)*tilesize;
    double yDisplay = pixmap->height()-(y-yPixmap+1)*tilesize;

    QPainter painter(pixmap);
//    painter.translate(0, pixmap->height());
//    painter.scale(1, -1);

    // tile texture
    if (type == 0) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
                                                          // powered by Source, yeah
        painter.setBrush(transparent);
        painter.setPen(NoPen);
        painter.drawRect(xDisplay, yDisplay, tilesize, tilesize);
    } else if (type < 3) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setBrush(transparent);

        painter.drawPixmap(xDisplay, yDisplay, tilesize, tilesize,
                           textureTileMain, 0, connection*tilesize, tilesize, tilesize);

        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.setBrush(color);
        painter.setPen(NoPen);
        painter.drawRect(xDisplay, yDisplay, tilesize, tilesize);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawPixmap(xDisplay, yDisplay, tilesize, tilesize,
                           textureTileBorder, 0, connection*tilesize, tilesize, tilesize);

    }
}

int gameUI::tileConnectionType(int x, int y)
{
    int type=0;
    if (tileSolid(levelCashe, x-1, y)) type+=1;
    if (tileSolid(levelCashe, x+1, y)) type+=2;
    if (tileSolid(levelCashe, x, y-1)) type+=4;
    if (tileSolid(levelCashe, x, y+1)) type+=8;
    return type;
}

void gameUI::renderTileAbsolute(int x, int y)
{
    int _x = floor(x/16.0);  // coords of
    int _y = floor(y/16.0);  // renderfragment
    renderfragment temp(_x, _y);
    int index = lpixmap.indexOf(temp);
    if (index != -1)
        renderTile(x, y, &lpixmap[index].data);
}

void gameUI::paintPlayer(player p)
{
    QPainter painter(&pixmap_player);  // init
//    painter.translate(0, pixmap_player.height());
//    painter.scale(1, -1);

//    painter.setCompositionMode(QPainter::CompositionMode_Source);
#ifdef DRAW_AABB
    painter.setBrush(NoBrush);               // disable fill
    painter.setPen(QColor(127,127,127,127)); // gray semi-opaque
    painter.drawRect((p.x()-xCorner)*tilesize, (h-p.y()-p.h()+yCorner)*tilesize, p.w()*tilesize-1, p.h()*tilesize-1);
        // refer to QPainter::drawRect documentation for '-1' explanation
#endif
    // texture
    int gamma=0, delta=0;
    if (qAbs(p.vx()) > 0.1) gamma = ticks/7%3 * p.w() * tilesize;
    if (qAbs(p.vx()) > 0.4) gamma = ticks/5%3 * p.w() * tilesize;
    if (qAbs(p.vx()) > 0.6) gamma = ticks/4%3 * p.w() * tilesize;
    if (qAbs(p.vx()) > 0.7) gamma = ticks/3%3 * p.w() * tilesize;
    if (p.faced) delta = p.h() * tilesize;
    painter.drawPixmap((p.x() - xCorner)*tilesize, (h - p.y() - p.h() + yCorner)*tilesize + 2, p.w()*tilesize,
                            p.h()*tilesize, texturePlayer, gamma, delta, p.w()*tilesize, p.h()*tilesize);
    }

void gameUI::paintBullet(projectile bullet)
{
    QPainter painter(&pixmap_projectiles);
//    painter.translate(0, pixmap_projectiles.height());
//    painter.scale(1, -1);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.save();     // saving original state
    double xRender = (bullet.x()+bullet.w()/2-xCorner) * tilesize;
    double yRender = (h - (bullet.y() + bullet.h()/2-yCorner))*tilesize;
    painter.translate(xRender, yRender);
    painter.rotate(-bullet.getangle()*180/pi);
    painter.drawPixmap(-bullet.sizex*tilesize/2, -bullet.sizey*tilesize/2,
                       bullet.sizex*tilesize, bullet.sizey*tilesize, textureBullet);
    painter.restore();  // restoring original state

#ifdef DRAW_AABB
    painter.setBrush(NoBrush);
    painter.setPen(red);
    /*painter.drawRect((bullet.x() - xCorner)*tilesize, (h - bullet.y() - bullet.h() + yCorner)*tilesize,
                     (bullet.w() * cos(bullet.getangle()) + bullet.h() * sin(qAbs(bullet.getangle())))*tilesize,
                     (bullet.w() * sin(qAbs(bullet.getangle())) + bullet.h() * cos(bullet.getangle()))*tilesize);*/
    painter.drawRect((bullet.x() - xCorner)*tilesize, (h - bullet.y() - bullet.h() + yCorner)*tilesize,
                     bullet.w()*tilesize, bullet.h()*tilesize);
#endif
}

void gameUI::paintHover()
{
    QPainter painter(&pixmap_hover);
//    painter.translate(0, pixmap_hover.height());
//    painter.scale(1, -1);
    pixmap_hover.fill(transparent);
    if (sqrt(pow(xMouse - (playerCashe.x()+playerCashe.w()/2), 2) +
             pow(yMouse - (playerCashe.y()+playerCashe.h()/2), 2)) < 6 &&
        playerCashe.currentSlot < 2) // shovel or block
    { // distance from cursor to player
        painter.setPen(QColor(200, 200, 200, 150));
        painter.setBrush(QColor(200, 200, 200, 150));
        painter.drawRect((floor(xMouse)-xCorner)*tilesize, height() - (floor(yMouse)-yCorner+1)*tilesize, tilesize, tilesize);
    }
}

void gameUI::paintSky()
{ // fixme
#ifdef RENDER_BACKGROUND
    static QPixmap base = QPixmap(width(), height()); // base background image
    static bool first = true;                         // it is been painted only for f. time
    if (first) {
        QPainter painter(&base);
        painter.scale(1, -1);
        QLinearGradient gradient(width()/2, 0.0, width()/2, height()/2);
        gradient.setColorAt(0, QColor("#d9e9ff"));  // sky top
        gradient.setColorAt(1, QColor("#74a9f9"));  // sky bottom
        painter.setPen(NoPen);
        painter.setBrush(gradient);
        painter.drawRect(0, 0, width(), height()/2);
        painter.setBrush(QColor("#7c654c"));        // dirt
        painter.drawRect(0, height()/2, width(), height()*4/5);
        painter.setBrush(QColor("#9a9a9a"));        // stone
        painter.drawRect(0, height()*4/5, width(), height());
        first=false;
    }
    QPainter painter(&pixmap_background);
    painter.scale(1, -1);
    painter.drawPixmap(0, 0, width(), height(), base);

 // clouds
    painter.setPen(NoPen);
    painter.setBrush(white);
    painter.drawRect(ticks%width()-40, 40, 100, 30);
    painter.drawRect(ticks%(width()+100)*1.1-230, 130, 80, 28);
    painter.drawRect(ticks%(width()+300)*1.2-375, 75, 120, 35);
#endif
}

void gameUI::paintHud()
{
    pixmap_hud.fill();
    QPainter painter(&pixmap_hud);
    painter.setPen(black);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawText(10, 0, pixmap_hud.width()-20, pixmap_hud.height(), AlignCenter,
                     "Health: " + QString::number(playerCashe.health) + " " +
                     "Ammo: " + QString::number(playerCashe.slot[2]->ammoLoad) +
                     "/" + QString::number(playerCashe.slot[2]->ammoCarr) + " " +
                     "Blocks: " + QString::number(playerCashe.slot[1]->ammoLoad));
}

// ---------- Other ----------
void gameUI::calcCorner(player p)
{
    xCorner = p.x() + p.w()/2 - (width()/tilesize)/2;
    yCorner = p.y() + p.h()/2 - (height()/tilesize)/2;
    if (xCorner < 0) xCorner = 0;
    if (xCorner > levelCashe.w-width()/tilesize) xCorner = levelCashe.w-width()/tilesize;
    if (yCorner < 0) yCorner = 0;
    if (yCorner > levelCashe.h-height()/tilesize) yCorner = levelCashe.h-height()/tilesize;
}

// ---------- Tick ----------

void gameUI::regulateRenderFragmets()
{
    int xScreen = floor(xCorner/16.0)*16;
    int yScreen = floor(yCorner/16.0)*16;
    int wScreen = ceil((width()/tilesize)/16.0)*16;
    int hScreen = ceil((height()/tilesize)/16.0)*16;

    // unload unnecessary
    for (int i=0; i<lpixmap.size(); i++)
        if ((lpixmap[i].x+1)*16 < xScreen || lpixmap[i].x*16 > xScreen+wScreen ||
            (lpixmap[i].y+1)*16 < yScreen || lpixmap[i].y*16 > yScreen+hScreen)
        {
//            cerr << "[RENDER] unloaded chunk " << lpixmap[i].x << "; " << lpixmap[i].y << "\n";
            lpixmap.removeAt(i);
        }

    // load necessary
    for (int i=xScreen; i<=xScreen+wScreen; i+=16)
    {
        for (int j=yScreen; j<=yScreen+hScreen; j+=16)
        {
            if (i>=0 && i<=levelCashe.w && j>=0 && j<=levelCashe.h)
            {
                renderfragment tmp(i/16, j/16);
                if (!lpixmap.contains(tmp)) {
                    lpixmap.append(tmp);
                    renderFragment(&lpixmap.last());
//                    cerr << "[RENDER] loaded chunk " << i << "; " << j << "\n";
                }
            }
        }
    }

    /*cerr << "~~~~~~~~~~~\n";
    for (int i=0; i<lpixmap.size(); i++)
        cerr << lpixmap[i].x << " " << lpixmap[i].y << "\n";*/
}

void gameUI::timerEvent(QTimerEvent *)
{
    static QTime time;
    static int frameCnt=0;
    static double timeElapsed=0;

    ticks++;

 // sky
    paintSky();

 // widget data
    calcCorner(playerCashe);
    xMouse = xMouseReal/double(tilesize) + xCorner;
    yMouse = (height()-yMouseReal) / double(tilesize) + yCorner;

 // level
    regulateRenderFragmets();

 // updated tiles
    forlist(i, rerender)
    {
        renderTileAbsolute(rerender.at(i).x, rerender.at(i).y);
        renderTileAbsolute(rerender.at(i).x-1, rerender.at(i).y);
        renderTileAbsolute(rerender.at(i).x+1, rerender.at(i).y);
        renderTileAbsolute(rerender.at(i).x, rerender.at(i).y-1);
        renderTileAbsolute(rerender.at(i).x, rerender.at(i).y+1);
    }
    rerender.clear();

 // send information
    QDataStream stream(tcpSocket);

    if (pressedW != pressedWPrev || pressedA != pressedAPrev || pressedS != pressedSPrev || pressedD != pressedDPrev)
    {
        stream << (quint8)0x03 << pressedW << pressedA << pressedS << pressedD
                  /*<< pressedL << pressedR << xMouse << yMouse*/;
    }
    pressedWPrev = pressedW;
    pressedAPrev = pressedA;
    pressedSPrev = pressedS;
    pressedDPrev = pressedD;

    if (gameGoes) {
        QDataStream stream(tcpSocket);
        stream << (quint8)0x04
               << (quint8)0xff << (quint8)0xff
               << xMouse << yMouse
               << playerCashe.currentSlot
               << playerCashe.getSelectedColor();
    }

    // ping
    if (ticks%30==0) // every ~1 sec
        stream << (quint8)0xff << qint32(globalTime.elapsed());
//    tcpSocket->flush();
//    qDebug() << "Waiting to write" << tcpSocket->bytesToWrite() << "bytes";
//    qDebug() << "Waiting to read" << tcpSocket->bytesAvailable() << "bytes";

 // players
    pixmap_player.fill(transparent);
    forlist(i, players) //{
        paintPlayer(players.at(i));
//        clog << "Painted other player at " << players[i].x() << ";" << players[i].y() << "\n";
//    }
    paintPlayer(playerCashe);
//    qDebug() << players.size();

    pixmap_projectiles.fill(transparent);
    forlist(i, bulletsCashe)
        paintBullet(bulletsCashe.at(i));

 // hightlighting tile under cursor...
    paintHover();

 // rendering interface
    paintHud();

 // fps counting...
    frameCnt++;
    timeElapsed += time.elapsed();
    time.restart();
    if (timeElapsed >= 500)
    {
        fps = frameCnt * 1000.0 / timeElapsed;
        timeElapsed = 0;
        frameCnt = 0;
    }

 // forced widget repainting...
    update();
}
