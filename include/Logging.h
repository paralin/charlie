#pragma once

#define DO_CHARLIE_LOG

#ifdef DO_CHARLIE_LOG
#define CLOG(msg) \
  ( \
    (std::cout << msg << std::endl) \
  )
#else
#define CLOG(msg) ;
#endif
