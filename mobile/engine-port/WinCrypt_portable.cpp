// WinCrypt_portable.cpp — CWinCrypt'in Windows CryptoAPI'siz uygulaması.
//
// Orijinal: CryptAcquireContext(PROV_RSA_FULL) → CryptCreateHash(CALG_SHA) → CryptHashData(Cipher)
//           → CryptDeriveKey(CALG_RC4, 128 bit) → her ReadFile'da CryptDecrypt(..., Final=TRUE, ...)
//
// CryptDeriveKey: istenen anahtar uzunluğu (16 bayt) hash uzunluğundan (20) küçük olduğu için
// anahtar = SHA-1(Cipher)'ın ilk 16 baytı. RC4 akış şifresinde Final=TRUE çağrısı anahtar
// akışını başa sarar; yani her ReadFile parçası RC4'ün ilk baytlarından itibaren çözülür.
// Bu dosya tam olarak bu davranışı yeniden üretir.
#if !defined(_WIN32)

#include <N3Base/StdAfxBase.h>
#include <N3Base/WinCrypt.h>
#include <N3Base/N3Base.h>

#include <FileIO/File.h>

#include <array>
#include <cstdint>
#include <cstring>

namespace
{
// --- SHA-1 (FIPS 180-1) ---
struct Sha1
{
	uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
	uint8_t block[64] {};
	size_t blockLen = 0;
	uint64_t total  = 0;

	static uint32_t rol(uint32_t v, int n)
	{
		return (v << n) | (v >> (32 - n));
	}

	void processBlock(const uint8_t* p)
	{
		uint32_t w[80];
		for (int i = 0; i < 16; ++i)
			w[i] = (uint32_t) p[i * 4] << 24 | (uint32_t) p[i * 4 + 1] << 16 | (uint32_t) p[i * 4 + 2] << 8 | (uint32_t) p[i * 4 + 3];
		for (int i = 16; i < 80; ++i)
			w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
		uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
		for (int i = 0; i < 80; ++i)
		{
			uint32_t f, k;
			if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999u; }
			else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1u; }
			else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDCu; }
			else { f = b ^ c ^ d; k = 0xCA62C1D6u; }
			uint32_t t = rol(a, 5) + f + e + k + w[i];
			e = d; d = c; c = rol(b, 30); b = a; a = t;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
	}

	void update(const void* data, size_t len)
	{
		const uint8_t* p = static_cast<const uint8_t*>(data);
		total += len;
		while (len > 0)
		{
			size_t take = std::min(len, sizeof(block) - blockLen);
			std::memcpy(block + blockLen, p, take);
			blockLen += take;
			p += take;
			len -= take;
			if (blockLen == sizeof(block))
			{
				processBlock(block);
				blockLen = 0;
			}
		}
	}

	void finish(uint8_t out[20])
	{
		uint64_t bits = total * 8;
		uint8_t pad   = 0x80;
		update(&pad, 1);
		uint8_t zero = 0;
		while (blockLen != 56)
			update(&zero, 1);
		uint8_t lenBytes[8];
		for (int i = 0; i < 8; ++i)
			lenBytes[i] = (uint8_t) (bits >> (56 - 8 * i));
		update(lenBytes, 8);
		for (int i = 0; i < 5; ++i)
		{
			out[i * 4]     = (uint8_t) (h[i] >> 24);
			out[i * 4 + 1] = (uint8_t) (h[i] >> 16);
			out[i * 4 + 2] = (uint8_t) (h[i] >> 8);
			out[i * 4 + 3] = (uint8_t) (h[i]);
		}
	}
};

// --- RC4 ---
struct Rc4
{
	uint8_t s[256];
	uint8_t i = 0, j = 0;

	void setKey(const uint8_t* key, size_t len)
	{
		for (int k = 0; k < 256; ++k)
			s[k] = (uint8_t) k;
		uint8_t jj = 0;
		for (int k = 0; k < 256; ++k)
		{
			jj = (uint8_t) (jj + s[k] + key[k % len]);
			std::swap(s[k], s[jj]);
		}
		i = j = 0;
	}

	void crypt(uint8_t* data, size_t len)
	{
		for (size_t n = 0; n < len; ++n)
		{
			i = (uint8_t) (i + 1);
			j = (uint8_t) (j + s[i]);
			std::swap(s[i], s[j]);
			data[n] ^= s[(uint8_t) (s[i] + s[j])];
		}
	}
};

// Türetilmiş 128-bit RC4 anahtarı; süreç boyunca sabit.
std::array<uint8_t, 16> DeriveKey()
{
	Sha1 sha;
	sha.update(CWinCrypt::Cipher, sizeof(CWinCrypt::Cipher) - 1);
	uint8_t digest[20];
	sha.finish(digest);
	std::array<uint8_t, 16> key {};
	std::memcpy(key.data(), digest, 16);
	return key;
}
} // namespace

CWinCrypt::CWinCrypt()
{
	m_bIsLoaded      = false;
	m_hCryptProvider = 0;
	m_hCryptHash     = 0;
	m_hCryptKey      = 0;
}

bool CWinCrypt::Load()
{
	Release();
	m_bIsLoaded = true;
	return true;
}

void CWinCrypt::Release()
{
	m_bIsLoaded      = false;
	m_hCryptProvider = 0;
	m_hCryptHash     = 0;
	m_hCryptKey      = 0;
}

bool CWinCrypt::ReadFile(File& file, void* buffer, size_t bytesToRead, size_t* bytesRead)
{
	if (!file.Read(buffer, bytesToRead, bytesRead))
		return false;

	if (IsLoaded())
	{
		static const std::array<uint8_t, 16> key = DeriveKey();
		Rc4 rc4;
		rc4.setKey(key.data(), key.size()); // Final=TRUE davranışı: her parça akışın başından
		rc4.crypt(static_cast<uint8_t*>(buffer), bytesToRead);
	}
	return true;
}

CWinCrypt::~CWinCrypt()
{
	Release();
}

// Birim testi için dışa açılan yardımcılar
void KoCrypt_Sha1(const void* data, size_t len, uint8_t out[20])
{
	Sha1 s;
	s.update(data, len);
	s.finish(out);
}

void KoCrypt_Rc4(const uint8_t* key, size_t keyLen, uint8_t* data, size_t len)
{
	Rc4 r;
	r.setKey(key, keyLen);
	r.crypt(data, len);
}

#endif // !_WIN32
