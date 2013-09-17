#include "declarations.h"

void setTile(_map &level, int x, int y, int type, const char color[]="", double durability=0);
void setTile(_map &level, int x, int y, tile another);
qint16 &tileType(_map &level, int x, int y);
char *tileColor(_map &level, int x, int y);
qint16 &tileDurability(_map &level, int x, int y);
bool tileDealDamage(_map &level, int x, int y, int d);
//int &tileMetadata(_map &level, int x, int y);

//int tileTransparent(_map &level, int x, int y);
bool tileSolid(_map &level, int x, int y);
bool tileConnected(_map &level, int x, int y);
