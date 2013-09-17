#include "projectile.h"
#include "gamecore.h"
#include "tile.h"
#include "math.h"
#include "player.h"

projectile::projectile()
{
    sizex = 1;
    sizey = 0.125;
}

projectile::projectile(gameCore *calledCore, int ownerID, double xs, double ys, double _angle, int _damage) :
    entity(calledCore)
{
    owner = ownerID;
    sizex = 1;
    sizey = 0.125;

    angle = _angle;
    w_ = qAbs(sizex * cos(angle)) + qAbs(sizey * sin(angle));
    h_ = qAbs(sizex * sin(angle)) + qAbs(sizey * cos(angle));
    if (w_ <= 0)
        qDebug() << "created projectile with w =" << w_ << "!";
    if (h_ <= 0)
        qDebug() << "created projectile with h =" << h_ << "!";
    //qDebug() << "created projectile with angle" << _angle << "and w" << w_ << "h" << h_;
    x_ = xs - w_/2;
    y_ = ys - h_/2;
    //qDebug() << "created projectile with center at" << x_ + w_/2 << y_ + h_/2;

    speed = 1.0;
    vx_ = speed * cos(angle);
    vy_ = speed * sin(angle);

    damage = _damage;
    liveTime = 0;
    energy = 1.0;
    path = 0;
    remove = false;
}

void projectile::tick()
{
#ifndef SNS_UI_ONLY
    foreach(SnsThread *t, core->threads)
    {
        if (interferes(this, t->p) && owner != t->p->id())
        {
            t->p->hurt(damage*energy);
//            energy = 0;
        }
    }
    move();
#endif
    path += speed;

    /*if (path < x50/2)
    {
        ;
    } else if (path >= x50/2 && path < x50)
    {
        ;
    } else if (path >= x50 && path < x0/2)
    {
        ;
    } else if (path >= x0/2 && path < x0)
    {
        ;
    } else
    {
        remove = true;
    }*/
    energy = (-1.0/(double)x0)*path + 1;
    if (energy < 0.01)
        remove = true;
}

double projectile::getangle()
{
    return angle;
}

#ifndef SNS_UI_ONLY
void projectile::move()
{
    x_+=vx_;
    y_+=vy_;

    foreach(QPoint i, tilesOccuped())
    {
        try {
            if (tileType(core->level, i.x(), i.y()) == 1) {
                if (tileDealDamage(core->level, i.x(), i.y(), energy*damage)) {
                    core->updateTile(i.x(), i.y());
                } else
                {
                    remove = true; // like delete this;
                }
                /*qint16 &durability = tileDurability(core->level, i.x(), i.y());
                if (durability < energy*damage) {
                    //energy -= durability;
                    setTile(core->level, i.x(), i.y(), 0, (char *)"#ffffff", 0);
                    core->updateTile(i.x(), i.y());
                } else {
                    durability -= energy*damage;
                    remove = true; // like delete this;
                }*/
            }
        } catch (...) {}
    }
}
#endif
