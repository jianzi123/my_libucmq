#include "kv_inner.h"


#define DIE abort

void logicError(const char* fmt, ...)
{
    if (!fmt) {
        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);
    }

    DIE();
}

void logicErrorExpr(int expr, const char* fmt, ...)
{
    if (!expr) {
        logicError(fmt);
    }
}
