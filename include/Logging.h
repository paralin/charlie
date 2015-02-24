#pragma once

#include <string>
#include <ostream>
#include <iostream>

#ifndef NDEBUG
#define DO_CHARLIE_LOG
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
