#include "StdAfxBase.h"
#include "AudioAsset.h"
#include "al_wrapper.h"

#include <FileIO/FileReader.h>
#include <shared/StringUtils.h>

bool ParseWAV(FileReader& file, ALenum* format, ALsizei* sampleRate, size_t* pcmChunkSize,
	const uint8_t** pcmDataBuffer, ALsizei* pcmDataSize);

BufferedAudioAsset::BufferedAudioAsset()
{
	Type     = AUDIO_ASSET_BUFFERED;
	BufferId = INVALID_AUDIO_BUFFER_ID;
}

BufferedAudioAsset::~BufferedAudioAsset()
{
	if (BufferId != INVALID_AUDIO_BUFFER_ID)
	{
		alDeleteBuffers(1, &BufferId);
		AL_CHECK_ERROR();
	}

	BufferId = INVALID_AUDIO_BUFFER_ID;
}

bool KoDecodeOggToPcm(const uint8_t* data, size_t size, std::vector<uint8_t>& pcm, int& channels, int& sampleRate);

// 2369: sound.tbl adları .mp3/.wav olabilir ama Snd/ klasöründe .ogg bulunur (ya da tersi). Var olan dosyayı seç.
static std::string KoResolveAudioPath(const std::string& filename)
{
	auto exists = [](const std::string& fn) {
		FileReader f;
		return f.OpenExisting(fn);
	};
	if (exists(filename))
		return filename;
	size_t dot = filename.find_last_of('.');
	if (dot == std::string::npos)
		return filename;
	std::string base = filename.substr(0, dot);
	for (const char* ext : {".ogg", ".mp3", ".wav"})
	{
		std::string alt = base + ext;
		if (alt != filename && exists(alt))
			return alt;
	}
	return filename;
}

static bool KoHasExt(const std::string& fn, const char* ext)
{
	return fn.size() >= 4 && strnicmp(fn.data() + fn.size() - 4, ext, 4) == 0;
}

bool BufferedAudioAsset::LoadFromFile(const std::string& filenameIn)
{
	const std::string filename = KoResolveAudioPath(filenameIn);
	if (KoHasExt(filename, ".ogg"))
	{
		FileReader file;
		if (!file.OpenExisting(filename))
			return false;
		std::vector<uint8_t> pcm;
		int ch = 0, rate = 0;
		if (!KoDecodeOggToPcm(static_cast<const uint8_t*>(file.Memory()), (size_t) file.Size(), pcm, ch, rate))
			return false;
		alGenBuffers(1, &BufferId);
		if (AL_CHECK_ERROR())
			return false;
		ALenum fmt = ch >= 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		alBufferData(BufferId, fmt, pcm.data(), (ALsizei) pcm.size(), rate);
		if (AL_CHECK_ERROR())
			return false;
		Filename   = filename;
		AlFormat   = fmt;
		SampleRate = rate;
		return true;
	}
	if (!KoHasExt(filename, ".wav"))
		return false;

	FileReader file;
	if (!file.OpenExisting(filename))
		return false;

	ALenum alFormat     = AL_FORMAT_STEREO16;
	size_t pcmChunkSize = 0;
	ALsizei sampleRate = 0, pcmDataSize = 0;
	const uint8_t* pcmDataBuffer = nullptr;
	if (!ParseWAV(file, &alFormat, &sampleRate, &pcmChunkSize, &pcmDataBuffer, &pcmDataSize))
		return false;

	alGenBuffers(1, &BufferId);
	if (AL_CHECK_ERROR())
		return false;

	alBufferData(BufferId, alFormat, pcmDataBuffer, pcmDataSize, sampleRate);
	if (AL_CHECK_ERROR())
		return false;

	Filename   = filename;
	AlFormat   = alFormat;
	SampleRate = static_cast<int32_t>(sampleRate);

	return true;
}

StreamedAudioAsset::StreamedAudioAsset()
{
	Type          = AUDIO_ASSET_STREAMED;
	PcmDataSize   = 0;
	PcmChunkSize  = 0;
	PcmDataBuffer = nullptr;
}

bool StreamedAudioAsset::LoadFromFile(const std::string& filenameIn)
{
	const std::string filename = KoResolveAudioPath(filenameIn);
	if (KoHasExt(filename, ".ogg"))
	{
		// Tam çözüp bellek içi PCM akışı olarak sun (AudioDecoderThread: File boşsa OwnedPcm'den okur)
		FileReader file;
		if (!file.OpenExisting(filename))
			return false;
		int ch = 0, rate = 0;
		if (!KoDecodeOggToPcm(static_cast<const uint8_t*>(file.Memory()), (size_t) file.Size(), OwnedPcm, ch, rate))
			return false;
		DecoderType   = AUDIO_DECODER_PCM;
		File.reset();
		Filename      = filename;
		AlFormat      = ch >= 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		SampleRate    = rate;
		PcmChunkSize  = (size_t) rate * ch * sizeof(short) / 4; // çeyrek saniyelik parça
		if (PcmChunkSize == 0)
			PcmChunkSize = 16384;
		PcmDataSize   = OwnedPcm.size();
		PcmDataBuffer = OwnedPcm.data();
		return true;
	}
	// Expect ".mp3" or ".wav", both 4 characters long.
	constexpr size_t ExtensionLength = 4;

	if (filename.length() < ExtensionLength)
		return false;

	const char* extension = filename.data() + filename.length() - ExtensionLength;

	if (strnicmp(extension, ".mp3", ExtensionLength) == 0)
		return load_from_file_mp3(filename);

	if (strnicmp(extension, ".wav", ExtensionLength) == 0)
		return load_from_file_wav(filename);

	// Unsupported extension
	return false;
}

bool StreamedAudioAsset::load_from_file_mp3(const std::string& filename)
{
	auto file = std::make_unique<FileReader>();
	if (file == nullptr)
		return false;

	if (!file->OpenExisting(filename))
		return false;

	DecoderType = AUDIO_DECODER_MP3;
	File        = std::move(file);
	Filename    = filename;
	AlFormat    = AL_FORMAT_STEREO16;
	SampleRate  = 44100;

	return true;
}

bool StreamedAudioAsset::load_from_file_wav(const std::string& filename)
{
	auto file = std::make_unique<FileReader>();
	if (file == nullptr)
		return false;

	if (!file->OpenExisting(filename))
		return false;

	ALenum alFormat    = -1;
	ALsizei sampleRate = 0, pcmDataSize = 0;
	size_t pcmChunkSize          = 0;
	const uint8_t* pcmDataBuffer = nullptr;

	if (!ParseWAV(*file, &alFormat, &sampleRate, &pcmChunkSize, &pcmDataBuffer, &pcmDataSize))
		return false;

	DecoderType   = AUDIO_DECODER_PCM;
	File          = std::move(file);
	Filename      = filename;
	AlFormat      = alFormat;
	SampleRate    = static_cast<int32_t>(sampleRate);

	PcmChunkSize  = pcmChunkSize;
	PcmDataSize   = static_cast<size_t>(pcmDataSize);
	PcmDataBuffer = pcmDataBuffer;

	return true;
}

StreamedAudioAsset::~StreamedAudioAsset()
{
}
