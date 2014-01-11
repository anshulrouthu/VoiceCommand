/*
 * utils.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: anshul
 */
#include "utils.h"
#include <stdarg.h>
int kbhit(void) {
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);

}

vcDebug::vcDebug(int level):m_level(level)
{
}

int vcDebug::DebugPrint(const char* formatString, ...)
{
    if (isDebugOn())
    {
        va_list argumentList;
        va_start(argumentList, formatString);
        vprintf(formatString, argumentList);
        va_end(argumentList);
    }

}

bool vcDebug::isDebugOn()
{
    return (m_level <= g_dbglevel);
}
