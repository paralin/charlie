#include <cserver/ServerModuleInstance.h>
#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <cserver/networking/CharlieClient.h>
#include <SModuleInterface.h>

#ifndef CHARLIE_MODULE
#ifndef IS_YCM
#define MLOG(msg) CLOG("["<<this->filename<<"] "<<msg);
#define MERR(msg) CERR("["<<this->filename<<"]! "<<msg);
#endif
#endif
#define EMPTYCATCH(sect) try{sect;}catch(...){}

ServerModuleInstance::ServerModuleInstance(std::string path, CharlieClient* cli)
{
  filename = boost::filesystem::path(path).filename().string();
  client = cli;
  libPath = path;
  baseModule = NULL;
  mainThread = NULL;
  setStatus(charlie::MODULE_INIT);
}

ServerModuleInstance::~ServerModuleInstance()
{
  unload();
  client = NULL;
}

void ServerModuleInstance::unload()
{
  bool wasShutdown = false;
  if(mainThread != NULL)
  {
    if(baseModule != NULL)
    {
      MLOG("Joining main thread...");
      EMPTYCATCH(baseModule->shutdown());
      wasShutdown = true;
      mainThread->interrupt();
      EMPTYCATCH(
      {
        if(mainThread->joinable()){
          boost::chrono::seconds sec(5);
          if(!mainThread->try_join_for(sec))
            MERR("Waited for 5 seconds but it hasn't exited.");
        }
      })
    }
    EMPTYCATCH(delete mainThread)
    mainThread = NULL;
  }
  if(baseModule != NULL)
  {
    try{
      //if(client != NULL)
      //  client->onModuleReleased(module->id());
      if (!wasShutdown)
        EMPTYCATCH(baseModule->shutdown())
      delete baseModule;
      MLOG("Deleted baseModule");
    }catch(std::exception& e)
    {
      MERR("Unable to delete module, might be a memory leak.");
      MERR(e.what());
    }
    baseModule = NULL;
  }
  if(gmod != NULL)
  {
    if(!g_module_close(gmod))
    {
      MERR("Unable to unload the module, maybe memory leak.");
    }else{
      MLOG("Released library file.");
    }
    gmod = NULL;
  }
  setStatus(charlie::MODULE_INIT);
}

bool ServerModuleInstance::load()
{
  if(inst.status() != charlie::MODULE_INIT)
  {
    MLOG("load() called when already loaded (status "<<inst.status()<<")..");
    return false;
  }

  setStatus(charlie::MODULE_LOADING);

  //Check if file exists
  if(!boost::filesystem::exists(libPath.c_str()))
  {
    MERR("Library doesn't exist: "<<libPath);
    setStatus(charlie::MODULE_INIT);
    return false;
  }

  MLOG("Attempting to load gmodule...");
  ConstructFunc construct = NULL;
  try {
    //XXX: this actually tries to append library suffixes and such. so just try to read the module id.
    gmod = g_module_open(libPath.c_str(), G_MODULE_BIND_LAZY);
  } catch (int ex)
  {
    MERR("Module load exception " << ex << " caught...");
    MERR(g_module_error());
    setStatus(charlie::MODULE_INIT);
    return false;
  }

  if(gmod == NULL)
  {
    MERR("GModule Open returned null...");
    MERR("Path: "<<libPath);
    MERR(g_module_error());
    setStatus(charlie::MODULE_INIT);
    return false;
  }

  if(g_module_symbol(gmod, "cmconstr", (gpointer*) &construct) == FALSE) {
    MERR("Can't find the cmconstr function.");
    g_module_close(gmod);
    setStatus(charlie::MODULE_INIT);
    gmod = NULL;
    return false;
  }

  server_modules::ServerModule* ninst = NULL;

  EMPTYCATCH(ninst = construct(););

  if(ninst == NULL)
  {
    MERR("Problem constructing the module.");
    g_module_close(gmod);
    gmod = NULL;
    setStatus(charlie::MODULE_INIT);
    return false;
  }

  setStatus(charlie::MODULE_LOADED);
  baseModule = ninst;

  EMPTYCATCH(ninst->setModuleInterface((server_modules::SModuleInterface*) client));

  MLOG("Loaded module successfully.");
  inst.set_id(ninst->getModuleId());

  // Inject all the other modules
  for (auto t : client->modules)
    if (t->status() >= charlie::MODULE_LOADED && t->inst.id() != inst.id())
      EMPTYCATCH(ninst->inject(t->inst.id(), t->baseModule));

  MLOG("Starting module thread...");
  try{
    mainThread = new boost::thread(&server_modules::ServerModule::module_main, ninst);
  }
  catch(...)
  {
    MERR("Error when starting module thread.");
  }
  setStatus(charlie::MODULE_LOADED_RUNNING);

  return true;
}

inline void ServerModuleInstance::setStatus(::charlie::EModuleStatus value)
{
  if(inst.status() == value) return;
  MLOG("status => "<<value);
  inst.set_status(value);
  // client->moduleStatusChanged(inst.id(), value);
}

charlie::EModuleStatus ServerModuleInstance::status()
{
  return inst.status();
}


void ServerModuleInstance::notifyModuleReleased(u32 id)
{
  if(baseModule != NULL)
    EMPTYCATCH(baseModule->release(id););
}

void ServerModuleInstance::notifyModuleLoaded(u32 id, void* ptr)
{
  if(baseModule != NULL)
    EMPTYCATCH(baseModule->inject(id, ptr););
}
