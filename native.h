// Determine whether we're in 32 or 64bit mode

#ifndef __NATIVE_H
#define __NATIVE_H

#if !defined(ENV64BIT) && !defined(ENV32BIT)
  // Check windows
  #if _WIN32 || _WIN64
    #if _WIN64
      #define ENV64BIT
    #else
      #define ENV32BIT
    #endif
  #endif

  // Check GCC
  #if __GNUC__
    #if __x86_64__ || __ppc64__
      #define ENV64BIT
    #else
      #define ENV32BIT
    #endif
  #endif
#endif

#ifdef ENV64BIT
  #define NUMTYPE long
  #define int long
#else
  #define NUMTYPE int
#endif

#endif
