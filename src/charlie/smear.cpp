#include <charlie/smear.h>

u16 mask[5] = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };

void smear( u16 id[] )
{
  for ( u32 i = 0; i < 5; i++ )
    for ( u32 j = i; j < 5; j++ )
      if ( i != j )
        id[i] ^= id[j];

  for ( u32 i = 0; i < 5; i++ )
    id[i] ^= mask[i];
}

void unsmear( u16 id[] )
{
  for ( u32 i = 0; i < 5; i++ )
    id[i] ^= mask[i];

  for ( u32 i = 0; i < 5; i++ )
    for ( u32 j = 0; j < i; j++ )
      if ( i != j )
        id[4-i] ^= id[4-j];
}
