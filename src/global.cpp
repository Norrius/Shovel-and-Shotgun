#include "global.h"
#include <stdlib.h>
#include <math.h>

void toggle(bool &a)
{
    a = a ? false : true;
}

template <class T, class U>
void pour(T &src, U amount, T &dest, U destlimit)
{
    T realamount = (src >= amount ? amount : src);
    if (destlimit < dest+realamount)
        realamount = destlimit - dest;
    src -= realamount;
    dest += realamount;
}

double myRand(double l, double r)
{
    return l + double(rand()) / RAND_MAX * (r - l);
}

int myRand(int l, int r)
{
    return (rand() % (r-l+1) + l);
}

/*double abs(double a)
{
    return a < 0 ? -a : a;
}

double max(double a, double b)
{
    return a > b ? a : b;
}

double roundTo(double value, double base)
{
    return floor((value + base / 2) / base) * base;
}
*/
