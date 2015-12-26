#pragma once
#ifndef TORC_H
#define TORC_H

#ifdef __cplusplus
extern "C" {
#endif

  void torc_main(int bindPort, const char* socksUsername, const char* socksPassword, const char* dataDir);
  void torc_shutdown();
  void torc_new_identity();
  int have_completed_a_circuit();

#ifdef __cplusplus
}
#endif

#endif
