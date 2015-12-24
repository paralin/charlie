#define MODULE_ID 6032034
#include <modules/torm/Torm.h>
#include <modules/torm/TorC.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

using namespace modules::torm;

TormModule::TormModule() :
  mInter(NULL),
  pInter(new TormInter(this)),
  running(true)
{
  MLOG("Torm module constructed...");
}

TormModule::~TormModule()
{
  delete pInter;
}

void TormModule::shutdown()
{
  MLOG("Shutting down torm module..");
  running = false;
  torc_shutdown();
}

void TormModule::setModuleInterface(ModuleInterface* inter)
{
  mInter = inter;
}

void* TormModule::getPublicInterface()
{
  return pInter;
}

void TormModule::injectDependency(u32 id, void* dep)
{
}

void TormModule::releaseDependency(u32 id)
{
}

bool TormModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void TormModule::module_main()
{
  parseModuleInfo();
  {
    SystemInfo *info = mInter->getSysInfo();
    torPort = info->lock_port + 1 + (rand() % 100);
  }
  connectControlLoop = boost::thread(&TormModule::connectControl, this);
  torc_main(torPort);
}

void TormModule::connectControl()
{
  MLOG("Starting connect control loop...");
  bool justCompleted = false;
  while (running)
  {
    if (!have_completed_a_circuit())
    {
      MLOG("Waiting for Tor to finish starting up...");
      justCompleted = true;
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
      continue;
    }
    if (justCompleted)
    {
      MLOG("Tor became ready, attempting connections...");
      justCompleted = false;
    }
  }
}

void TormModule::handleEvent(u32 eve, void* data)
{
}

TormInter::TormInter(TormModule * mod)
{
  this->mod = mod;
}

bool TormInter::ready()
{
  return false;
}

void TormInter::disconnectInvalid()
{
}

tcp::socket* TormInter::getSocket()
{
  return NULL;
}

TormInter::~TormInter()
{
}

void TormInter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  MERR("Don't know how to handle emsg " << targ.emsg() << ".");
}

CHARLIE_CONSTRUCT(TormModule);
