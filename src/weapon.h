#ifndef WEAPON_H
#define WEAPON_H

/*****************************************
 * Abstract type for weapons
 * Should reimplement pure shoot()
 * to make it work
 *****************************************/

#include "declarations.h"

class weapon
{

public:
    player *owner;
//    gameCore *core;

    // weapon characteristics
    qint16 baseDamage;

    qint16 ammoLoad;
    qint16 ammoLoadMax;
    qint16 ammoCarr;
    qint16 ammoCarrMax;

    qint16 reloadTime;
    qint16 cooldownTime;

    // cooldown timer
    qint16 cooldown;

    // weapon construction!
    weapon(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner);
    virtual ~weapon();

    //  use functions
#ifndef SNS_UI_ONLY
    virtual void shoot(double x, double y)=0;
#endif
    void reload();
};

// one weapon type - one subclass
class weaponShovel : public weapon
{
public:
    weaponShovel(qint16 _baseDamage, qint16 _cooldownTime, player *_owner);
    ~weaponShovel();
#ifndef SNS_UI_ONLY
    void shoot(double x, double y);
#endif
};

class weaponBlock : public weapon
{
public:
    weaponBlock(qint16 _ammoLoadMax, qint16 _cooldownTime, player *_owner);
    ~weaponBlock();
#ifndef SNS_UI_ONLY
    void shoot(double x, double y);
#endif
};

class weaponSmg : public weapon
{
public:
    weaponSmg(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner);
    ~weaponSmg();
#ifndef SNS_UI_ONLY
    void shoot(double x, double y);
#endif
};

class weaponShotgun : public weapon
{
public:
    weaponShotgun(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner);
    ~weaponShotgun();
#ifndef SNS_UI_ONLY
    void shoot(double x, double y);
#endif
};

class weaponSniperRifle : public weapon
{
public:
    weaponSniperRifle(qint16 _baseDamage, qint16 _ammoLoadMax, qint16 _ammoCarrMax, qint16 _cooldownTime, qint16 _reloadTime, player *_owner);
    ~weaponSniperRifle();
#ifndef SNS_UI_ONLY
    void shoot(double x, double y);
#endif
};

#endif // WEAPON_H
