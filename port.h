#ifndef PORT_H
#define PORT_H

#ifdef _WIN32
struct timespec;
int clock_gettime(int X, timespec *tv);
#endif

#endif // PORT_H
