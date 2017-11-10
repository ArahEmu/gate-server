#ifndef SERVERCLOCK_H
#define SERVERCLOCK_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include "defines.h"

#ifndef _WIN32
#   include <unistd.h>
#endif

#include <stdio.h>
#include <time.h>

class ServerClock
{
public:
    ServerClock(unsigned int rate);
public:
    void Start();
    bool InFrame();
    void Frame();
    void SetRate(unsigned int rate);
private:
    timespec m_cTime;
    timespec m_pTime;
    long long m_uDelta;
public:
    int m_Frames;
    long m_FrameDelta;
    unsigned int m_Rate;
};

#endif // SERVERCLOCK_H
