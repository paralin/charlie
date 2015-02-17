#include <cserver/System.h>
#include <http/server.h>

System::System()
{
}

System::~System()
{
}

int System::main(int argc, const char* argv[])
{
  http::server::server s("localhost", "9921", ".");
  s.run();

  CLOG("Exiting...");
}

