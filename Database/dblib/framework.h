// This header centralises the inclusion of platform‑specific system headers.  On
// Windows the WIN32_LEAN_AND_MEAN macro reduces the amount of code pulled in
// from the Windows headers.  On non‑Windows platforms the windows.h header
// does not exist, so we avoid including it by wrapping in an appropriate
// preprocessor check.

#pragma once

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

// Windows Header Files
#ifdef _WIN32
#  include <windows.h>
#endif
