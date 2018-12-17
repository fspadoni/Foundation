//
//  PlatformDefine-win32.h
//  foundation
//
//  Created by Federico Spadoni on 24/08/17.
//
//

#ifndef PlatformDefine_win32_h
#define PlatformDefine_win32_h

#include "PlatformConfig.h"


#if defined(_LIB)
#define FoundationDLL
#endif

#if defined(_USRDLL)
#define FoundationDLL     __declspec(dllexport)
#else         /* use a DLL library */
#define FoundationDLL     __declspec(dllimport)
#endif


#endif /* PlatformDefine_win32_h */
