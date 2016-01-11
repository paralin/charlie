#include <server_modules/mongo/Mongo.h>

using namespace server_modules::mongo;

bool MongoModule::mongoInited = false;

MongoModule::MongoModule() :
  mInter(NULL),
  connected(false)
{
  MLOG("Mongo module constructed...");
  connMtx.lock();
}

void MongoModule::shutdown()
{
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
    mongoUrl = mongourl;
  else
  {
    CERR("No MONGO_URL specified, using localhost as default.");
    mongoUrl = "localhost";
  }

  if (const char* db = std::getenv("MONGO_DB"))
    dbname = db;
  else
  {
    CERR("No MONGO_DB specified, using 'charlie'.");
    dbname = "charlie";
  }

  CLOG("Attempting to connect to "  << mongoUrl);
  try
  {
    conn.connect(mongoUrl);
  } catch (::mongo::UserException& ex)
  {
    CERR("Unable to connect to mongo! " << ex.what());
    return;
  }

  CLOG("Connected to mongo.");
  connMtx.unlock();
  clientsNs = dbname + ".clients";
}

void MongoModule::initWithInfo(modules::client::CClientSystemInfo& info)
{
  connMtx.lock();
  connMtx.unlock();

  std::string clientId = mInter->getClientId();
  ::mongo::BSONObj clientQuery = BSON("_id" << clientId);

  // Search for existing
  std::shared_ptr<::mongo::DBClientCursor> curs = conn.query(clientsNs, clientQuery);
  if (!curs->more())
  {
    CLOG("This is a new client, inserting record.");
    ::mongo::BSONObj obj = BSON(
        "_id" << clientId <<
        "system_id" << info.system_id() <<
        "cpu_hash" << info.cpu_hash() <<
        "hostname" << info.hostname());
    conn.insert(clientsNs, obj);
  }
  else
  {
    CLOG("Updating client info.");
    conn.update(clientsNs, clientQuery,
        BSON(
          "$set" << BSON(
            "system_id" << info.system_id() <<
            "cpu_hash" << info.cpu_hash() <<
            "hostname" << info.hostname())
          )
        );
  }
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
