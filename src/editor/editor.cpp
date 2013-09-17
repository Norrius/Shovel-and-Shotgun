#include "editor.h"
#include "ui_editor.h"
#include <QPainter>
#include <QInputEvent>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>

editor::editor(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::editor)
{
    ui->setupUi(this);

    ui->toolBar->addAction(QIcon(":resources/tileAir.png"), "Air", this, SLOT(actTileAir()));
    ui->toolBar->addAction(QIcon(":resources/tileNormal.png"), "Normal tile", this, SLOT(actTileNormal()));
    ui->toolBar->addAction(QIcon(":resources/tileAdamantine.png"), "Adamantine tile", this, SLOT(actTileAdamantine()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon(":resources/colorPicker.png"), "Select color", this, SLOT(actColor()));
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon(), "Smoother", this, SLOT(actSmoother()));
    QSlider *slider;
    slider = new QSlider(Horizontal);
    slider->setMaximumWidth(100);
    slider->setMinimum(15);
    slider->setMaximum(250);
    slider->setValue(70);
    QAction *actSlider = ui->toolBar->addWidget(slider);
    actSlider->setVisible(true);

    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(actNew()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(actOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(actSave()));
    connect(ui->actionSave_as, SIGNAL(triggered()), this, SLOT(actSaveAs()));

    connect(ui->actionAir, SIGNAL(triggered()), this, SLOT(actTileAir()));
    connect(ui->actionNormal_tile, SIGNAL(triggered()), this, SLOT(actTileNormal()));
    connect(ui->actionAdamantine, SIGNAL(triggered()), this, SLOT(actTileAdamantine()));
    connect(ui->actionColor, SIGNAL(triggered()), this, SLOT(actColor()));

    sprite_tile = QPixmap();
    sprite_tile.load(":resources/tile.png");
    sprite_border = QPixmap();
    sprite_border.load(":resources/border.png");

    levelLoaded = false;
    pressedL = false;
    pressedR = false;
    mode = 1;
    qstrcpy(selectedColor, "#0000ff");
    selectedDurability = 70;

    xCorner = yCorner = 0;
    xMouseReal = yMouseReal = 0;

    show();

    /*// ~~~ generating flat map ~~~
    QFile f("maps/fall.snsmap");
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    _map tmp;
    tmp.w=150;
    tmp.h=60;
    tmp.allocate();
    for (int i=0; i<tmp.w; i++)
        for (int j=0; j<tmp.h; j++)
            if (j<25) {
                tmp[i][j].type = 1;
                qstrcpy(tmp[i][j].color, "#cccccc");
                tmp[i][j].durability = 70;
            } else if (j==25) {
                tmp[i][j].type = 1;
                qstrcpy(tmp[i][j].color, "#00a000");
                tmp[i][j].durability = 50;
            } else {
                tmp[i][j].type = 0;
                qstrcpy(tmp[i][j].color, "#ffffff");
                tmp[i][j].durability = 0;
            }
    out << tmp;*/
}

editor::~editor()
{
    delete ui;
}

void editor::actNew()
{
    if (levelLoaded)
        level.clear();
    int wNew = QInputDialog::getInt(this, QString("Creating the map"), QString("Width:"), 0, 0, 1024);
    int hNew = QInputDialog::getInt(this, QString("Creating the map"), QString("Height:"), 0, 0, 1024);
    QString nameNew = QInputDialog::getText(this, QString("Creating the map"), QString("Name:"));

    QFile f("maps/"+nameNew+".snsmap");
    f.open(QIODevice::WriteOnly);
    QDataStream out(&f);
    _map tmp;
    tmp.w=wNew;
    tmp.h=hNew;
    tmp.allocate();
    for (int i=0; i<tmp.w; i++)
        for (int j=0; j<tmp.h; j++)
        {
            tmp[i][j].type = 0;
            qstrcpy(tmp[i][j].color, "#ffffff");
            tmp[i][j].durability = 0;
        }
    out << tmp;

    readLevel("maps/"+nameNew+".snsmap");
}

void editor::actOpen()
{
    if (levelLoaded)
        level.clear();
    QString file = QFileDialog::getOpenFileName();
    readLevel(file);
    mapFilename = file;
    pixmap = new QPixmap(level.w*tilesize, level.h*tilesize);
    render();
    levelLoaded = true;
}

void editor::actSave()
{
    writeLevel(mapFilename);
}

void editor::actSaveAs()
{
    QString file = QFileDialog::getSaveFileName();
    writeLevel(file);
}

void editor::actColor()
{
    const char *color = QColorDialog::getColor().name().toLatin1().constData();
    qstrcpy(selectedColor, color);
}

void editor::actTileAir()
{
    mode = 0;
}

void editor::actTileNormal()
{
    mode = 1;
}

void editor::actTileAdamantine()
{
    mode = 2;
}

void editor::actSmoother()
{
    mode = 3;
}

void editor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (levelLoaded)
    {
        painter.drawPixmap(xCorner, yCorner, *pixmap);

        /*int x = floor((xMouseReal-xCorner)/double(tilesize));
        int y = level.h - floor((yMouseReal-yCorner)/double(tilesize));
        if (x>=0&&x<level.w&&y>=0&&y<level.h)
        {
            painter.drawText(2, 34, QString::number(level[x][y].type));
            painter.drawText(2, 46, QColor(level[x][y].color).name());
            painter.drawText(2, 58, QColor(selectedColor).name());
        }*/
    }
}

void editor::mousePressEvent(QMouseEvent *event)
{
    xMouseReal = event->x();
    yMouseReal = event->y();
    switch (event->button()) {
    case LeftButton:
        pressedL = true;
        break;
    case RightButton:
        pressedR = true;
        break;
    case MiddleButton:
    {
        int x = floor((xMouseReal-xCorner)/double(tilesize));
        int y = level.h - floor((yMouseReal-yCorner)/double(tilesize)) - 1;
        qstrcpy(selectedColor, tileColor(x, y));
        break;
    }
    default:    // fixing warnings...
        break;
    }
    listenToMouse();
}

void editor::mouseReleaseEvent(QMouseEvent *event)
{
    xMouseReal = event->x();
    yMouseReal = event->y();
    switch (event->button()) {
    case LeftButton:
        pressedL = false;
        break;
    case RightButton:
        pressedR = false;
        render();
        break;
    default:
        break;
    }
}

void editor::mouseMoveEvent(QMouseEvent *event)
{
    if (pressedR)
    {
        xCorner += event->x() - xMouseReal;
        yCorner += event->y() - yMouseReal;
        xMouseReal = event->x();
        yMouseReal = event->y();
        update();
    } else if (pressedL)
    {
        xMouseReal = event->x();
        yMouseReal = event->y();
        listenToMouse();
        update();
    } else
    {
        xMouseReal = event->x();
        yMouseReal = event->y();
    }
}

void editor::listenToMouse()
{
    if (levelLoaded)
    {
        if (pressedL)
        {
            int x = floor((xMouseReal-xCorner)/double(tilesize));
            int y = level.h - floor((yMouseReal-yCorner)/double(tilesize)) - 1;
            switch (mode)
            {
            case 0:
            case 1:
            case 2:
                try {
                    setTile(x, y, mode,
                            mode?selectedColor:(char *)"#ffffff",
                            selectedDurability);
                } catch (...) {}
                updateTile(x, y);
                break;
            case 3:
                if (x>=0 && y>=0 && x<level.w && y<level.h && tileType(x, y))
                {
                    int h=0, s=0, v=0;
                    int c = 0;
                    for (int i=-1; i<2; i++)
                        for (int j=-1; j<2; j++, c++)
                        {
                            try {
                                QColor tmp(tileColor(x+i, y+j));
                                h += tmp.hue();
                                s += tmp.saturation();
                                v += tmp.value();
                            } catch (...) {
                                QColor tmp(tileColor(x+(-1*i), y+(-1*j)));
                                h += tmp.hue();
                                s += tmp.saturation();
                                v += tmp.value();
                            }
                        }
                    h /= c;
                    s /= c;
                    v /= c;
                    QColor q;
                    q.setHsv(h, s, v);
                    char m[8];
                    for (int i=0; i<8; i++)
                        m[i] = (q.name().constData()+i)->toAscii();
                    qstrcpy(tileColor(x, y), m);
                    updateTile(x, y);
                    break;
                }
            }
        }
    }
}

void editor::render()
{
    pixmap->fill(transparent);
    for (int i = -xCorner/tilesize; i <= -(xCorner-width())/tilesize; i++)
        for (int j = -(height()-yCorner/tilesize); j <= level.h+(yCorner/tilesize); j++)
            renderTile(i, j);
    update();
}

void editor::renderTile(int x, int y)
{
    int type;
    QColor color;
    int connection;
    try {
        type = tileType(x, y);
        color = QColor(tileColor(x, y));
        connection = tileConnectionType(x, y);
    }
    catch (char *ex) {
        return;
    }

//    cerr << "Rendering tile ("<<x<<"; "<<y<<") as type="<<type<<"\n";

    double xDisplay = x*tilesize;
    double yDisplay = pixmap->height() - (y+1)*tilesize;

    QPainter painter(pixmap);

    // tile texture
    if (type == 0) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.setBrush(white);
        painter.setPen(NoPen);
        painter.drawRect(xDisplay, yDisplay, tilesize, tilesize);
    } else if (type < 3) {
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPixmap(xDisplay, yDisplay, tilesize, tilesize,
                           sprite_tile, 0, connection*tilesize, tilesize, tilesize);

        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.setBrush(color);
        painter.setPen(NoPen);
        painter.drawRect(xDisplay, yDisplay, tilesize, tilesize);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawPixmap(xDisplay, yDisplay, tilesize, tilesize,
                           sprite_border, 0, connection*tilesize, tilesize, tilesize);
    }
}

void editor::updateTile(int x, int y)
{
    renderTile(x, y);
    renderTile(x-1, y);
    renderTile(x+1, y);
    renderTile(x, y-1);
    renderTile(x, y+1);
    update();
}

// ---------- level data ----------

void editor::setTile(int x, int y, int type, char color[], double durability)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw (char *)"Exception: tile is not in range.";
    level[x][y].type = type;
    qstrcpy(level[x][y].color, color);
    level[x][y].durability = durability;
}

int editor::tileConnectionType(int x, int y)
{
    try {
        int type=0;
        if (tileSolid(x-1, y)) type+=1;
        if (tileSolid(x+1, y)) type+=2;
        if (tileSolid(x, y-1)) type+=4;
        if (tileSolid(x, y+1)) type+=8;
        return type;
    }
    catch (char *ex) {
#ifdef LOG_EXCEPTION_TILE_IS_NOT_IN_RANGE
        cerr << ex << "\n";
#endif
        return 0;
    }
}

bool editor::tileSolid(int x, int y)
{
    try {
        return (tileType(x, y) == 1 || tileType(x, y) == 2);
    }
    catch (char *ex) {
#ifdef LOG_EXCEPTION_TILE_IS_NOT_IN_RANGE
        cerr << ex << "\n";
#endif
        return false;
    }
}

qint16 &editor::tileType(int x, int y)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw (char *)"Exception: tile is not in range.";
    return level[x][y].type;
}

char *editor::tileColor(int x, int y)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw (char *)"Exception: tile is not in range.";
    return level[x][y].color;
}

// ---------- I/O ----------

void editor::readLevel(QString _file)
{
    QFile file(_file);
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    stream >> level;
}

void editor::writeLevel(QString _file)
{
    QFile file(_file);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << level;
}

QDataStream &operator<<(QDataStream & s, const char &c)
{
    quint8 t;
    t = c;
    s << t;
    return s;
}

QDataStream &operator>>(QDataStream & s, char &c)
{
    quint8 t;
    s >> t;
    c = t;
    return s;
}

QDataStream &operator<<(QDataStream & s, const tile &t)
{
    s << t.type;
    fr(i,8)
        s << t.color[i];
    s << t.durability;
    return s;
}

QDataStream &operator>>(QDataStream & s, tile &t)
{
    s >> t.type;
    fr(i,8)
        s >> t.color[i];
    s >> t.durability;
    return s;
}

QDataStream &operator<<(QDataStream & s, _map &m)
{
    s << m.w << m.h;
    for (int i=0; i<m.w; i++)
        for (int j=0; j<m.h; j++)
            s << m[i][j];
    return s;
}

QDataStream &operator>>(QDataStream & s, _map &m)
{
    s >> m.w >> m.h;
    m.allocate();
    for (int i=0; i<m.w; i++)
        for (int j=0; j<m.h; j++)
            s >> m[i][j];
    return s;
}
