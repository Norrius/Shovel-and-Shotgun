#ifndef PROJECTILE_H
#define PROJECTILE_H
#include "declarations.h"
#include "entity.h"

const double x50 = 32;
const double x0 = 72;

class projectile : public entity
{
    double angle;
    double speed;
    int damage;
    int liveTime; // old
    double energy;
    double path;
    int owner;

public:
    projectile();
    projectile(gameCore *calledCore, int ownerID, double xs, double ys, double _angle, int _damage);
    bool remove;

    void tick();
#ifndef SNS_UI_ONLY
    void move();
#endif

    double getangle();

    friend QDataStream &operator<<(QDataStream & s, const projectile &p);
    friend QDataStream &operator>>(QDataStream & s, projectile &p);
    friend int sizeofprojectile();
};

#endif // PROJECTILE_H
