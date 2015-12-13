#pragma once

#include <string>
#include <ostream>
#include <iostream>

#ifdef DEBUG
#ifndef DO_CHARLIE_LOG
#define DO_CHARLIE_LOG
#endif
#endif

#ifdef IS_YCM
#undef DO_CHARLIE_LOG
#define MLOG(msg) ;
#define MERR(msg) ;
#endif

#ifdef DO_CHARLIE_LOG
#define CLOG(msg) \
  ( \
    (std::cout << msg << std::endl << std::flush) \
  )
#define CERR(msg) \
  ( \
    (std::cerr << msg << std::endl << std::flush) \
  )
#else
#define CLOG(msg) ;
#define CERR(msg) ;
#endif
