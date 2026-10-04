#include "crc32.h"

uint32_t crc32(const uint8_t* s, uint32_t len, uint32_t startVal)
{
	uint32_t i, crc32val = startVal;
	for (i = 0; i < len; i++)
		crc32val = crc32_tab[(crc32val ^ s[i]) & 0xff] ^ (crc32val >> 8);

	return crc32val;
}
