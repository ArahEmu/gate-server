#include "text.h"

text::text()
{

}

int text::GetTextLengthNL(const char *Line, int Max)
{
    for (int i = 0; i < Max; i++) {
        if (Line[i] == '\n' || Line[i] == '\r') {
            return i;
        }
    }
    return -1;
}

const char *text::MoveToNextLine(const char *Line, int Max)
{
    const char* Ptr = Line;
    for (int i = 0; i < Max; i++) {
        if (Line[i] == '\n' || Line[i] == '\r') {
            Ptr++;
        } else {
            return Ptr;
        }
    }
    return 0;
}

void text::PrintHex(const unsigned char *Input, int InputLength)
{
    for (auto i = 0; i < InputLength; i++) {
        printf("%02X, ", Input[i]);
    }
}
