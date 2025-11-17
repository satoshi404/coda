#ifndef __debug_h__
#define __debug_h__

#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <pipe.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    DEBUG_LEVEL_TRACE = 0,
    DEBUG_LEVEL_DEBUG = 1,
    DEBUG_LEVEL_INFO  = 2,
    DEBUG_LEVEL_WARN  = 3,
    DEBUG_LEVEL_ERROR = 4,
    DEBUG_LEVEL_FATAL = 5
} DebugLevel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ANSI color codes for terminal output
#if PIPE_LINUX
    #define COLOR_RESET   "\033[0m"
    #define COLOR_RED     "\033[31m"
    #define COLOR_GREEN   "\033[32m"
    #define COLOR_YELLOW  "\033[33m"
    #define COLOR_BLUE    "\033[34m"
    #define COLOR_MAGENTA "\033[35m"
    #define COLOR_CYAN    "\033[36m"
    #define COLOR_WHITE   "\033[37m"
    #define COLOR_GRAY    "\033[90m"
#else
    #define COLOR_RESET   ""
    #define COLOR_RED     ""
    #define COLOR_GREEN   ""
    #define COLOR_YELLOW  ""
    #define COLOR_BLUE    ""
    #define COLOR_MAGENTA ""
    #define COLOR_CYAN    ""
    #define COLOR_WHITE   ""
    #define COLOR_GRAY    ""
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Global debug level (can be changed at runtime)
static DebugLevel g_debugLevel = DEBUG_LEVEL_TRACE;

// Set minimum debug level
static inline void debug_setLevel(DebugLevel level) {
    g_debugLevel = level;
}

// Get current debug level
static inline DebugLevel debug_getLevel(void) {
    return g_debugLevel;
}

// Internal logging function
static inline void debug_log(DebugLevel level, const char* format, ...) {
    if (level < g_debugLevel) {
        return;
    }
    
    const char* levelStr = "";
    const char* color = COLOR_RESET;
    
    switch (level) {
        case DEBUG_LEVEL_TRACE:
            levelStr = "TRACE";
            color = COLOR_GRAY;
            break;
        case DEBUG_LEVEL_DEBUG:
            levelStr = "DEBUG";
            color = COLOR_CYAN;
            break;
        case DEBUG_LEVEL_INFO:
            levelStr = "INFO";
            color = COLOR_GREEN;
            break;
        case DEBUG_LEVEL_WARN:
            levelStr = "WARN";
            color = COLOR_YELLOW;
            break;
        case DEBUG_LEVEL_ERROR:
            levelStr = "ERROR";
            color = COLOR_RED;
            break;
        case DEBUG_LEVEL_FATAL:
            levelStr = "FATAL";
            color = COLOR_MAGENTA;
            break;
    }
    
    fprintf(stderr, "%s[%s]%s ", color, levelStr, COLOR_RESET);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
    fflush(stderr);
    
    // Exit on fatal error
    if (level == DEBUG_LEVEL_FATAL) {
        abort();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Convenience macros
#define LOG_TRACE(...) debug_log(DEBUG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) debug_log(DEBUG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  debug_log(DEBUG_LEVEL_INFO,  __VA_ARGS__)
#define LOG_WARN(...)  debug_log(DEBUG_LEVEL_WARN,  __VA_ARGS__)
#define LOG_ERROR(...) debug_log(DEBUG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) debug_log(DEBUG_LEVEL_FATAL, __VA_ARGS__)

// Assert macro
#ifdef NDEBUG
    #define ASSERT(condition, ...) ((void)0)
#else
    #define ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                LOG_FATAL("Assertion failed: %s, file %s, line %d: " __VA_ARGS__, \
                         #condition, __FILE__, __LINE__); \
            } \
        } while (0)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Static assert (compile-time)
#define STATIC_ASSERT(condition, message) \
    typedef char static_assert_##message[(condition) ? 1 : -1]

// Check macros
#define CHECK_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            LOG_ERROR("NULL pointer check failed: %s, file %s, line %d", \
                     #ptr, __FILE__, __LINE__); \
            return 0; \
        } \
    } while (0)

#define CHECK_NULL_RET(ptr, ret) \
    do { \
        if ((ptr) == NULL) { \
            LOG_ERROR("NULL pointer check failed: %s, file %s, line %d", \
                     #ptr, __FILE__, __LINE__); \
            return ret; \
        } \
    } while (0)

// Memory debugging
#ifdef DEBUG_MEMORY
    #define DEBUG_ALLOC(size) \
        do { \
            LOG_TRACE("Allocating %zu bytes at %s:%d", (size_t)(size), __FILE__, __LINE__); \
        } while (0)
    
    #define DEBUG_FREE(ptr) \
        do { \
            LOG_TRACE("Freeing memory at %p from %s:%d", (ptr), __FILE__, __LINE__); \
        } while (0)
#else
    #define DEBUG_ALLOC(size) ((void)0)
    #define DEBUG_FREE(ptr) ((void)0)
#endif

// Performance timing
#if PIPE_LINUX
    #include <time.h>
    #define PERF_START() \
        struct timespec _perf_start, _perf_end; \
        clock_gettime(CLOCK_MONOTONIC, &_perf_start)
    
    #define PERF_END(name) \
        do { \
            clock_gettime(CLOCK_MONOTONIC, &_perf_end); \
            double _elapsed = (_perf_end.tv_sec - _perf_start.tv_sec) * 1000.0; \
            _elapsed += (_perf_end.tv_nsec - _perf_start.tv_nsec) / 1000000.0; \
            LOG_DEBUG("Performance [%s]: %.3f ms", name, _elapsed); \
        } while (0)
#elif PIPE_WINDOWS
    #include <windows.h>
    #define PERF_START() \
        LARGE_INTEGER _perf_freq, _perf_start, _perf_end; \
        QueryPerformanceFrequency(&_perf_freq); \
        QueryPerformanceCounter(&_perf_start)
    
    #define PERF_END(name) \
        do { \
            QueryPerformanceCounter(&_perf_end); \
            double _elapsed = (_perf_end.QuadPart - _perf_start.QuadPart) * 1000.0 / _perf_freq.QuadPart; \
            LOG_DEBUG("Performance [%s]: %.3f ms", name, _elapsed); \
        } while (0)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// OpenGL error checking
#ifdef DEBUG_OPENGL
    #include <GL/gl.h>
    static inline void debug_checkGLError(const char* file, int line) {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            const char* error = "Unknown error";
            switch (err) {
                case GL_INVALID_ENUM:      error = "GL_INVALID_ENUM"; break;
                case GL_INVALID_VALUE:     error = "GL_INVALID_VALUE"; break;
                case GL_INVALID_OPERATION: error = "GL_INVALID_OPERATION"; break;
                case GL_OUT_OF_MEMORY:     error = "GL_OUT_OF_MEMORY"; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: error = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            LOG_ERROR("OpenGL Error: %s at %s:%d", error, file, line);
        }
    }
    #define CHECK_GL_ERROR() debug_checkGLError(__FILE__, __LINE__)
#else
    #define CHECK_GL_ERROR() ((void)0)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PIPE_WINDOWS && COMPILER_MSVC
    #define DEBUG_BREAK() __debugbreak()
#elif PIPE_LINUX && (COMPILER_GCC || COMPILER_CLANG)
    #define DEBUG_BREAK() __builtin_trap()
#else
    #include <signal.h>
    #define DEBUG_BREAK() raise(SIGTRAP)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* __debug_h__ */