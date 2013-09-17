#include "declarations.h"
#ifndef SNS_UI_ONLY
#include "gamecore.h"
#endif
#include "global.h"
#include "tile.h"
#include "entity.h"
//#include <QPixmap>

entity::entity(gameCore *calledCore)
{
    core=calledCore;
    x_ = y_ = w_ = h_ = 0;
    vx_ = vy_ = 0;
    sizex = sizey = 0;
    flyingTime=0;
}

entity::~entity()
{
    ;
}

void entity::spawn(double xs, double ys)
{
    x_=xs; y_=ys;
}

void entity::tick(){}
void entity::display(){}

void entity::change_vx(double dvx)
{
//    if (checkSpaceDown(1) <= 0)
        vx_+=dvx;
//    else
//        vx_+=dvx*0.5;
}

void entity::change_vy(double dvy)
{
    vy_+=dvy;
}

#ifndef SNS_UI_ONLY
void entity::move()
{
    if (flying || checkSpaceDown(1)==0)
        flyingTime=0;
    else
        flyingTime++;

    if (vx_ > 0) {
        if (checkSpaceRight(vx_) >= vx_) {
            x_+=vx_;
            //cerr << checkSpaceRight(vx_) << ", working\n";
        }
        else {
            x_+=checkSpaceRight(vx_);
            vx_=0; // enity touched a block, slowing down...
            //cerr << vx_ << " > " << checkSpaceRight(vx_) << ", slowind down\n";
        }
        if (vx_>0.72) vx_=0.72;
    }

    if (vx_ < 0) {
        if (checkSpaceLeft(-vx_) >= -vx_) { // -vx_ = abs(vx_)
            x_+=vx_;
            //cerr << -vx_ << " <= " << checkSpaceLeft(-vx_) << ", working\n";
        }
        else {
            x_-=checkSpaceLeft(-vx_);
            vx_=0;
        }
        if (vx_<-0.72) vx_=-0.72;
    }

    if (vy_ > 0) {
        if (checkSpaceUp(vy_) >= vy_) {
            y_+=vy_;
        }
        else {
            y_+=checkSpaceUp(vy_);
            vy_=0;
        }
        if (vy_>0.72) vy_=0.72;
    }

    if (vy_ < 0) {
        if (checkSpaceDown(-vy_) >= -vy_) // -vy_ = abs(vy_)
            y_+=vy_;
        else {
            y_-=checkSpaceDown(-vy_);
            vy_=0;
        }
    }

    if (qAbs(vx_) < 0.001) vx_ = 0;
    if (qAbs(vy_) < 0.001) vy_ = 0;

//    if (x_<0) {x_=0; vx_=0;}
//    if (x_>w-sizex) {x_=w-sizex; vx_=0;}
    if (y_<0) {y_=0; vy_=0;}
    if (y_>core->level.h-h_) {y_=core->level.h-y_;/* vy_=0;*/}
    if (x_<0) x_=0;
    if (x_>core->level.w-w_) x_=core->level.w-w_;
//    if (y_>h-sizey) {y_=h-sizey; vy_=0;}
}
#endif

void entity::friction()
{
    vx_*=0.85; vy_*=0.85;
}

void entity::gravity()
{
    vy_ -= 0.5 * 0.005 * flyingTime * flyingTime;
}

#ifndef SNS_UI_ONLY
double entity::checkSpaceDown(double checkDist)
{ // возвращает свободное расстояние снизу
    if (checkDist < 0) cerr << "Negative D check argument!\n";
  //cerr << "sizex: " << sizex << " sizey: " << sizey << "\n";
    double i, j;
    double minDist = checkDist; // промежуточное расстояние до тайла, ищется минимум
    int xTemp, yTemp; // координаты тайла, к кот. принадлежит точка проверки
    i = -1;
    do { // проверка каждой ключевой точки нижней границы, вправо
        if (++i>w_) i=w_; // инкремент цикла
        xTemp = floor(x_ + i);
        j = -1;
        do { // проверка точек через 1 тайл, вниз
            if (++j>checkDist) j=checkDist; // инкремент цикла
            yTemp = floor(y_ - j);
            if (tileSolid(core->level, xTemp, yTemp)){
                minDist = (y_ - yTemp - 1) < minDist ? (y_ - yTemp - 1) : minDist;
            }            //y_ - (int(yTemp)+1)
        } while (j < checkDist);
    } while (i < w_);

    return minDist;
}

double entity::checkSpaceUp(double checkDist)
{ // возвращает свободное расстояние сверху
    if (checkDist < 0) cerr << "Negative U check argument!\n";
    double i, j;
    double minDist = checkDist+0.05; // промежуточное расстояние до тайла, ищется минимум
    int xTemp, yTemp; // координаты тайла, к которому принадлежит точка проверки
    i = -1;
    do { // проверка каждой ключевой точки верхней границы, вправо
        if (++i>w_) i=w_; // инкремент цикла
        xTemp = floor(x_ + i);
        j = -1;
        do { // проверка точек через 1 тайл, вверх
            if (++j>checkDist) j=checkDist; // инкремент цикла
            yTemp = floor(y_ + h_ + j);
            if (tileSolid(core->level, xTemp, yTemp)){
                minDist = (yTemp - y_ - h_) < minDist ? (yTemp - y_ - h_) : minDist;
            }
        } while (j < checkDist);
    } while (i < w_);

    return minDist-0.05;
}

double entity::checkSpaceLeft(double checkDist)
{ // возвращает свободное расстояние слева
    if (checkDist < 0) cerr << "Negative L check argument!\n";
    double i, j;
    double minDist = checkDist; // промежуточное расстояние до тайла, ищется минимум
    int xTemp, yTemp; // координаты тайла, к которому принадлежит точка проверки
    i = -1;
    do { // проверка каждой ключевой точки левой границы, вверх
        if (++i>h_) i=h_; // инкремент цикла
        yTemp = floor(y_ + i);
        j = -1;
        do { // проверка точек через 1 тайл, вниз
            if (++j>checkDist) j=checkDist; // инкремент цикла
            xTemp = floor(x_ - j);
            if (tileSolid(core->level, xTemp, yTemp)){
                minDist = (x_ - xTemp - 1) < minDist ? (x_ - xTemp - 1) : minDist;
            }
        } while (j < checkDist);
    } while (i < h_);

    return minDist;
}

double entity::checkSpaceRight(double checkDist)
{ // возвращает свободное расстояние справа
    if (checkDist < 0) cerr << "Negative R check argument!\n";
    double i, j;
    double minDist = checkDist+0.05; // промежуточное расстояние до тайла, ищется минимум
    int xTemp, yTemp; // координаты тайла, к кот. принадлежит точка проверки
    i = -1;
    do { // проверка каждой ключевой точки правой границы, вверх
        if (++i>h_) i=h_; // инкремент цикла
        yTemp = floor(y_ + i);
        j = -1;
        do { // проверка точек через 1 тайл, вниз
            if (++j>checkDist) j=checkDist; // инкремент цикла
            xTemp = floor(x_ + w_ + j);
            if (tileSolid(core->level, xTemp, yTemp)){
                minDist = (xTemp - x_ - w_) < minDist ? (xTemp - x_ - w_) : minDist;
            }
        } while (j < checkDist);
    } while (i < h_);

    /*if (checkDist > 1.8) // voodoo magic
        return minDist-0.05 > 0 ? minDist-0.05 : 0;
    else*/
    return minDist-0.05;
}

double entity::checkSpaceHor(double checkDist)
{
//    double i, j;
    cerr << "WORKING";
    cerr << qAbs(checkDist);
    double minDist = qAbs(checkDist);
    cerr << minDist;
    /*int xTemp, yTemp;
    i = -1;
    do {
        cerr << "W\n";
        if (++i>sizey) i=sizey;
        yTemp = y_ + i;
        j = -1;
        do {
            if (++j>checkDist) j=checkDist;
            xTemp = checkDist>0 ? x_ + sizex + j : x_ - j;
            if (tileSolid(xTemp, yTemp)) {
                if (checkDist > 0)
                    minDist = (xTemp - x_ - sizex) < minDist ? (xTemp - x_ - sizex) : minDist;
                else
                    minDist = (x_ - xTemp - 1) < minDist ? (x_ - xTemp - 1) : minDist;
            }
        } while (j < abs(checkDist));
    } while (i < sizey);

    return minDist;*/
    return -1;
}

bool inTiles()
{

}

QList<QPoint> entity::tilesOccuped()
{
    QList<QPoint> tiles;
    for (double i=x_; i<x_+sizex; i++)
        for (double j=y_; j<y_+sizey; j++)
            try {
                if (tileSolid(core->level, i, j))
                    tiles.append(QPoint(i,j));
            } catch (...) {}
    return tiles;
}

#endif

int entity::id(){return id_;}
double entity::x(){return x_;}
double entity::y(){return y_;}
double entity::w(){return w_;}
double entity::h(){return h_;}
double entity::vx(){return vx_;}
double entity::vy(){return vy_;}

// ================================

bool interferes(entity *e1, entity *e2)
{
    double x1 = e1->x();
    double y1 = e1->y();
    double w1 = e1->w();
    double h1 = e1->h();
    double x2 = e2->x();
    double y2 = e2->y();
    double w2 = e2->w();
    double h2 = e2->h();

    return  (x1 >= x2 && x1 <= x2+w2 && y1 >= y2 && y1 <= y2+h2) ||
            (x1+w1 >= x2 && x1+w1 <= x2+w2 && y1 >= y2 && y1 <= y2+h2) ||
            (x1 >= x2 && x1 <= x2+w2 && y1+h1 >= y2 && y1+h1 <= y2+h2) ||
            (x1+w1 >= x2 && x1+w1 <= x2+w2 && y1+h1 >= y2 && y1+h1 <= y2+h2) ||
            (x2 >= x1 && x2 <= x1+w1 && y2 >= y1 && y2 <= y1+h1) ||
            (x2+w2 >= x1 && x2+w2 <= x1+w1 && y2 >= y1 && y2 <= y1+h1) ||
            (x2 >= x1 && x2 <= x1+w1 && y2+h2 >= y1 && y2+h2 <= y1+h1) ||
            (x2+w2 >= x1 && x2+w2 <= x1+w1 && y2+h2 >= y1 && y2+h2 <= y1+h1);
}
