#pragma once
#ifndef TORC_H
#define TORC_H

#ifdef __cplusplus
extern "C" {
#endif

  void torc_main(int bindPort);
  void torc_shutdown();
  int have_completed_a_circuit();

#ifdef __cplusplus
}
#endif

#endif
