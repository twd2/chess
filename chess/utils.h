#ifndef UTILS_H
#define UTILS_H

#include <QObject>

inline quint32 min(const quint32 &a, const quint32 &b)
{
    return a < b ? a : b;
}

#ifndef GAME_PORT
#define GAME_PORT 23334
#endif

#endif // UTILS_H
