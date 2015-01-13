#include <Logging.h>
#include <charlie/Crypto.h>
#include <Common.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <proto/charlie.pb.h>
#include <iostream>
#include <fstream>
#include <charlie/xor.h>
#include <google/protobuf/text_format.h>

//#define PRINT_VERBOSE

namespace fs = boost::filesystem;

void printNotFound()
{
  CERR("!!! EXISTING IDENTITY FILE NOT FOUND !!!");
  CERR("!!! New identity will be generated!");
}
void printCantOpen()
{
  CERR("!!! CAN'T OPEN EXISTING IDENTITY FILE!!!");
  CERR("!!! New identity will be generated!");
}
void printCantParse()
{
  CERR("!!! CAN'T PARSE EXISTING IDENTITY FILE!!!");
  CERR("!!! New identity will be generated!");
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: ./genidentity <filename> <xorkey>\n";
    return 0;
  }

  std::string output;
  std::string pth1 (argv[0]);
  std::string pth2;

  if(boost::starts_with(pth1, "./"))
    pth2 = pth1.substr(2);
  else
    pth2 = pth1;

  fs::path full_path( fs::initial_path<fs::path>() );
  full_path = fs::system_complete( fs::path( pth2 ) ).remove_filename()/"/";
  fs::path output_path(full_path/argv[1]);

  Crypto * crypto = new Crypto();
  charlie::CIdentity ident;

  if (argc == 4 || argc == 5)
  {
    std::string a1 ("-finishCMake");
    if(a1.compare(argv[1]) == 0)
    {
      CLOG("Loading server identity file...");
      fs::path input_path(full_path/argv[2]);
      output_path = fs::path(full_path/argv[3]);
      std::ifstream inFile (input_path.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
      if(inFile.is_open())
      {
        std::streampos size;
        char* memblock;
        size = inFile.tellg();
        memblock = new char [size];
        inFile.seekg (0, std::ios::beg);
        inFile.read (memblock, size);
        inFile.close();
        if(argc == 5)
        {
          CLOG("Applying XOR key \""<<argv[4]<<"\"...");
          apply_xor(memblock, size, (const char*)argv[4], strlen(argv[4]));
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

        if(argc == 5)
        {
          CLOG("Applying XOR key \""<<argv[4]<<"\"...");
          apply_xor(memblock, outbufsize, (const char*)argv[4], strlen(argv[4]));
          CLOG("Key: "<<argv[4]<<" Len: "<<strlen(argv[4]));
        }

        std::ofstream of;
        of.open (output_path.c_str(), std::ios::out|std::ios::binary);
        of.write(memblock, outbufsize);
        of.close();
        free(memblock);
        return 0;
      }
      else
      {
        CLOG("Unable to open "<<input_path.c_str()<<"...");
        return -1;
      }
    }
  }


  CLOG("Path to root: " << full_path.string());
  CLOG("Output path: "<<output_path.string());

  CLOG("Generating an identity ["<<argc<<"]...");
  crypto->genLocalKeyPair();
  unsigned char* pkey;
  int pkeyLen = crypto->getLocalPriKey(&pkey);
  unsigned char* pubkey;
  int pubkeyLen = crypto->getLocalPubKey(&pubkey);
  ident.set_public_key(pubkey, pubkeyLen);
  ident.set_private_key(pkey, pkeyLen);

#ifdef PRINT_VERBOSE
  google::protobuf::TextFormat::PrintToString(ident, &output);
  CLOG("Final identity: "<<std::endl<<output.c_str());
#endif

  int outSize = ident.ByteSize();
  char* out = (char*)malloc(sizeof(char)*outSize);
  if(!ident.SerializeToArray(out, outSize))
    CERR("Unable to serialize data to array.");

  if(argc > 2)
  {
    CLOG("Applying XOR key \""<<argv[2]<<"\" with length \""<<strlen(argv[2])<<"\"...");
    apply_xor(out, outSize, (const char*)argv[2], strlen(argv[2]));
  }

  std::ofstream outFile;
  outFile.open (output_path.c_str(), std::ios::out|std::ios::binary);
  outFile.write(out, outSize);
  outFile.close();

  delete crypto;
  free(out);
  return 0;
}
