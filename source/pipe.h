#ifndef __pipe_h__
#define __pipe_h__

/* Pipeline definitions */

#if defined(__linux__)
    #define PIPE_LINUX   ( 1 )
    #define PIPE_WINDOWS ( 0 )
    #define PIPE_NAME   "Linux"
    
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xutil.h>

    #include <linux/limits.h>
    #include <sys/types.h>
    #include <unistd.h>

#elif defined(_WIN32) || defined(_WIN64)
    #define PIPE_WINDOWS ( 1 )
    #define PIPE_LINUX   ( 0 )
    #define PIPE_NAME   "Windows"

    #include <windows.h>

    #include <shellapi.h>
    #include <shlobj.h>

#else
    #error "Unsupported platform"
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define COMPILER_MSVC 1
    #define COMPILER_GCC 0
    #define COMPILER_CLANG 0
#elif defined(__clang__)
    #define COMPILER_CLANG 1
    #define COMPILER_GCC 0
    #define COMPILER_MSVC 0
#elif defined(__GNUC__)
    #define COMPILER_GCC 1
    #define COMPILER_MSVC 0
    #define COMPILER_CLANG 0
#endif

// Function attributes
#if COMPILER_MSVC
    #define FORCEINLINE __forceinline
    #define NOINLINE __declspec(noinline)
    #define ALIGN(x) __declspec(align(x))
#else
    #define FORCEINLINE inline __attribute__((always_inline))
    #define NOINLINE __attribute__((noinline))
    #define ALIGN(x) __attribute__((aligned(x)))
#endif

// DLL export/import
#if PLATFORM_WINDOWS
    #ifdef BUILD_DLL
        #define API_EXPORT __declspec(dllexport)
    #else
        #define API_EXPORT __declspec(dllimport)
    #endif
#else
    #define API_EXPORT __attribute__((visibility("default")))
#endif

#endif /* __pipe_h__ */