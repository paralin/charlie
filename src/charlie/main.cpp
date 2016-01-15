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
  int main( int argc, const char* argv[] )
#endif
{
    CLOG("Starting up...");
    System *sys = new System();
#if defined(_WIN32) && defined(CHARLIE_ENABLE_GUI)
    char n_argv[1][MAX_PATH];
    if (GetModuleFileName(NULL, n_argv[0], MAX_PATH) == 0)
    {
      CERR("Unable to discover our module path.");
      n_argv[0][0] = (char) NULL;
    }

    int r = sys->main(1, (const char**) n_argv);
#else
    int r = sys->main(argc, argv);
#endif
    bool startSubprocess = sys->startSubprocess;
    std::string subprocessPath = sys->subprocessPath;
    std::vector<std::string> subprocessArgs = sys->subprocessArgs;
    delete sys;
    if(startSubprocess)
      execute(run_exe(subprocessPath), set_args(subprocessArgs));
    return r;
  };
