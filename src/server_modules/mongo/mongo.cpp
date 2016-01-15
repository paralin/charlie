#include <server_modules/mongo/Mongo.h>
#include <boost/thread.hpp>

#define MONGO_FUNC_INIT \
  connMtx.lock(); \
  connMtx.unlock(); \
  boost::unique_lock<boost::mutex> lock(clientMtx); \
  std::string clientId = mInter->getClientId(); \
  ::mongo::BSONObj clientQuery = BSON("_id" << clientId);


using namespace server_modules::mongo;

bool MongoModule::mongoInited = false;

MongoModule::MongoModule() :
  mInter(NULL),
  connected(false),
  inited(false),
  isShutdown(false)
{
  MLOG("Mongo module constructed...");
  connMtx.lock();
}

void MongoModule::shutdown()
{
  if (connected)
    setOfflineState();
  isShutdown = true;
}

void MongoModule::setModuleInterface(SModuleInterface* inter)
{
  mInter = inter;
}

void MongoModule::inject(u32 id, void* dep)
{
}

void MongoModule::release(u32 id)
{
}

void MongoModule::handleEvent(u32 event, void* data)
{
}

void MongoModule::module_main()
{
  if (!mongoInited)
  {
    mongoInited = true;
    ::mongo::client::initialize();
  }

  std::string mongoUrl;

  if (const char* mongourl = std::getenv("MONGO_URL"))
  {
    mongoUrl = mongourl;
  } else
  {
    CERR("Please specify MONGO_URL in the env.");
    mongoUrl = "mongodb://localhost/charlie";
  }

  std::string errmsg;
  ::mongo::ConnectionString cs = ::mongo::ConnectionString::parse(mongoUrl, errmsg);

  if (!cs.isValid())
  {
    CERR("MONGO_URL specified is invalid: " << errmsg);
    mongoUrl = "mongodb://localhost/charlie";
    cs = ::mongo::ConnectionString::parse(mongoUrl, errmsg);
    dbname = "charlie";
  } else
  {
    dbname = cs.getDatabase();
    CLOG("MONGO_URL parsed, db: " << dbname);
  }

  CLOG("Attempting to connect...");
  try
  {
    conn.reset();
    conn = std::shared_ptr<::mongo::DBClientBase>(cs.connect(errmsg, 5000));
    if (!conn)
    {
      CERR("Unable to connect to mongo! " << errmsg);
      return;
    }
  } catch (::mongo::UserException& ex)
  {
    CERR("Unable to connect to mongo! " << ex.what());
    return;
  }

  CLOG("Connected to mongo.");
  connected = true;
  connMtx.unlock();
  clientsNs = dbname + ".clients";

  while (!inited && !isShutdown)
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
  if (isShutdown) return;

  while (!isShutdown)
  {
    // Update the lastOnline time
    boost::this_thread::sleep(boost::posix_time::seconds(5));
    setOnlineState();
  }
}

void MongoModule::setOnlineState()
{
  ::mongo::BSONObj stat = BSON(
      "offline" << false <<
      "lastOnline" << (int) std::time(NULL)
      );
  conn->update(clientsNs, BSON("_id" << mInter->getClientId()),
      BSON("$set" <<
        BSON(
          "status" << stat
          )
        )
      );
}

void MongoModule::setOfflineState()
{
  ::mongo::BSONObj stat = BSON(
      "offline" << true <<
      "lastOnline" << (int) std::time(NULL)
      );
  conn->update(clientsNs, BSON("_id" << mInter->getClientId()),
      BSON("$set" <<
        BSON(
          "status" << stat
          )
        )
      );
}

void MongoModule::initWithInfo(modules::client::CClientSystemInfo& info)
{
  MONGO_FUNC_INIT;

  // Search for existing
  std::shared_ptr<::mongo::DBClientCursor> curs = conn->query(clientsNs, clientQuery);
  ::mongo::BSONObj nfo = BSON(
      "system_id" << info.system_id() <<
      "cpu_hash" << info.cpu_hash() <<
      "hostname" << info.hostname()
      );
  ::mongo::BSONObj stat = BSON(
      "offline" << false <<
      "lastOnline" << (int) std::time(NULL)
      );

  if (!curs->more())
  {
    CLOG("This is a new client, inserting record.");
    ::mongo::BSONObj obj = BSON(
        "_id" << clientId <<
        "info" << nfo <<
        "status" << stat
        );
    conn->insert(clientsNs, obj);
  }
  else
  {
    CLOG("Updating client info.");
    conn->update(clientsNs, clientQuery,
        BSON(
          "$set" << BSON(
            "info" << nfo <<
            "status" << stat
            )
          )
        );
  }
  inited = true;
}

void MongoModule::updateModuleStates(modules::client::CClientModuleState& info)
{
  MONGO_FUNC_INIT;

  ::mongo::BSONObjBuilder bld;
  for (int i = 0; i < info.modules_size(); i++)
  {
    const auto& m = info.modules(i);
    bld.append(std::to_string(m.id()), m.status());
  }

  conn->update(clientsNs, clientQuery,
      BSON(
        "$set" << BSON(
          "modules" << bld.obj()
          )
        )
      );
}

void MongoModule::handleMessage(charlie::CMessageTarget* target, std::string& data)
{
}

u32 MongoModule::getModuleId()
{
  return 210060544;
}

MongoModule::~MongoModule()
{
}

CHARLIE_CONSTRUCT(MongoModule);
