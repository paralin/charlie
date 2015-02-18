#include <Common.h>
#include <Logging.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <protogen/charlie.pb.h>
#include <fstream>
#include <charlie/xor.h>
#include <google/protobuf/text_format.h>
#include <boost/program_options.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/format.hpp>
#include <charlie/Crypto.h>
#include <stdlib.h>
#include <glib.h>
#include <gmodule.h>
#include <stdio.h>
#include <boost/regex.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <charlie/hash.h>
#include <openssl/sha.h>
#include <cserver/ModuleTable.h>
#include <ctime>

#include <protogen/manager.pb.h>

//#define PRINT_VERBOSE

namespace po = boost::program_options;
namespace fs = boost::filesystem;

struct GenericOptions {
  int cmdid;
  std::string xorkey_;
};

struct GenIdentCommand : public GenericOptions {
  std::string output_;
};

struct GenPubkeyCommand : public GenericOptions {
  std::string output_;
  std::string identity_;
};

struct GenModtableCommand : public GenericOptions {
  std::string output_;
  std::string json_;
  bool        sign_;
  std::string identity_;
  std::string identxor_;
  bool        hash_;
  std::string libprefix_;
  std::string libsuffix_;
};

struct EmbedCommand : public GenericOptions {
  std::string output_;
  std::string input_;
  bool usegmodule_;
};

struct ProtoCleanCommand : public GenericOptions {
  std::string output_;
  std::string input_;
};

struct HashStrCommand : public GenericOptions {
  std::string str_;
};

typedef boost::variant<GenIdentCommand, GenPubkeyCommand, EmbedCommand, ProtoCleanCommand, HashStrCommand, GenModtableCommand> Command;

Command ParseOptions(int argc, const char *argv[])
{
  po::options_description global("Global options"); global.add_options()
    ("xorkey", po::value<std::string>(), "XOR key for input/output")
    ("command", po::value<std::string>()->required(), "command to execute")
    ("subargs", po::value<std::vector<std::string> >(), "Arguments for command");

  po::positional_options_description pos;
  pos.add("command", 1).
    add("subargs", -1);

  po::variables_map vm;

  po::parsed_options parsed = po::command_line_parser(argc, argv).
    options(global).
    positional(pos).
    allow_unregistered().
    run();

  po::store(parsed, vm);

  std::string cmd;
  try
  {
    cmd = vm["command"].as<std::string>();
  }catch(const std::exception& e ) {
  }

  if (cmd == "genident")
  {
    po::options_description geni_desc("genident options");
    geni_desc.add_options()
      ("output", po::value<std::string>(), "Output file");

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Parse again...
    po::store(po::command_line_parser(opts).options(geni_desc).run(), vm);

    GenIdentCommand comm;
    comm.cmdid = 0;
    try
    {
      comm.output_ = vm["output"].as<std::string>();
      if(vm.count("xorkey") > 0)
        comm.xorkey_ = vm["xorkey"].as<std::string>();
    }
    catch (const std::exception& e)
    {
      std::cout << "Charlie Utility App" << std::endl
        << "./cutils --xorkey [key] genident --output [file]" << std::endl << std::endl
        << global << std::endl
        << geni_desc << std::endl
        << "genident: Generate a new identity file." << std::endl;
      throw e;
    }
    return comm;
  }
  else if (cmd == "genpubkey")
  {
    po::options_description geni_desc("genpubkey options");
    geni_desc.add_options()
      ("identity", po::value<std::string>(), "Identity file")
      ("output", po::value<std::string>(), "Output file");

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Parse again...
    po::store(po::command_line_parser(opts).options(geni_desc).run(), vm);

    GenPubkeyCommand comm;
    comm.cmdid = 1;
    try
    {
      comm.output_ = vm["output"].as<std::string>();
      if(vm.count("xorkey") > 0)
        comm.xorkey_ = vm["xorkey"].as<std::string>();
      comm.identity_ = vm["identity"].as<std::string>();
    }
    catch (const std::exception& e)
    {
      std::cout << "Charlie Utility App" << std::endl
        << "./cutils --xorkey [key] genpubkey --output [file] --identity [file]" << std::endl << std::endl
        << global << std::endl
        << geni_desc << std::endl
        << "genpubkey: Remove the private key from an identity." << std::endl;
      throw e;
    }
    return comm;
  }
  else if (cmd == "embed")
  {
    po::options_description geni_desc("embed options");
    geni_desc.add_options()
      ("usegmodule", "Use gmodule filename formatter")
      ("input", po::value<std::string>(), "Path to file to embed")
      ("output", po::value<std::string>(), "Output file");

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Parse again...
    po::store(po::command_line_parser(opts).options(geni_desc).run(), vm);

    EmbedCommand comm;
    comm.cmdid = 2;
    try
    {
      comm.output_ = vm["output"].as<std::string>();
      comm.input_ = vm["input"].as<std::string>();
      comm.usegmodule_ = vm.count("usegmodule")>0;
      if(vm.count("xorkey")>0)
        comm.xorkey_ = vm["xorkey"].as<std::string>();
    }
    catch (const std::exception& e)
    {
      std::cout << "Charlie Utility App" << std::endl
        << "./cutils --xorkey [key] embed --output [file] --input [file] [--usegmodule]" << std::endl << std::endl
        << global << std::endl
        << geni_desc << std::endl
        << "embed: Compile a binary file into a const char* arr." << std::endl;
      throw e;
    }
    return comm;
  }
  else if (cmd == "protoclean")
  {
    po::options_description geni_desc("protoclean options");
    geni_desc.add_options()
      ("input", po::value<std::string>(), "Path to file to clean")
      ("output", po::value<std::string>(), "Output file");

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Parse again...
    po::store(po::command_line_parser(opts).options(geni_desc).run(), vm);

    ProtoCleanCommand comm;
    comm.cmdid = 3;
    try
    {
      comm.output_ = vm["output"].as<std::string>();
      comm.input_ = vm["input"].as<std::string>();
    }
    catch (const std::exception& e)
    {
      std::cout << "Charlie Utility App" << std::endl
        << "./cutils protoclean --output [file] --input [file]" << std::endl << std::endl
        << global << std::endl
        << geni_desc << std::endl
        << "protoclean: Clean a protobuf file of any info." << std::endl;
      throw e;
    }
    return comm;
  }
  else if (cmd == "hashstr")
  {
    po::options_description geni_desc("protoclean options");
    geni_desc.add_options()
      ("str", po::value<std::string>(), "String to hash");

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Parse again...
    po::store(po::command_line_parser(opts).options(geni_desc).run(), vm);

    HashStrCommand comm;
    comm.cmdid = 4;
    try
    {
      comm.str_ = vm["str"].as<std::string>();
    }
    catch (const std::exception& e)
    {
      std::cout << "Charlie Utility App" << std::endl
        << "./cutils hashstr --str [string]" << std::endl << std::endl
        << global << std::endl
        << geni_desc << std::endl
        << "hashstr: Test std::hash to integer." << std::endl;
      throw e;
    }
    return comm;
  }
  else if (cmd == "genmodtable")
  {
    po::options_description geni_desc("genmodtable options");
    geni_desc.add_options()
      ("hash", "Hash the libraries using gmodule for name")
      ("sign", "Sign the data in a CSignedBuffer.")
      ("identity", po::value<std::string>(), "Identity for CSignedBuffer")
      ("identxor", po::value<std::string>(), "XOR key for identity file")
      ("output", po::value<std::string>(), "File to write the module table to")
      ("libprefix", po::value<std::string>(), "Prefix of a library file")
      ("libsuffix", po::value<std::string>(), "Suffix of a library file")
      ("json", po::value<std::string>(), "Input json file for the module table.");

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
    opts.erase(opts.begin());

    // Parse again...
    po::store(po::command_line_parser(opts).options(geni_desc).run(), vm);

    GenModtableCommand comm;
    comm.cmdid = 5;
    try
    {
      comm.json_ = vm["json"].as<std::string>();
      comm.output_ = vm["output"].as<std::string>();
      comm.libprefix_ = vm["libprefix"].as<std::string>();
      comm.libsuffix_ = vm["libsuffix"].as<std::string>();
      comm.sign_ = false;
      if(vm.count("hash") > 0){
        comm.hash_ = true;
      }
      if(vm.count("sign") > 0){
        comm.sign_ = true;
        comm.identity_ = vm["identity"].as<std::string>();
        if(vm.count("identxor") > 0)
          comm.identxor_ = vm["identxor"].as<std::string>();
      }
      if(vm.count("xorkey")>0)
        comm.xorkey_ = vm["xorkey"].as<std::string>();
    }
    catch (const std::exception& e)
    {
      std::cout << "Charlie Utility App" << std::endl
        << "./cutils --xorkey [key] genmodtable --json [file] --output [file]" << std::endl << std::endl
        << global << std::endl
        << geni_desc << std::endl
        << "genmodtable: Serialize a JSON module table to protobuf." << std::endl;
      throw e;
    }
    return comm;
  }

  // unrecognised command
  std::cout << "Charlie Utility App" << std::endl
    << "./cutils --xorkey [key] [command] [args]" << std::endl
    << global << std::endl
    << "Note: subargs and command are positional" << std::endl
    << "Commands: genident, genpubkey, embed, protoclean, hashstr, genmodtable" << std::endl;
  throw po::invalid_option_value(cmd);
}

int generateIdentity(GenIdentCommand* comm, fs::path *full_path)
{
  std::string output;
  fs::path output_path((*full_path)/comm->output_);

  Crypto * crypto = new Crypto();
  charlie::CIdentity ident;

  CLOG("Path to root: " << (*full_path).string());
  CLOG("Output path: "<<output_path.string());

  CLOG("Generating an identity...");
  crypto->genLocalKeyPair();
  unsigned char* pkey;
  int pkeyLen = crypto->getLocalPriKey(&pkey);
  unsigned char* pubkey;
  int pubkeyLen = crypto->getLocalPubKey(&pubkey);
  ident.set_public_key(pubkey, pubkeyLen);
  ident.set_private_key(pkey, pkeyLen);

  int outSize = ident.ByteSize();
  char* out = (char*)malloc(sizeof(char)*outSize);
  if(!ident.SerializeToArray(out, outSize))
    CERR("Unable to serialize data to array.");

  if(comm->xorkey_.length()>0)
  {
    CLOG("Applying XOR key \""<<comm->xorkey_<<"\"...");
    apply_xor(out, outSize, comm->xorkey_.c_str(), comm->xorkey_.length());
  }

  std::ofstream outFile;
  outFile.open (output_path.string().c_str(), std::ios_base::out|std::ios_base::binary);
  outFile.write(out, outSize);
  outFile.close();

  delete crypto;
  free(out);
  return 0;
}

int generateEmbedFile(EmbedCommand* comm, fs::path *full_path)
{
  fs::path libpath(comm->input_);
  const gchar* curr = full_path->string().c_str();
  const gchar* symfile = comm->output_.c_str();
  fs::path symfilep(symfile);
  fs::path symn = symfilep.stem();

  const char* sym = symn.string().c_str();

  const char* path;
  if(comm->usegmodule_)
    path = (libpath.remove_filename()/boost::filesystem::path(g_module_build_path(curr, libpath.filename().string().c_str())).filename()).string().c_str();
  else
    path = (libpath.remove_filename()/boost::filesystem::path(libpath.filename().string().c_str())).filename().string().c_str();
  CLOG("Embedding "<<path<<" into "<<symfile);

  char* memblock;
  std::streampos size;
  std::ifstream inFile (path, std::ios_base::in|std::ios_base::binary|std::ios_base::ate);
  if(inFile.is_open())
  {
    size = inFile.tellg();
    memblock = new char [size];
    inFile.seekg (0, std::ios_base::beg);
    inFile.read (memblock, size);
    inFile.close();
  }else{
    CERR("Unable to open input file "<<path);
    return -1;
  }

  if(comm->xorkey_.length()>0)
  {
    CLOG("Applying XOR key \""<<comm->xorkey_<<"\"...");
    apply_xor(memblock, size, comm->xorkey_.c_str(), comm->xorkey_.length());
  }

  std::ofstream outFile (symfile, std::ios_base::out);
  if(outFile.is_open())
  {
    outFile << "#pragma GCC diagnostic ignored \"-Woverflow\"\n";
    outFile << "#include <stdlib.h>\n";
    outFile << "const char "<<sym<<"[] = {\n";
    size_t i;
    size_t linecount;
    int plen;
    char p [50];
    for(i=0;i<size;i++)
    {
      plen = sprintf(p, "0x%02x,", memblock[i]);
      outFile.write(p, plen);
      if (++linecount == 10) { outFile.put('\n'); linecount = 0; }
    }
    outFile << "\n};\nconst size_t "<<sym<<"_len = sizeof("<<sym<<");";
    outFile.close();
  }else{
    CERR("Unable to open output file "<<symfile);
    return -1;
  }
  free(memblock);
  return 0;
}


int generatePubKey(GenPubkeyCommand* comm, fs::path *full_path)
{
  std::string output;
  fs::path output_path((*full_path)/comm->output_);

  Crypto * crypto = new Crypto();
  charlie::CIdentity ident;

  CLOG("Loading server identity file...");
  fs::path input_path((*full_path)/comm->identity_);
  output_path = fs::path((*full_path)/comm->output_);
  std::ifstream inFile (input_path.string().c_str(), std::ios_base::in|std::ios_base::binary|std::ios_base::ate);
  if(inFile.is_open())
  {
    std::streampos size;
    char* memblock;
    size = inFile.tellg();
    memblock = new char [size];
    inFile.seekg (0, std::ios_base::beg);
    inFile.read (memblock, size);
    inFile.close();
    if(comm->xorkey_.length()>0)
    {
      CLOG("Applying XOR key \""<<comm->xorkey_<<"\"...");
      apply_xor(memblock, size, comm->xorkey_.c_str(), comm->xorkey_.length());
    }

    if(!ident.ParseFromArray(memblock, size))
    {
      CLOG("Unable to parse the memblock.");
      return 1;
    }
    delete[] memblock;

    CLOG("Clearing the private key");
    ident.clear_private_key();
#ifdef PRINT_VERBOSE
    google::protobuf::TextFormat::PrintToString(ident, &output);
    CLOG("Final identity: "<<std::endl<<output.c_str());
#endif
    int outbufsize = ident.ByteSize();
    memblock = (char*)malloc(sizeof(char)*outbufsize);
    if(!ident.SerializeToArray(memblock, outbufsize)){
      CERR("Unable to serialize data to array.");
      return 1;
    }

    if(comm->xorkey_.length()>0)
    {
      CLOG("Applying XOR key \""<<comm->xorkey_<<"\"...");
      apply_xor(memblock, outbufsize, comm->xorkey_.c_str(), comm->xorkey_.length());
    }

    std::ofstream of;
    of.open (output_path.string().c_str(), std::ios_base::out|std::ios_base::binary);
    of.write(memblock, outbufsize);
    of.close();
    free(memblock);
    return 0;
  }
  else
  {
    CLOG("Unable to open "<<input_path.string().c_str()<<"...");
    return -1;
  }
}

int hashStrTest(HashStrCommand* comm, fs::path *full_path)
{
  unsigned int hash = hashString((const char*)comm->str_.c_str());
  std::cout << hash << std::endl;
  return 0;
}

int protoCleanFile(ProtoCleanCommand* comm, fs::path *full_path)
{
  std::ifstream t(comm->input_.c_str());
  if(t.is_open())
  {
    boost::regex expression("(::std::string )([a-zA-Z]+)(::GetTypeName\\(\\) const {)");

    std::stringstream buffer;
    std::string    line;
    int i=-1;
    bool nextGetName = false;
    while(std::getline(t, line))
    {
      i++;
      boost::cmatch what;
      if(i == 0 && boost::starts_with(line, "//"))
      {
        buffer << "#define __FILE__ \"\"\n";
      }
      else if(nextGetName)
      {
        buffer<<"    return \"\";\n";
        nextGetName = false;
        continue;
      }
      else if(boost::regex_match(line.c_str(), what, expression))
      {
        CLOG("REPN: "<<i<<" "<<line);
        nextGetName = true;
      }
      buffer << line << "\n";
    }

    //Write output
    std::ofstream of;
    of.open(comm->output_.c_str(), std::ios_base::out);
    of << buffer.str();
    of.close();
  }else
  {
    CERR("Unable to open "<<comm->input_<<"...");
    return -1;
  }
  return 0;
}

Crypto * loadCrypto(std::string *infile, std::string *identxor)
{
  //Load the identity
  Crypto * crypt = new Crypto();

  std::ifstream inFile (infile->c_str(), std::ios_base::in|std::ios_base::binary|std::ios_base::ate);
  if(inFile.is_open())
  {
    std::streampos size;
    char* memblock;
    size = inFile.tellg();
    memblock = new char [size];
    inFile.seekg (0, std::ios_base::beg);
    inFile.read (memblock, size);
    inFile.close();

    CLOG("Loaded identity file...");

    if(identxor->length()>0)
    {
      CLOG("Applying XOR key \""<<identxor->c_str()<<"\"...");
      apply_xor(memblock, size, identxor->c_str(), identxor->length());
    }

    charlie::CIdentity ident;
    if(!ident.ParseFromArray(memblock, size))
    {
      CLOG("Unable to parse the identity.");
      free(memblock);
      return NULL;
    }

    if(!ident.has_private_key())
    {
      CERR("Identity doesn't have a private key.");
      free(memblock);
      return NULL;
    }

    std::string* pkey = ident.mutable_private_key();

    if(!crypt->setLocalPriKey((unsigned char*)pkey->c_str(), pkey->length()) == SUCCESS)
    {
      CERR("Private key is invalid.");
      free(memblock);
      return NULL;
    }

    free(memblock);
    return crypt;
  }else
  {
    CLOG("Can't open file "<<infile->c_str()<<"...");
    return NULL;
  }
}

int generateModuleTable(GenModtableCommand* comm, fs::path *full_path)
{
  fs::path output_path((*full_path)/comm->output_);

  CLOG("Loading json input file...");

  fs::path input_path((*full_path)/comm->json_);
  output_path = fs::path((*full_path)/comm->output_);
  std::ifstream inFile (input_path.string().c_str(), std::ios_base::in);
  if(inFile.is_open())
  {
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    Crypto * crypt = loadCrypto(&(comm->identity_), &(comm->identxor_));
    char* out;
    size_t outSize;
    if(generateModuleTableFromJson(buffer.str().c_str(), &out, crypt, &outSize, comm->hash_, comm->libprefix_, comm->libsuffix_, full_path->string(), comm->sign_) != 0)
    {
      CERR("Unable to generate module table from json.");
      return -1;
    }

    if(crypt != NULL)
      delete crypt;

    if(comm->xorkey_.length()>0)
    {
      CLOG("Applying XOR key \""<<comm->xorkey_<<"\"...");
      apply_xor(out, outSize, comm->xorkey_.c_str(), comm->xorkey_.length());
    }

    std::ofstream of;
    of.open (output_path.string().c_str(), std::ios_base::out|std::ios_base::binary);
    of.write((const char*)out, outSize);
    of.close();
    free(out);
    return 0;
  }
  else
  {
    CLOG("Unable to open "<<input_path.c_str()<<"...");
    return -1;
  }
}

int main(int argc, const char** argv)
{
  std::string pth1 (argv[0]);
  std::string pth2;

  if(boost::starts_with(pth1, "./"))
    pth2 = pth1.substr(2);
  else
    pth2 = pth1;

  fs::path full_path( fs::initial_path<fs::path>() );
  full_path = fs::system_complete( fs::path( pth2 ) ).remove_filename()/"/";

  Command comm;
  try
  {
    comm = ParseOptions(argc, argv);
  }
  catch(const std::exception & e)
  {
    //Usage already printed, quit
    return 1;
  }
  GenericOptions* opt = (GenericOptions*)(&comm);
  if(opt->cmdid == 0)
    return generateIdentity(&boost::get<GenIdentCommand>(comm), &full_path);
  else if(opt->cmdid == 1)
    return generatePubKey(&boost::get<GenPubkeyCommand>(comm), &full_path);
  else if(opt->cmdid == 2)
    return generateEmbedFile(&boost::get<EmbedCommand>(comm), &full_path);
  else if(opt->cmdid == 3)
    return protoCleanFile(&boost::get<ProtoCleanCommand>(comm), &full_path);
  else if(opt->cmdid == 4)
    return hashStrTest(&boost::get<HashStrCommand>(comm), &full_path);
  else if(opt->cmdid == 5)
    return generateModuleTable(&boost::get<GenModtableCommand>(comm), &full_path);
  return 1;
}
