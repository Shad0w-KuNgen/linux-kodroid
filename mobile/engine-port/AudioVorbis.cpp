// Ogg Vorbis çözücü (stb_vorbis). 2369 istemcisinin Snd/ klasörü .ogg; sound.tbl'de adlar .mp3/.wav olabilir.
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.c"
#include <vector>
#include <cstdint>
#include <cstdlib>

bool KoDecodeOggToPcm(const uint8_t* data, size_t size, std::vector<uint8_t>& pcm, int& channels, int& sampleRate)
{
	pcm.clear();
	if (data == nullptr || size == 0)
		return false;
	short* out = nullptr;
	int ch = 0, rate = 0;
	int samples = stb_vorbis_decode_memory(data, (int) size, &ch, &rate, &out);
	if (samples <= 0 || out == nullptr || ch <= 0)
	{
		if (out)
			free(out);
		return false;
	}
	channels   = ch;
	sampleRate = rate;
	pcm.assign((const uint8_t*) out, (const uint8_t*) out + (size_t) samples * ch * sizeof(short));
	free(out);
	return true;
}
