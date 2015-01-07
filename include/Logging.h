#pragma once

#define DO_CHARLIE_LOG

#ifdef DO_CHARLIE_LOG
#define CLOG(msg) \
  ( \
    (std::cout << msg << std::endl) \
  )
#define CERR(msg) \
  ( \
    (std::cerr << msg << std::endl) \
  )
#else
#define CLOG(msg) ;
#define CERR(msg) ;
#endif
