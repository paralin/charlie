#include <cserver/System.h>
#include <charlie/random.h>
#include <cserver/NetHost.h>
#include <cserver/networking/CharlieClient.h>
#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>
#include <Logging.h>
#include <cserver/ModuleTable.h>
#include <openssl/md5.h>
#include <limits.h>
#include <charlie/xor.h>

#define ALLOWED_TIME_SKEW 30

CharlieClient::CharlieClient(std::shared_ptr<tcp::socket>& socket, NetHost* host)
  : socket(socket),
  host(host)
{
}

void CharlieClient::start()
{
  CLOG("Starting client...");
  session = std::make_shared<CharlieSession>(socket, this, host->sys->privateKey);
  session->start();
}

void CharlieClient::send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data)
{
  session->send(targetModule, targetEmsg, jobid, data);
}

void CharlieClient::send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target)
{
  session->send(emsg, data, target);
}

void CharlieClient::handleMessage(charlie::CMessageHeader& head, std::string& data)
{
  switch (head.emsg())
  {
    case charlie::EMsgFailure:
      handleFailure(data);
      break;
    case charlie::EMsgRoutedMessage:
      handleRoutedMessage(head, data);
      break;
    case charlie::EMsgKeepalive:
      break;
    default:
      CERR("Unrecognized emsg " << head.emsg());
      session->disconnect();
      return;
  }
}

void CharlieClient::handleRoutedMessage(charlie::CMessageHeader& head, std::string& data)
{
  if (!head.has_target())
  {
    CERR("Routed message without a target!");
    session->disconnect();
    return;
  }
  charlie::CMessageTarget* target = head.mutable_target();
  if (!target->has_target_module())
  {
    CERR("Routed message without a target.target_module!");
    session->disconnect();
    return;
  }
  for (auto m : modules)
  {
    if (m->inst.id() == target->target_module())
    {
      try
      {
        m->baseModule->handleMessage(target, data);
      } catch (std::exception& ex)
      {
        CERR("Error thrown by module message handler: " << ex.what());
      }
      return;
    }
  }

  CERR("Routed message target " << target->target_module() << " not found.");
  sendDeliveryFailure();
}

void CharlieClient::sendDeliveryFailure()
{
  charlie::CNetFailure fail;
  fail.set_fail_type(charlie::FAILURE_MODULE_NOTFOUND);
  fail.set_error_message("Unable to find that module.");
  std::string data = fail.SerializeAsString();
  // Maybe set the target later
  send(charlie::EMsgFailure, data);
}

void CharlieClient::handleFailure(std::string& data)
{
  charlie::CNetFailure fail;
  if (!fail.ParseFromString(data))
  {
    CERR("Unable to parse failure.");
    return;
  }

  CERR("Failure notification from client, " << fail.fail_type());
}

void CharlieClient::sendInitData()
{
  // Generate a module table
  sendModuleTable();
}

void CharlieClient::sendModuleTable()
{
  charlie::CModuleTable* table = host->sys->generateModuleTableFromFile();
  if (table == NULL)
  {
    CERR("Problem generating new module table for client.");
    return;
  }
  charlie::CSignedBuffer* sbuf = new charlie::CSignedBuffer();
  sbuf->set_data(table->SerializeAsString());
  delete table;
  updateSignedBuf(sbuf, host->sys->crypt);
  modules::manager::CModuleTableUpdate upd;
  upd.set_allocated_buf(sbuf);
  std::string data = upd.SerializeAsString();
  send(MANAGER_MODULE_ID, modules::manager::EManagerEMsg_ModuleTableUpdate, 0, data);
}

void CharlieClient::onHandshakeComplete()
{
  sendInitData();

  CLOG("Initializing server modules.");
  modules = host->sys->buildModuleSet(this);
  for (auto m : modules)
    m->load();
}

void CharlieClient::onDisconnected()
{
  host->clientDisconnected(shared_from_this());
}

CharlieClient::~CharlieClient()
{
  CLOG("Deconstructing client...");
}
