#include <charlie/machine_id.h>
#include <string.h>
#include <boost/asio/ip/host_name.hpp>

u16 smear_mask[5] = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };

void smear( u16 id[] )
{
  for ( u32 i = 0; i < 5; i++ )
    for ( u32 j = i; j < 5; j++ )
      if ( i != j )
        id[i] ^= id[j];

  for ( u32 i = 0; i < 5; i++ )
    id[i] ^= smear_mask[i];
}

void unsmear( u16 id[] )
{
  for ( u32 i = 0; i < 5; i++ )
    id[i] ^= smear_mask[i];

  for ( u32 i = 0; i < 5; i++ )
    for ( u32 j = 0; j < i; j++ )
      if ( i != j )
        id[4-i] ^= id[4-j];
}

#define s8 char

#if defined(_WIN64) || defined(_WIN32)

#include <windows.h>
#include <intrin.h>
#include <iphlpapi.h>

// we just need this for purposes of unique machine id.
// So any one or two mac's is fine.
u16 hashMacAddress( PIP_ADAPTER_INFO info )
{
  u16 hash = 0;
  for ( u32 i = 0; i < info->AddressLength; i++ )
  {
    hash += ( info->Address[i] << (( i & 1 ) * 8 ));
  }
  return hash;
}

void getMacHash( u16& mac1, u16& mac2 )
{
  IP_ADAPTER_INFO AdapterInfo[32];
  DWORD dwBufLen = sizeof( AdapterInfo );

  DWORD dwStatus = GetAdaptersInfo( AdapterInfo, &dwBufLen );
  if ( dwStatus != ERROR_SUCCESS )
    return; // no adapters.

  PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
  mac1 = hashMacAddress( pAdapterInfo );
  if ( pAdapterInfo->Next )
    mac2 = hashMacAddress( pAdapterInfo->Next );

  // sort the mac addresses. We don't want to invalidate
  // both macs if they just change order.
  if ( mac1 > mac2 )
  {
    u16 tmp = mac2;
    mac2 = mac1;
    mac1 = tmp;
  }
}

u16 getVolumeHash()
{
  DWORD serialNum = 0;

  // Determine if this volume uses an NTFS file system.
  GetVolumeInformation( "c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0 );
  u16 hash = (u16)(( serialNum + ( serialNum >> 16 )) & 0xFFFF );

  return hash;
}

std::string getMachineName()
{
  static char computerName[1024];
  DWORD size = 1024;
  GetComputerName( computerName, &size );
  return std::string(&(computerName[0]));
}

u16 getCpuHash()
{
  int cpuinfo[4] = { 0, 0, 0, 0 };
  __cpuid( cpuinfo, 0 );
  u16 hash = 0;
  u16* ptr = (u16*)(&cpuinfo[0]);
  for ( u32 i = 0; i < 8; i++ )
    hash += ptr[i];

  return hash;
}


#else
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#ifdef DARWIN
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/if_types.h>
#else //!DARWIN
#include <linux/sockios.h>
#endif //!DARWIN

#include <sys/resource.h>
#include <sys/utsname.h>

//---------------------------------get MAC addresses ---------------------------------
// we just need this for purposes of unique machine id. So any one or two
// mac's is fine.
u16 hashMacAddress( u8* mac )
{
  u16 hash = 0;

  for ( u32 i = 0; i < 6; i++ )
  {
    hash += ( mac[i] << (( i & 1 ) * 8 ));
  }
  return hash;
}

void getMacHash( u16& mac1, u16& mac2 )
{
  mac1 = 0;
  mac2 = 0;

#ifdef DARWIN

  struct ifaddrs* ifaphead;
  if ( getifaddrs( &ifaphead ) != 0 )
    return;

  // iterate over the net interfaces
  bool foundMac1 = false;
  struct ifaddrs* ifap;
  for ( ifap = ifaphead; ifap; ifap = ifap->ifa_next )
  {
    struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifap->ifa_addr;
    if ( sdl && ( sdl->sdl_family == AF_LINK ) && ( sdl->sdl_type == IFT_ETHER ))
    {
      if ( !foundMac1 )
      {
        foundMac1 = true;
        mac1 = hashMacAddress( (u8*)(LLADDR(sdl))); //sdl->sdl_data) +
        sdl->sdl_nlen) );
      } else {
        mac2 = hashMacAddress( (u8*)(LLADDR(sdl))); //sdl->sdl_data) +
        sdl->sdl_nlen) );
        break;
      }
    }
  }

  freeifaddrs( ifaphead );

#else // !DARWIN

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP );
  if ( sock < 0 ) return;

  // enumerate all IP addresses of the system
  struct ifconf conf;
  char ifconfbuf[ 128 * sizeof(struct ifreq)  ];
  memset( ifconfbuf, 0, sizeof( ifconfbuf ));
  conf.ifc_buf = ifconfbuf;
  conf.ifc_len = sizeof( ifconfbuf );
  if ( ioctl( sock, SIOCGIFCONF, &conf ))
  {
    return;
  }

  // get MAC address
  bool foundMac1 = false;
  struct ifreq* ifr;
  for ( ifr = conf.ifc_req; (s8*)ifr < (s8*)conf.ifc_req + conf.ifc_len; ifr++ )
  {
    if ( ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data )
      continue;  // duplicate, skip it

    if ( ioctl( sock, SIOCGIFFLAGS, ifr ))
      continue;  // failed to get flags, skip it
    if ( ioctl( sock, SIOCGIFHWADDR, ifr ) == 0 )
    {
      if ( !foundMac1 )
      {
        foundMac1 = true;
        mac1 = hashMacAddress( (u8*)&(ifr->ifr_addr.sa_data));
      } else {
        mac2 = hashMacAddress( (u8*)&(ifr->ifr_addr.sa_data));
        break;
      }
    }
  }

  close( sock );

#endif // !DARWIN

  // sort the mac addresses. We don't want to invalidate
  // both macs if they just change order.
  if ( mac1 > mac2 )
  {
    u16 tmp = mac2;
    mac2 = mac1;
    mac1 = tmp;
  }
}

std::string getMachineName()
{
  static struct utsname u;

  if ( uname( &u ) < 0 )
  {
    assert(0);
    return std::string("unknown");
  }
  return std::string(u.nodename);
}

u16 getVolumeHash()
{
  // we don't have a 'volume serial number' like on windows.
  // Lets hash the system name instead.
  std::string sysnames = getMachineName();
  u8* sysname = (u8*)sysnames.c_str();
  u16 hash = 0;

  for ( u32 i = 0; sysname[i]; i++ )
    hash += ( sysname[i] << (( i & 1 ) * 8 ));

  return hash;
}

#ifdef DARWIN
#include <mach-o/arch.h>
u16 getCpuHash()
{
  const NXArchInfo* info = NXGetLocalArchInfo();
  u16 val = 0;
  val += (u16)info->cputype;
  val += (u16)info->cpusubtype;
  return val;
}

#else // !DARWIN

void getCpuid( u32* p, u32 ax )
{
  __asm __volatile
    (   "movl %%ebx, %%esi\n\t"
        "cpuid\n\t"
        "xchgl %%ebx, %%esi"
        : "=a" (p[0]), "=S" (p[1]),
        "=c" (p[2]), "=d" (p[3])
        : "0" (ax)
    );
}

u16 getCpuHash()
{
  u32 cpuinfo[4] = { 0, 0, 0, 0 };
  getCpuid( cpuinfo, 0 );
  u16 hash = 0;
  u32* ptr = (&cpuinfo[0]);
  for ( u32 i = 0; i < 4; i++ )
    hash += (ptr[i] & 0xFFFF) + ( ptr[i] >> 16 );

  return hash;
}
#endif // !DARWIN

#endif

u16* computeSystemUniqueId()
{
  static u16 id[5];
  static bool computed = false;

  if ( computed ) return id;

  // produce a number that uniquely identifies this system.
  id[0] = getCpuHash();
  id[1] = getVolumeHash();
  getMacHash( id[2], id[3] );

  // fifth block is some checkdigits
  id[4] = 0;
  for ( u32 i = 0; i < 4; i++ )
    id[4] += id[i];

  smear( id );

  computed = true;
  return id;
}

const char* getSystemUniqueId()
{
  // get the name of the computer
  std::string buf = getMachineName();

  u16* id = computeSystemUniqueId();
  for ( u32 i = 0; i < 5; i++ )
  {
    char num[16];
    snprintf( num, 16, "%x", id[i] );
    buf.append("-");
    switch( strlen( num ))
    {
      case 1: buf.append("000"); break;
      case 2: buf.append("00");  break;
      case 3: buf.append("0");   break;
    }
    buf.append(num);
  }

  std::transform(buf.begin(), buf.end(), buf.begin(), ::toupper);

  char * cstr = (char*)malloc((buf.length()+1)*sizeof(char));
  std::strcpy (cstr, buf.c_str());
  return cstr;
}
