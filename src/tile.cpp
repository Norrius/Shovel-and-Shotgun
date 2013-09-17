#include "tile.h"

void setTile(_map &level, int x, int y, int type, const char color[], double durability)
{
    if (x<0||x>=level.w||y<0||y>=level.h) return; //throw "Exception: tile does not exist.";
    level[x][y].type = type;
    qstrcpy(level[x][y].color, color);
    level[x][y].durability = durability;
}

void setTile(_map &level, int x, int y, tile another)
{
    if (x<0||x>=level.w||y<0||y>=level.h) return; //throw "Exception: tile does not exist.";
    level[x][y].type = another.type;
    qstrcpy(level[x][y].color, another.color);
    level[x][y].durability = another.durability;
}

qint16 &tileType(_map &level, int x, int y)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw "Exception: tile does not exist.";
    return level[x][y].type;
}

char *tileColor(_map &level, int x, int y)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw "Exception: tile does not exist.";
    return level[x][y].color;
}

qint16 &tileDurability(_map &level, int x, int y)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw "Exception: tile does not exist.";
    return level[x][y].durability;
}

bool tileDealDamage(_map &level, int x, int y, int d)
{ // returns true if tile has been destroyed
    if (x<0||x>=level.w||y<0||y>=level.h)throw "Exception: tile does not exist.";
    if (level[x][y].type == 1)
    {
        if (level[x][y].durability > d)
        {
            level[x][y].durability -= d;
            return false;
        }
        else
        {
            setTile(level, x, y, tile());
            return true;
        }
    } else
    {
        return false;
    }
}

/*int &tileMetadata(_map &level, int x, int y)
{
    if (x<0||x>=level.w||y<0||y>=level.h)throw "Exception: tile does not exist.";
    return level[x][y].metadata;
}*/

/*int tileTransparent(_map &level, int x, int y)
{
    try {
        if (tileType(level, x, y) == 0)
            return 1; // transparent
        if (tileType(level, x, y) == 3)
            return 2; // translucent
        else
            return 0; // opaque
    }
    catch (const char *ex) {
        //cerr << "[" << ticks << "] " << ex << "\n";
        return -1;
    }
    catch (...) {
        cerr << "Unknown exception in \'tile\' module!\n";
        return -1;
    }
}*/

bool tileSolid(_map &level, int x, int y)
{
    try {
        return (tileType(level, x, y) == 1 || tileType(level, x, y) == 2);
    }
    catch (const char *ex) {
        //cerr << "[" << ticks << "] " << ex << "\n";
        return true;
    }
    catch (...) {
        cerr << "Unknown exception in \'tile\' module!\n";
        return true;
    }
}

bool tileConnected(_map &level, int x, int y)
{
    return (tileSolid(level, x-1, y) ||
            tileSolid(level, x+1, y) ||
            tileSolid(level, x, y-1) ||
            tileSolid(level, x, y+1));
}
