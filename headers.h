#ifndef _WINDLL
#include <dlfcn.h>
#else
#include <Windows.h>
#endif
#include <float.h>
#include <iostream>
#ifndef _WINDLL
#include <link.h>
#endif
#include <string.h>
#include <string>
#ifndef _WINDLL
#include <sys/mman.h>
#include <unistd.h>
#endif
#include <vector>
#include <assert.h>
#include <sys/stat.h>

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t byte;
typedef void* ptr_t;
typedef unsigned char BYTE;
typedef int QueryCvarCookie_t;
