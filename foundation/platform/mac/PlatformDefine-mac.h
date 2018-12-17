//
//  PlatformDefine_mac.h
//  foundation
//
//  Created by Federico Spadoni on 24/08/17.
//
//

#ifndef PlatformDefine_mac_h
#define PlatformDefine_mac_h

#include "PlatformConfig.h"

#ifdef _USRDLL
#define FoundationDLL __attribute__ ((visibility("default")))
#else
#define FoundationDLL
#endif

#endif /* PlatformDefine_mac_h */
