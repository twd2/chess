#ifndef SESSION_H
#define SESSION_H

#include "engine.h"

class Session
{
public:
    bool authorized = false;
    bool isPeer = false;
    bool isHttp = false;
    chess_t color = CH_VIEWER;
    Session();
};

#endif // SESSION_H
