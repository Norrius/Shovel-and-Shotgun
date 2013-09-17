/****************************************************************
 * This module contains epic functions
 * that i don't know where to put
 ****************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

// useful...
void toggle(bool &a);
template <class T>
void cap(T &i, T a, T b)
{
    if (i<a)
        i = a;
    if (i>b)
        i = b;
}
template <class T, class U>
void pour(T &src, U amount, T &dest, U destlimit);
double myRand(double l, double r);
int myRand(int l, int r);

// ...and useless ones
/*double abs(double a);
double max(double a, double b);
double roundTo(double value, double base);*/

#endif // GLOBAL_H
