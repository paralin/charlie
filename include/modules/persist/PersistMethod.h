#pragma once
#include <ModuleInterface.h>

// Describes a way of persisting
namespace modules
{
  namespace persist
  {
    class PersistModule;
    class VISIBLE PersistMethod
    {
      public:
        virtual ~PersistMethod() {};

        // Passes in the pointers to various things
        virtual void init(ModuleInterface *inter, PersistModule* persist) = 0;

        // Checks if this method is currently in use
        virtual bool inUse() = 0;

        // Checks if this method can be used on this system
        virtual bool canUse() = 0;

        // Initializes this persist method. Including call to manager -> prepare to move.
        // Expected to cleanup appropriately if failed.
        virtual bool setup() = 0;

        // Assuming currently in use, this will remove traces of this method.
        virtual void cleanup() = 0;
    };
  }
}

