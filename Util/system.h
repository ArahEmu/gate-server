#ifndef SYSTEM_H
#define SYSTEM_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include "defines.h"

#include <stdio.h>
#include <signal.h>

#ifndef _WIN32
#   include <unistd.h>
#endif

#include <sys/types.h>

class System
{
private:
    System();
public:
    static void IgnoreSigPipe();
    static void CheckRoot();
    static void DebugDump(const char* Filename, const char* Data, int Length);
    static void DebugWriteMessage(const char* Filename, const char* Data, int Length);
};

#endif // SYSTEM_H
