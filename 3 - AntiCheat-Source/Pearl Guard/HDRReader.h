#pragma once
#include "stdafx.h"
#include <map>

struct SRC
{
	uint32 nameLen;
	string name;
	uint32 offset;
	uint32 sizeInBytes;
	char* buff;
	SRC(uint32 nl, string n, uint32 o, uint32 s, char* b)
	{
		nameLen = nl;
		name = n;
		offset = o;
		sizeInBytes = s;
		buff = b;
	}
};

class HDRReader
{
	char* m_hdr;
	size_t m_hdrSize;
	string m_basePath;
	DWORD FindPattern(std::string search);
	std::map<uint32, SRC> srcBackup;
public:
	DWORD FindPatternEx(char* mem, int size, std::string search);
	HDRReader(string basePath);
	SRC GetSRC(string fileName);
};

extern HDRReader* hdrReader;