// crypt_test.cpp — SHA-1 ve RC4 bilinen test vektörleri (FIPS 180-1, RFC 6229).
#include <cstdint>
#include <cstdio>
#include <cstring>

void KoCrypt_Sha1(const void* data, size_t len, uint8_t out[20]);
void KoCrypt_Rc4(const uint8_t* key, size_t keyLen, uint8_t* data, size_t len);

static int fails = 0;
static void hex(const uint8_t* p, size_t n, char* out)
{
	for (size_t i = 0; i < n; ++i)
		std::sprintf(out + i * 2, "%02x", p[i]);
	out[n * 2] = 0;
}

int main()
{
	uint8_t d[20];
	char h[64];
	KoCrypt_Sha1("abc", 3, d);
	hex(d, 20, h);
	if (std::strcmp(h, "a9993e364706816aba3e25717850c26c9cd0d89d") != 0) { std::printf("SHA1(abc) hatalı: %s\n", h); ++fails; }
	KoCrypt_Sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 56, d);
	hex(d, 20, h);
	if (std::strcmp(h, "84983e441c3bd26ebaae4aa1f95129e5e54670f1") != 0) { std::printf("SHA1(56 bayt) hatalı: %s\n", h); ++fails; }

	// RFC 6229: key 0102030405, ilk 16 çıktı baytı b2 39 63 05 f0 3d c0 27 cc c3 52 4a 0a 11 18 a8
	uint8_t key[5] = {1, 2, 3, 4, 5};
	uint8_t buf[16] = {};
	KoCrypt_Rc4(key, 5, buf, 16);
	hex(buf, 16, h);
	if (std::strcmp(h, "b2396305f03dc027ccc3524a0a1118a8") != 0) { std::printf("RC4 hatalı: %s\n", h); ++fails; }

	std::printf(fails ? "SONUÇ: %d hata\n" : "SONUÇ: SHA-1 ve RC4 vektörleri geçti\n", fails);
	return fails ? 1 : 0;
}
