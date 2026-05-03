// pch.h
//
// Precompiled header used by the test project.  Including Windows.h
// here allows the compiler to build a precompiled representation of
// the Windows API which speeds up subsequent compilations.  You can
// add other headers that rarely change here as well.

#pragma once

// Exclude rarely-used services from Windows headers.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
