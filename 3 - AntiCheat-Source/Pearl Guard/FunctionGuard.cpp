#include "FunctionGuard.h"

void FunctionGuard::KeepFunction(DWORD startAddress, size_t size, std::string n)
{
	DWORD hash = crc32((uint8*)startAddress, size, -1);
	keeps.push_back(KeepInfo(startAddress + 32, size, hash, n));
}

bool __fastcall FunctionGuard::Check()
{
	for (uint32 i = 0; i < keeps.size(); i++)
	{
		KeepInfo inf = keeps[i];
		DWORD hash = crc32((uint8*)(inf.inAddress - 32), inf.size, -1);
		if (hash != inf.hash) {
			return false;
		}
	}
	return true;
}
