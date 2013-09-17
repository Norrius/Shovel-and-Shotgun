#include "io.h"
#include "player.h"
#include "weapon.h"
#include "projectile.h"

//stream.setVersion(QDataStream::Qt_4_8);

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
    //qDebug().nospace() << "Outgoing level data: " << m.w << "x" << m.h;
    for (int i=0; i<m.w; i++)
        for (int j=0; j<m.h; j++)
            s << m[i][j];
    //cerr << "Size of map: " << sizeof(m.w)+sizeof(m.h)+(sizeof(tile::type)+sizeof(tile::color)+sizeof(tile::durability))*m.w*m.h<< "\n";
    return s;
}

QDataStream &operator>>(QDataStream & s, _map &m)
{
    s >> m.w >> m.h;
    //qDebug().nospace() << "Incoming level data: " << m.w << "x" << m.h;
    m.allocate();
    for (int i=0; i<m.w; i++)
        for (int j=0; j<m.h; j++)
            s >> m[i][j];
    return s;
}

QDataStream &operator<<(QDataStream & s, const player &p)
{
    s << p.x_ << p.y_ << p.vx_ << p.vy_
      /*<< p.selectedItem << p.selectedColor*/
      << p.faced
      << p.health
      << p.counterAfterDeath << p.counterTillRespawn;
//    cerr << "Player's size: " << sizeof(p.x_)+sizeof(p.y_)+sizeof(p.vy_)+sizeof(p.vx_)+sizeof(p.selectedItem)+sizeof(p.selectedColor) << "\n";
    return s;
}

QDataStream &operator>>(QDataStream & s, player &p)
{
    s >> p.x_ >> p.y_ >> p.vx_ >> p.vy_
      /*>> p.selectedItem >> p.selectedColor*/
      >> p.faced
      >> p.health
      >> p.counterAfterDeath >> p.counterTillRespawn;
    return s;
}

QDataStream &operator<<(QDataStream & s, const weapon &w)
{
    s << w.ammoLoad << w.ammoLoadMax
      << w.ammoCarr << w.ammoCarrMax;
    return s;
}

QDataStream &operator>>(QDataStream & s, weapon &w)
{
    s >> w.ammoLoad >> w.ammoLoadMax
      >> w.ammoCarr >> w.ammoCarrMax;
    return s;
}

QDataStream &operator<<(QDataStream & s, const projectile &p)
{
    s << p.x_ << p.y_ << p.w_ << p.h_ << /*p.vx_ << p.vy_ << */p.angle;
    return s;
}

QDataStream &operator>>(QDataStream & s, projectile &p)
{
    s >> p.x_ >> p.y_ >> p.w_ >> p.h_ >> /*p.vx_ >> p.vy_ >> */p.angle;
    return s;
}

// sizes used in networking

int sizeoftile()
{
    return sizeof(tile::type)+sizeof(tile::color[0])*8+sizeof(tile::durability);
}

int sizeofmap(_map *level)
{
    return sizeof(level->w)+sizeof(level->h)+sizeoftile()*level->w*level->h;
}

int sizeofplayer()
{
    return sizeof(player::x_)+sizeof(player::y_)+sizeof(player::vx_)+sizeof(player::vy_)+
           sizeof(player::faced)+sizeof(player::health)+
           sizeof(player::counterAfterDeath)+sizeof(player::counterTillRespawn);
}

int sizeofweapon()
{
    return sizeof(weapon::ammoLoad)+sizeof(weapon::ammoLoadMax)+
           sizeof(weapon::ammoCarr)+sizeof(weapon::ammoCarrMax);
}

int sizeofprojectile()
{
    return sizeof(projectile::x_)+sizeof(projectile::y_)+
           sizeof(projectile::w_)+sizeof(projectile::h_)+
           sizeof(projectile::angle);
}
