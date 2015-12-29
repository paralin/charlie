#pragma once
#ifndef ISESSIONCONTROLLER_H
#define ISESSIONCONTROLLER_H

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <string>

class CharlieSession;
class ISessionController
{
public:
  ~ISessionController() {};
  virtual void handleMessage(charlie::CMessageHeader& head, std::string& body) = 0;
  virtual void onDisconnected() = 0;
  virtual void onHandshakeComplete() = 0;
};

#endif
