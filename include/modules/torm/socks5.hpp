#ifndef SOCKS5_HPP
#define SOCKS5_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>

namespace socks5 {

  const unsigned char version = 0x05;

  struct socks5_ident_req
  {
    unsigned char Version;
    unsigned char NumberOfMethods;
    unsigned char Methods[2];
  };

  struct socks5_ident_resp
  {
    unsigned char Version;
    unsigned char Method;
  };

  struct socks5_generic_response
  {
    unsigned char Version;
    unsigned char Status;
  };

  struct socks5_req
  {
    unsigned char Version;
    unsigned char Cmd;
    unsigned char Reserved;
    unsigned char AddrType;
    /*
    union {
      in_addr IPv4;
      in6_addr IPv6;
      struct {
        unsigned char DomainLen;
        char Domain[256];
      };
    } DestAddr;
    unsigned short DestPort;
    */
  };

  struct socks5_resp
  {
    unsigned char Version;
    unsigned char Reply;
    unsigned char Reserved;
    unsigned char AddrType;
  };

} // namespace socks5

#endif // SOCKS5_HPP
