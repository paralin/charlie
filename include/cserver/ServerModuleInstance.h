#pragma once

#include <Common.h>
#include <Logging.h>
#include <ServerModule.h>

#include <string>
#include <set>

#include <glib.h>
#include <gmodule.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <boost/thread.hpp>

typedef server_modules::ServerModule* (*ConstructFunc) (void);

class CharlieClient;
class ServerModuleInstance
{
public:
  ServerModuleInstance(std::string path, CharlieClient* cli);
  ~ServerModuleInstance();

  bool load();
  void unload();

  charlie::CModuleInstance inst;
  void notifyModuleReleased(u32 id);
  void notifyModuleLoaded(u32 mod, void* ptr);
  charlie::EModuleStatus status();

  server_modules::ServerModule* baseModule;

private:
  std::string libPath;
  inline void setStatus(::charlie::EModuleStatus value);
  GModule* gmod;
  CharlieClient* client;
  boost::thread* mainThread;
  std::string filename;
};
