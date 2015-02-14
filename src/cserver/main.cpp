#include <iostream>
#include <Logging.h>
#include <cserver/System.h>

int main( int argc, const char* argv[] ) {
  CLOG("Charlie server starting up...");
  System *system = new System();
  int r = system->main(argc, argv);
  delete system;
  return r;
};
