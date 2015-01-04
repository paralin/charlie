#include <iostream>
#include <charlie/System.h>

int main( int argc, const char* argv[] ) {
#ifdef CHARLIE_DEBUG
  std::cout << "Starting up" << std::endl;
#endif
  System *system = new System();
  int r = system->main(argc, argv);
  delete system;
  return r;
};
