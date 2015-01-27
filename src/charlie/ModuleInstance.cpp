#include <charlie/ModuleInstance.h>
#include <boost/filesystem.hpp>

#define MLOG(msg) CLOG("["<<module->id()<<"i] "<<msg);
#define MERR(msg) CERR("["<<module->id()<<"i]! "<<msg);
#define EMPTYCATCH(sect) try{sect}catch(...){}

ModuleInstance::ModuleInstance(charlie::CModule* mod, std::string path, ModuleManager* man)
{
  this->module = mod;
  this->libPath = path;
  this->baseModule = NULL;
  this->mManager = man;
  this->mInter = new modules::ModuleInterface(man, this);
  setStatus(charlie::MODULE_INIT);
}

ModuleInstance::~ModuleInstance()
{
  unload();
  mManager = NULL;
  delete mInter;
}

void ModuleInstance::unload()
{
  if(baseModule != NULL)
  {
    try{
      EMPTYCATCH(baseModule->shutdown(););
      delete baseModule; MLOG("Deleted baseModule");
    }catch(...)
    {
      MERR("Unable to delete module, might be a memory leak.");
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

bool ModuleInstance::load()
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
  gmod = g_module_open(libPath.c_str(), G_MODULE_BIND_LAZY);

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

  modules::Module* ninst = NULL;

  EMPTYCATCH(ninst = construct(););

  if(ninst == NULL)
  {
    MERR("Problem constructing the module.");
    g_module_close(gmod);
    gmod = NULL;
    setStatus(charlie::MODULE_INIT);
    return false;
  }

  setStatus(charlie::MODULE_LOADED_INACTIVE);
  baseModule = ninst;

  EMPTYCATCH(ninst->setModuleInterface(mInter););

  MLOG("Loaded module successfully.");
  return true;
}

inline void ModuleInstance::setStatus(::charlie::EModuleStatus value)
{
  if(inst.status() == value) return;
  MLOG("status => "<<value);
  inst.set_status(value);
}
