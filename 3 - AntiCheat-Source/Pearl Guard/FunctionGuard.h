#pragma once
#include "stdafx.h"

struct KeepInfo
{
	DWORD inAddress;
	DWORD size;
	DWORD hash;
	std::string fncName;
	KeepInfo(DWORD a, DWORD b, DWORD h, std::string n)
	{
		inAddress = a;
		size = b;
		hash = h;
		fncName = n;
	}
};

class FunctionGuard
{
private:
	std::vector<KeepInfo> keeps;
public:
	void KeepFunction(DWORD startAddress, size_t size, std::string n = "-");
	bool __fastcall Check();
};
