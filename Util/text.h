#ifndef TEXT_H
#define TEXT_H

/*
 * GW2SEX - Guild Wars 2 Server Emulator For xnix
 *
 * Developed by Nomelx
 * */

#include <stdio.h>

#define T_PREF_LENGTH 256

class text
{
private:
    text();
public:
    static int GetTextLengthNL(const char* Line, int Max);
    static const char* MoveToNextLine(const char* Line, int Max);
public:
    static void PrintHex(const unsigned char* Input, int InputLength);
};

#endif // TEXT_H
