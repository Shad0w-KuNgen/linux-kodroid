// KoCrashHandler.h — yerel çökmelerde (SIGSEGV/SIGABRT/SIGBUS/SIGFPE/SIGILL) geri izlemeyi Log.txt'ye
// ve Android logcat'e yazar; adresler "kitaplık+ofset" biçimindedir (CI'daki sembollü .so ile
// llvm-addr2line / ndk-stack çözer).
#pragma once

#include <string>

void KoInstallCrashHandler(const std::string& logPath);
