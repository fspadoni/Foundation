//
//  PlatformConfig.h
//  foundation
//
//  Created by Federico Spadoni on 24/08/17.
//
//

#ifndef PlatformConfig_h
#define PlatformConfig_h

/**
 Config of project, per target platform.
 
 THIS FILE MUST NOT INCLUDE ANY OTHER FILE
 */

//////////////////////////////////////////////////////////////////////////
// pre configure
//////////////////////////////////////////////////////////////////////////

// define supported target platform macro which CC uses.
#define FOUNDATION_PLATFORM_UNKNOWN            0
#define FOUNDATION_PLATFORM_MAC                1
#define FOUNDATION_PLATFORM_LINUX              2
#define FOUNDATION_PLATFORM_WIN32              3
#define FOUNDATION_PLATFORM_ANDROID            4
//#define FOUNDATION_PLATFORM_IOS                5


// Determine target platform by compile environment macro.
#define FOUNDATION_TARGET_PLATFORM             FOUNDATION_PLATFORM_UNKNOWN

// mac
#if defined(__APPLE__)
#undef  FOUNDATION_TARGET_PLATFORM
#define FOUNDATION_TARGET_PLATFORM         FOUNDATION_PLATFORM_MAC
#endif

// linux
#if defined(LINUX) && !defined(__APPLE__)
#undef  FOUNDATION_TARGET_PLATFORM
#define FOUNDATION_TARGET_PLATFORM         FOUNDATION_PLATFORM_LINUX
#endif

// win32
#if defined(_WIN32) && defined(_WINDOWS)
#undef  FOUNDATION_TARGET_PLATFORM
#define FOUNDATION_TARGET_PLATFORM         FOUNDATION_PLATFORM_WIN32
#endif

// android
#if defined(ANDROID)
#undef  FOUNDATION_TARGET_PLATFORM
#define FOUNDATION_TARGET_PLATFORM         FOUNDATION_PLATFORM_ANDROID
#endif

// iphone
//#if defined(TARGET_OS_IPHONE)
//#undef  FOUNDATION_TARGET_PLATFORM
//#define FOUNDATION_TARGET_PLATFORM         FOUNDATION_PLATFORM_IOS
//#endif



//////////////////////////////////////////////////////////////////////////
// post configure
//////////////////////////////////////////////////////////////////////////

// check user set platform
#if ! FOUNDATION_TARGET_PLATFORM
#error  "Cannot recognize the target platform; are you targeting an unsupported platform?"
#endif


#endif /* PlatformConfig_h */
