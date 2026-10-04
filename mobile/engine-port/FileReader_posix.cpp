// FileReader_posix.cpp — FileReader'ın Boost.Interprocess yerine POSIX mmap ile uygulanması.
// Davranış FileIO/FileReader.cpp ile aynıdır (salt okunur bellek eşlemesi, ofset takibi).
#if !defined(_WIN32)

#include <FileIO/FileReader.h>

#include <windows.h> // KoResolvePath

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>

FileReader::FileReader()
{
}

bool FileReader::OpenExisting(const std::filesystem::path& path)
{
	Close();
	std::string resolved = KoResolvePath(path.string());
	int fd               = ::open(resolved.c_str(), O_RDONLY);
	if (fd < 0)
		return false;
	struct stat sb {};
	if (fstat(fd, &sb) != 0 || !S_ISREG(sb.st_mode))
	{
		::close(fd);
		return false;
	}
	void* base = nullptr;
	if (sb.st_size > 0)
	{
		base = mmap(nullptr, (size_t) sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (base == MAP_FAILED)
		{
			::close(fd);
			return false;
		}
	}
	_fd      = fd;
	_mapSize = (size_t) sb.st_size;
	_address = base;
	_size    = (uint64_t) sb.st_size;
	_offset  = 0;
	_path    = path;
	_open    = true;
	return true;
}

bool FileReader::Create(const std::filesystem::path&)
{
	return false;
}

bool FileReader::Read(void* buffer, size_t bytesToRead, size_t* bytesRead)
{
	if (bytesRead != nullptr)
		*bytesRead = 0;
	if (buffer == nullptr)
		return false;
	if (bytesToRead == 0)
		return true;
	if (!_open || _offset > _size)
		return false;
	const size_t remainingBytes = static_cast<size_t>(_size - _offset);
	const size_t bytesToCopy    = std::min(bytesToRead, remainingBytes);
	if (bytesToCopy == 0)
		return false;
	std::memcpy(buffer, static_cast<uint8_t*>(_address) + _offset, bytesToCopy);
	_offset += bytesToCopy;
	if (bytesRead != nullptr)
		*bytesRead = bytesToCopy;
	return true;
}

bool FileReader::Write(const void*, size_t, size_t*)
{
	return false;
}

bool FileReader::Seek(int64_t offset, int origin)
{
	if (!IsOpen())
		return false;
	int64_t newOffset = offset;
	switch (origin)
	{
		case SEEK_SET: break;
		case SEEK_CUR: newOffset += static_cast<int64_t>(_offset); break;
		case SEEK_END: newOffset += static_cast<int64_t>(_size); break;
		default: return false;
	}
	if (newOffset < 0 || newOffset > static_cast<int64_t>(_size))
		return false;
	_offset = static_cast<uint64_t>(newOffset);
	return true;
}

void FileReader::Flush()
{
}

bool FileReader::Close()
{
	if (!_open)
		return false;
	if (_address && _mapSize)
		munmap(_address, _mapSize);
	if (_fd >= 0)
		::close(_fd);
	_fd      = -1;
	_mapSize = 0;
	_address = nullptr;
	_size    = 0;
	_offset  = 0;
	_open    = false;
	_path.clear();
	return true;
}

FileReader::~FileReader()
{
	Close();
}

#endif // !_WIN32
