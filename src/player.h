#ifndef PLAYER_H
#define PLAYER_H

#include "declarations.h"
#include "entity.h"
#include "weapon.h"
#ifndef SNS_UI_ONLY
#include "gamecore.h"
//#else
//#include "gameui.h"
#endif

class player : public entity
{
    char selectedColor[8];  // "#rrggbb\0"
    qint16 attackCooldown;

public:
    player(int _id=0, gameCore *calledCore=0);

    QString name;

//    bool inGame;

    qint16 health;
    // weapons and tools
//    qint16 blockAmount;
    weapon *slot[3];
    qint16 currentSlot;
    weapon *slotBlock;
    weapon *slotShovel;
    weapon *slotPrimary;
    //weapon *currentWeapon;

    // counters
    qint16 counterAfterDeath;
    qint16 counterTillRespawn;

    bool faced; // 0 means >, 1 means <
    bool reloading;

    void spawn(double xs, double ys);
    bool alive();
    void hurt(int h);
#ifndef SNS_UI_ONLY
    void tick();

    void jump();

    void useFire(double x, double y);
    void useAltFire(double x, double y);
#endif

    char *getSelectedColor();
    void setColor(char *_color);

    friend QDataStream &operator<<(QDataStream & s, const player &p);
    friend QDataStream &operator>>(QDataStream & s, player &p);
    friend int sizeofplayer();

};


#endif // PLAYER_H
