//
//  PlatformDefine.h
//  foundation
//
//  Created by Federico Spadoni on 24/08/17.
//
//

#ifndef PlatformDefine_h
#define PlatformDefine_h

#include "PlatformConfig.h"

#if FOUNDATION_TARGET_PLATFORM == FOUNDATION_PLATFORM_MAC
#include "mac/PlatformDefine-mac.h"
#elif FOUNDATION_TARGET_PLATFORM == FOUNDATION_PLATFORM_LINUX
#include "linux/PlatformDefine-linux.h"
#elif FOUNDATION_TARGET_PLATFORM == FOUNDATION_PLATFORM_WIN32
#include "win32/PlatformDefine-win32.h"
#elif FOUNDATION_TARGET_PLATFORM == FOUNDATION_PLATFORM_ANDROID
#include "android/PlatformDefine-android.h"
//#elif FOUNDATION_TARGET_PLATFORM == CC_PLATFORM_IOS
//#include "ios/PlatformDefine-ios.h"
#endif

#endif /* PlatformDefine_h */
