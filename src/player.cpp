#include "player.h"
#include "gamecore.h"
#include "global.h"
#include "tile.h"
#include <QMutexLocker>

//QPixmap texture;

player::player(int _id, gameCore *calledCore) :
    entity(calledCore)
{
    id_ = _id;
//    inGame = false;
    flying = false;

    w_ = sizex = 1.5;
    h_ = sizey = 2.75;

    health = 100;
    //slot1 = itemShovel;
    //slot2 = itemBlock;
    slot[0] = slotShovel = new weaponShovel(60, 5, this);
    slot[1] = slotBlock = new weaponBlock(75, 5, this);
    slot[2] = slotPrimary = new weaponSmg(30, 30, 150, 3, 17, this);
    currentSlot = 2;
    //currentWeapon = slot[currentSlot];
//    slotPrimary;
//    primaryWeapon = new weaponSmg(100, 30, 150, 3, 17, this);
//    selectedSlot = 1;
    //selectedItem = itemShovel;

    counterAfterDeath = -1;
    counterTillRespawn = 0;

    qstrcpy(selectedColor, "#0000ff");
    attackCooldown = 0;
//    inGame = true;
}

#ifndef SNS_UI_ONLY
void player::tick()
{ // calculating
    if (health > 0)
    {
        gravity();
        friction();
        move();
        if (attackCooldown > 0)
            attackCooldown--;

        if (vx_>0)
            faced = 0;
        else if (vx_<0)
            faced = 1;
        if (slot[currentSlot]->cooldown > 0)
            slot[currentSlot]->cooldown--;
    } else if (health <= 0 && counterAfterDeath == -1)
    {
        counterAfterDeath = 0;
        counterTillRespawn = 50;
        health = 0;
        qDebug() << name << "died";
    } else if (health <= 0 && counterTillRespawn == 0)
    {
        counterAfterDeath = -1;
        health = 100;
        fr(i,3)
        {
            slot[i]->ammoLoad = slot[i]->ammoLoadMax;
            slot[i]->ammoCarr = slot[i]->ammoCarrMax;
        }
        spawn(myRand(core->level.w*0.1, core->level.w*0.9), core->level.h*0.6);
    } else
    {
        counterAfterDeath++;
        counterTillRespawn--;
    }
}

// ---------- Move functions ----------

void player::jump()
{
    static bool firstFlight=true;
    if (checkSpaceDown(1) <= 0)
        firstFlight = true;
    if (checkSpaceDown(1) > 0 && vy_ < 0)
        firstFlight = false;

    if (flying || (checkSpaceDown(1) <= 4 && firstFlight))
        vy_ += 0.16;
}
#endif

// ---------- Game functions ----------

void player::spawn(double xs, double ys)
{
    x_=xs; y_=ys;
}

bool player::alive()
{
    return health > 0;
}

void player::hurt(int h)
{
    health -= h;
}

char *player::getSelectedColor()
{
    return selectedColor;
}

void player::setColor(char *_color)
{
    qstrcpy(selectedColor, _color);
}

#ifndef SNS_UI_ONLY
void player::useFire(double x, double y)
{
    slot[currentSlot]->shoot(x, y);
}

void player::useAltFire(double x, double y)
{
    ;
}
#endif
