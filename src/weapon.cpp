#include "weapon.h"
#include "global.h"
#include "tile.h"
#include "player.h"
#include "gamecore.h"
#include "projectile.h"

// -------- base class --------
weapon::weapon(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner)
{
    owner = _owner;
    baseDamage = _baseDamage;
    ammoLoad = ammoLoadMax = _ammoLoadMax;
    ammoCarr = ammoCarrMax = _ammoCarrMax;
    reloadTime = _reloadTime;
    cooldownTime = _cooldownTime;
    cooldown = 0;
}

weapon::~weapon(){}

void weapon::reload()
{
    if (cooldown == 0)
        if (ammoLoad < ammoLoadMax && ammoCarr > 0)
        {
            qint16 amount = qMin(qint16(ammoLoadMax - ammoLoad), ammoCarr);
            ammoCarr -= amount;
            ammoLoad += amount;
            cooldown = reloadTime;
        }
}

// -------- subclasses --------
weaponShovel::weaponShovel(qint16 _baseDamage, qint16 _cooldownTime, player *_owner)
    : weapon(_baseDamage, 0, 0, _cooldownTime, 0, _owner){}
weaponShovel::~weaponShovel(){}

#ifndef SNS_UI_ONLY
void weaponShovel::shoot(double x, double y)
{
    if (cooldown == 0 &&
        sqrt(pow(x - (owner->x()+owner->w()/2), 2) +
             pow(y - (owner->y()+owner->h()/2), 2)) < 6)
    {
        if (tileType(owner->core->level, floor(x), floor(y)) == 1)
        {
            try {
                setTile(owner->core->level, floor(x), floor(y), 0, "#ffffff", 0);
            } catch (...)
            {
                return;
            }
            owner->core->updateTile(floor(x), floor(y));
            owner->slotBlock->ammoLoad++;
        }
    }
}
#endif

weaponBlock::weaponBlock(qint16 _ammoLoadMax, qint16 _cooldownTime, player *_owner)
    : weapon(0, _ammoLoadMax, 0, _cooldownTime, 0, _owner){}
weaponBlock::~weaponBlock(){}

#ifndef SNS_UI_ONLY
void weaponBlock::shoot(double x, double y)
{
    if (cooldown == 0 &&
        ammoLoad > 0 &&
        sqrt(pow(x - (owner->x()+owner->w()/2), 2) +
             pow(y - (owner->y()+owner->h()/2), 2)) < 6)
    {
        if (tileType(owner->core->level, floor(x), floor(y)) == 0 &&
            tileConnected(owner->core->level, floor(x), floor(y)))
        {
            try {
                setTile(owner->core->level, floor(x), floor(y), 1, owner->getSelectedColor(), 70);
            } catch (...)
            {
                return;
            }
            owner->core->updateTile(floor(x), floor(y));
            ammoLoad--;
        }
    }
}
#endif

weaponSmg::weaponSmg(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner)
    : weapon(_baseDamage, _ammoLoadMax, _ammoCarrMax, _cooldownTime, _reloadTime, _owner){}
weaponSmg::~weaponSmg(){}

#ifndef SNS_UI_ONLY
void weaponSmg::shoot(double x, double y)
{
    double a = atan2(y - (owner->y()+owner->h()/2), x - (owner->x()+owner->w()/2));
    if (cooldown == 0)
    {
        if (ammoLoad > 0)
        {
            owner->core->createBullet(owner->id(), owner->x() + owner->w()/2, owner->y() + owner->h()/2, a + myRand(-0.05, 0.05), baseDamage);
            ammoLoad--;
            cooldown = cooldownTime;
        } else
            reload();
    }
}
#endif

weaponShotgun::weaponShotgun(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner)
    : weapon(_baseDamage, _ammoLoadMax, _ammoCarrMax, _cooldownTime, _reloadTime, _owner){}
weaponShotgun::~weaponShotgun(){}

#ifndef SNS_UI_ONLY
void weaponShotgun::shoot(double x, double y)
{
    double a = atan2(y - (owner->y()+owner->h()/2), x - (owner->x()+owner->w()/2));
    if (cooldown == 0)
    {
        if (ammoLoad > 0)
        {
            for (int i=0; i<6; i++)
                owner->core->createBullet(owner->id(), owner->x() + owner->w()/2, owner->y() + owner->h()/2, a + myRand(-0.1, 0.1), baseDamage);
            ammoLoad--;
            cooldown = cooldownTime;
        } else
            reload();
    }
}
#endif

weaponSniperRifle::weaponSniperRifle(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner)
    : weapon(_baseDamage, _ammoLoadMax, _ammoCarrMax, _cooldownTime, _reloadTime, _owner){}
weaponSniperRifle::~weaponSniperRifle(){}

#ifndef SNS_UI_ONLY
void weaponSniperRifle::shoot(double x, double y)
{
    double a = atan2(y - (owner->y()+owner->h()/2), x - (owner->x()+owner->w()/2));
    if (cooldown == 0)
    {
        if (ammoLoad > 0)
        {
            owner->core->createBullet(owner->id(), owner->x() + owner->w()/2, owner->y() + owner->h()/2, a, baseDamage);
            ammoLoad--;
            cooldown = cooldownTime;
        } else
            reload();
    }
}
#endif
