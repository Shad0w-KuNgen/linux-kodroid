#include "HDRReader.h"
#include "Pearl Engine.h"

std::string basename(const std::string& filename);
bool fileexists(std::string& filename);

char* ReadAllBytes(const char* filename, size_t* read)
{
	ifstream ifs(filename, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();
	int length = pos;
	char* pChars = new char[length];
	ifs.seekg(0, ios::beg);
	ifs.read(pChars, length);
	ifs.close();
	*read = length;
	return pChars;
}

HDRReader::HDRReader(string basePath)
{
	m_basePath = basePath;
}

DWORD HDRReader::FindPatternEx(char* mem, int size, std::string search)
{
	char* pattern = (char*)search.c_str();
	uint32 mask = search.length();
	if (mask > size) return NULL;

	for (size_t i = 0; i < size; i++)
	{
		bool found = true;
		for (int j = 0; j < mask; j++)
			if (mem[i + j] != pattern[j])
				found = false;
		if (found)
			return i;
	}

	return NULL;
}

DWORD HDRReader::FindPattern(std::string search)
{
	char* pattern = (char*)search.c_str();
	uint32 mask = search.length();
	if (mask > m_hdrSize) return NULL;

	for (size_t i = 0; i < m_hdrSize; i++)
	{
		bool found = true;
		for (size_t j = 0; j < mask; j++)
			if (m_hdr[i + j] != pattern[j])
				found = false;
		if (found)
			return (DWORD)m_hdr + i;
	}

	return NULL;
}


SRC HDRReader::GetSRC(string fileName)
{
	if (Engine->str_contains(fileName, xorstr("test\\")))
		return SRC(0, "", 0, 0, NULL);

	m_hdr = ReadAllBytes(string(m_basePath + xorstr("UI\\ui.hdr")).c_str(), &m_hdrSize);

	fileName = basename(fileName);

	DWORD address = FindPattern(fileName);

	if (address == NULL)
		return SRC(0, "", 0, 0, NULL);

	uint32 nameLen = *(int*)(address - 4);

	if (!(nameLen > 0))
		return SRC(0, "", 0, 0, NULL);

	char* name = new char[nameLen + 1]{ 0 };
	memcpy(&name[0], (void*)address, nameLen);

	uint32 offset = *(int*)(address + nameLen);
	uint32 sizeInBytes = *(int*)(address + nameLen + sizeof(offset));

	std::free(m_hdr);

	auto it = srcBackup.find(offset);
	if (it != srcBackup.end())
		return it->second;

	LPVOID buff = VirtualAlloc(NULL, sizeInBytes + 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (buff) {
		HANDLE hFile = CreateFileA("UI\\ui.src", FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			DWORD dwCount;
			SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
			if (ReadFile(hFile, buff, sizeInBytes, &dwCount, NULL))
			{
				CloseHandle(hFile);
				SRC tmp = SRC(nameLen, name, offset, sizeInBytes, (char*)buff);
				srcBackup.insert({ tmp.offset, tmp });
				return tmp;
			}
			CloseHandle(hFile);
		}
	}
	return SRC(0, "", 0, 0, NULL);
}

std::string basename(const std::string& filename)
{
	if (filename.empty()) {
		return "";
	}

	auto len = filename.length();
	auto index = filename.find_last_of("/\\");

	if (index == std::string::npos) {
		return filename;
	}

	if (index + 1 >= len) {

		len--;
		index = filename.substr(0, len).find_last_of("/\\");

		if (len == 0) {
			return filename;
		}

		if (index == 0) {
			return filename.substr(1, len - 1);
		}

		if (index == std::string::npos) {
			return filename.substr(0, len);
		}

		return filename.substr(index + 1, len - index - 1);
	}

	return filename.substr(index + 1, len - index);
}

bool fileexists(std::string& filename) {
	std::ifstream ifile(filename.c_str());
	return (bool)ifile;
}

