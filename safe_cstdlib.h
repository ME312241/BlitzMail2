// safe_cstdlib.h — Drop-in replacement for <cstdlib> that fixes the exit bug in VS 2022+
#ifndef SAFE_CSTDLIB_H
#define SAFE_CSTDLIB_H

// Force the old global names to exist (satisfies the using declarations)
#define _CRT_DECLARE_NONSTDC_NAMES 1

// Pre-declare the globals that <cstdlib> expects but doesn't get
#ifdef __cplusplus
extern "C" {
#endif
    void __cdecl exit(int status);
    void __cdecl abort(void);
    int __cdecl atexit(void (*func)(void));
#ifdef __cplusplus
}
#endif

// Now safely include the real header
#include <cstdlib>

#endif // SAFE_CSTDLIB_H