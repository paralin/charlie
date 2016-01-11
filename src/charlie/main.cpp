#include <iostream>
#include <Logging.h>
#include <charlie/System.h>
#include <boost/process.hpp>

using namespace boost::process;
using namespace boost::process::initializers;

#if defined(_WIN32) && defined(CHARLIE_ENABLE_GUI)
#include <windows.h>
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
#else
  int main( int argc, const char* argv[] ) {
#endif
    CLOG("Starting up...");
    System *system = new System();
    int r = system->main(argc, argv);
    bool startSubprocess = system->startSubprocess;
    std::string subprocessPath = system->subprocessPath;
    std::vector<std::string> subprocessArgs = system->subprocessArgs;
    delete system;
    if(startSubprocess)
      execute(run_exe(subprocessPath), set_args(subprocessArgs));
    return r;
  };
