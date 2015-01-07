#include <iostream>
#include <Logging.h>
#include <charlie/System.h>

int main( int argc, const char* argv[] ) {
  CLOG("Starting up...");
  System *system = new System();
  int r = system->main(argc, argv);
  delete system;
  return r;
};
