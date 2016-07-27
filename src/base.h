
/*
// Copyright (c) 1998-2008 Joe Bertolami. All Right Reserved.
//
// base.h
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice, this
//     list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Additional Information:
//
//   For more information, visit http://www.bertolami.com.
*/

#ifndef __BASE_H__
#define __BASE_H__

/**********************************************************************************
//
// Platform definitions
//
**********************************************************************************/

#if defined (_WIN32) || defined (_WIN64)
    #include "windows.h"                                    
    #pragma warning (disable : 4244)                      // conversion, possible loss of data   
    #pragma warning (disable : 4018)                      // signed / unsigned mismatch
    #pragma warning (disable : 4996)                      // deprecated interfaces
    #pragma warning (disable : 4221)                      // empty translation unit
    #pragma warning (disable : 4273)                      // inconsistent linkage
    
    #define BASE_PLATFORM_WINDOWS                         // building a Windows application

#elif defined (__APPLE__)
    #include "TargetConditionals.h"
    #include "unistd.h"
    #include "sys/types.h"
    #include "ctype.h"

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define BASE_PLATFORM_IOS                         // building an application for iOS
    #elif TARGET_OS_MAC
        #define BASE_PLATFORM_MACOSX                      // building a Mac OSX application
    #endif
#else
    #error "Unsupported target platform detected."
#endif

/**********************************************************************************
//
// Debug definitions
//
**********************************************************************************/

#if defined (BASE_PLATFORM_WINDOWS)
    #ifdef _DEBUG
        #define BASE_DEBUG _DEBUG
    #elif defined (DEBUG)
        #define BASE_DEBUG DEBUG
    #endif
    #if defined(BASE_DEBUG) && !defined(debug_break)
        #define debug_break __debugbreak
    #endif
    #define __BASE_FUNCTION__  __FUNCTION__
#elif defined (BASE_PLATFORM_IOS) || defined (BASE_PLATFORM_MACOSX)
   #ifdef DEBUG
       #define BASE_DEBUG DEBUG
       #if !defined(debug_break)
           #define debug_break() __builtin_trap()
       #endif
    #endif
    #define __BASE_FUNCTION__ __func__
#endif

/**********************************************************************************
//
// Standard headers
//
**********************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

/**********************************************************************************
//
// Standard types
//
**********************************************************************************/

namespace base {

#if defined (BASE_PLATFORM_WINDOWS)
    typedef INT64 int64;		  
    typedef INT32 int32;		    
    typedef INT16 int16;		     
    typedef INT8  int8; 

    typedef UINT64 uint64;		      
    typedef UINT32 uint32;		        
    typedef UINT16 uint16;		        
    typedef UINT8  uint8;
#elif defined (BASE_PLATFORM_IOS) || defined (BASE_PLATFORM_MACOSX)
    typedef int64_t int64;		
    typedef int32_t int32;		
    typedef int16_t int16;		
    typedef int8_t  int8; 

    typedef u_int64_t uint64;	    
    typedef u_int32_t uint32;	    
    typedef u_int16_t uint16;	    
    typedef u_int8_t uint8;	 
#endif

typedef float float32;         
typedef double float64;           
typedef wchar_t wchar;

} // namespace base

#define BASE_KB       ((base::uint32) 1024)
#define BASE_MB       (BASE_KB*BASE_KB)
#define BASE_GB       (BASE_MB*BASE_KB)

/**********************************************************************************
//
// Status codes
//
**********************************************************************************/

namespace base { typedef uint8 status; }

#define BASE_SUCCESS                                 (0)
#define BASE_ERROR_INVALIDARG                        (1)
#define BASE_ERROR_NOTIMPL                           (2)
#define BASE_ERROR_OUTOFMEMORY                       (3)
#define BASE_ERROR_UNDEFINED                         (4)
#define BASE_ERROR_HARDWAREFAIL                      (5)
#define BASE_ERROR_INVALID_INDEX                     (6)
#define BASE_ERROR_CAPACITY_LIMIT                    (7)
#define BASE_ERROR_INVALID_RESOURCE                  (8)
#define BASE_ERROR_OPERATION_TIMEDOUT                (9)
#define BASE_ERROR_EXECUTION_FAILURE                 (10)
#define BASE_ERROR_PERMISSION_DENIED                 (11)
#define BASE_ERROR_IO_FAILURE                        (12)
#define BASE_ERROR_RESOURCE_UNREACHABLE              (13)
#define BASE_ERROR_SYSTEM_FAILURE                    (14)
#define BASE_ERROR_NOT_READY                         (15)
#define BASE_ERROR_OPERATION_COMPLETED               (16)
#define BASE_ERROR_RESOURCE_UNUSED                   (17)

#define base_succeeded(code)                         ((code) == BASE_SUCCESS)
#define base_failed(code)                            (!base_succeeded(code))

/**********************************************************************************
//
// Debug support
//
**********************************************************************************/

#ifdef BASE_DEBUG
    #define BASE_PARAM_CHECK (1)
    #define base_err(fmt, ...) do { printf("[BASE-ERR] "); \
                                    printf(fmt, ##__VA_ARGS__); \
                                    printf("\n"); debug_break(); \
                               } while(0)

    #define base_msg(fmt, ...) do { printf("[BASE-MSG] "); \
                                    printf(fmt, ##__VA_ARGS__); \
                                    printf("\n"); \
                               } while(0)
#else
    #define BASE_PARAM_CHECK (0)
    #define base_err(fmt, ...)                              
    #define base_msg(fmt, ...) do { printf("[BASE-MSG] "); \
                                    printf(fmt, ##__VA_ARGS__); \
                                    printf("\n"); \
                               } while(0)
#endif 

#define base_error_create_string(x) ((char *) #x)
#define base_post_error(x) post_error_i(x, base_error_create_string(x), __BASE_FUNCTION__, (char *) __FILE__, __LINE__)

namespace base {

inline uint32 post_error_i(uint8 error, const char *error_string, const char *function, const char *filename, uint32 line) 
{
#ifdef BASE_DEBUG      
    const char *path = filename;
    for (int32 i = (int32) strlen(filename); i >= 0; --i) 
    {
        if (filename[ i ] == '/') 
            break;

        path = &filename[i];    
    }

    base_err("*** RIP *** %s @ %s in %s:%i", error_string, function, path, line);
#endif
    return error;
}

} // namespace base

/**********************************************************************************
//
// Standard helpers
//
**********************************************************************************/

#define BASE_DISABLE_COPY_AND_ASSIGN(type) \
    type(const type &rvalue); \
    type &operator = (const type &rvalue);

#define BASE_TEMPLATE_T                         template <class T>
#define BASE_TEMPLATE_SPEC                      template <>

#define BASE_VARG(fmt)                          va_list argptr;                               \
                                                char text[1*BASE_KB] = {0};                   \
                                                va_start(argptr, fmt);                        \
                                                vsprintf(text, fmt, argptr);                  \
                                                va_end(argptr);    
#endif // __BASE_H__