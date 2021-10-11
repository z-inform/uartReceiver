#ifndef CRC32H
#define CRC32H
#define CRCPOLY 0xEDB88320
#define CRCINIT 0xFFFFFFFF

#include <stdint.h>

typedef uint32_t U32;
typedef uint8_t U8;

const U32 *GetCRC32PolyTable ();
U32 doCRC32(const void* dptr, U32 size);
U32 UpdateCRC32(const void* dptr, U32 size, U32 prev_crc);
U32 CRC32 (U8 data, U32 crc);
#endif
