#ifndef ENTITY_H
#define ENTITY_H

#include "declarations.h"

/****************************************************************
 * a_ represents private/protected variable
 * a() - public getter function
 * w_ & h_ are AABB size, while sizex & sizey - texture size
 ****************************************************************/

class entity
{
protected:
    int id_;
    //int flyingTime;
    double x_, y_;
    double w_, h_;
    double vx_, vy_;

#ifndef SNS_UI_ONLY
    double checkSpaceDown(double checkDist);
    double checkSpaceUp(double checkDist);
    double checkSpaceLeft(double checkDist);
    double checkSpaceRight(double checkDist);

    double checkSpaceHor(double checkDist);

    // new physics!
    bool inTiles();
    QList<QPoint> tilesOccuped();

#endif

public:
    gameCore *core;

    double sizex, sizey; // basic model size [in-game]

    bool flying;
    double flyingTime;

    entity(gameCore *calledCore=0);
    ~entity();
    void spawn(double xs, double ys);
    virtual void display();
    virtual void tick();
    void change_vx(double dvx);
    void change_vy(double dvy);
#ifndef SNS_UI_ONLY
    void move();
#endif
    void friction();
    void gravity();

    int id();
    double x();
    double y();
    double w();
    double h();
    double vx();
    double vy();

};

bool interferes(entity *e1, entity *e2);

#endif // ENTITY_H
