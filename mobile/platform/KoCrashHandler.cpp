// KoCrashHandler.cpp — bkz. KoCrashHandler.h
#include "KoCrashHandler.h"

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#if defined(__ANDROID__) || defined(__linux__)
#include <dlfcn.h>
#include <unistd.h>
#include <unwind.h>
#endif
#if defined(__ANDROID__)
#include <android/log.h>
#endif

namespace
{
std::string g_logPath;
static char g_altStack[64 * 1024];

#if defined(__ANDROID__) || defined(__linux__)
struct Backtrace
{
	uintptr_t pcs[64];
	int count = 0;
};

_Unwind_Reason_Code Trace(struct _Unwind_Context* ctx, void* arg)
{
	Backtrace* bt = (Backtrace*) arg;
	uintptr_t pc  = _Unwind_GetIP(ctx);
	if (pc == 0)
		return _URC_NO_REASON;
	if (bt->count >= (int) (sizeof(bt->pcs) / sizeof(bt->pcs[0])))
		return _URC_END_OF_STACK;
	bt->pcs[bt->count++] = pc;
	return _URC_NO_REASON;
}

void Emit(FILE* f, const char* line)
{
	if (f)
	{
		fputs(line, f);
		fputc('\n', f);
	}
	fputs(line, stderr);
	fputc('\n', stderr);
#if defined(__ANDROID__)
	__android_log_write(ANDROID_LOG_FATAL, "ko-crash", line);
#endif
}

void CrashHandler(int sig, siginfo_t* si, void*)
{
	// Sinyal bağlamında tam güvenli değil; çökme anında en iyi çaba.
	FILE* f = g_logPath.empty() ? nullptr : fopen(g_logPath.c_str(), "ab");
	char line[512];
	time_t now = time(nullptr);
	struct tm tmv;
	localtime_r(&now, &tmv);
	snprintf(line, sizeof(line), "    [%02d:%02d:%02d] FATAL: sinyal %d (%s) adres %p", tmv.tm_hour, tmv.tm_min, tmv.tm_sec, sig,
		sig == SIGSEGV ? "SIGSEGV" : sig == SIGABRT ? "SIGABRT" : sig == SIGBUS ? "SIGBUS" : sig == SIGFPE ? "SIGFPE" : sig == SIGILL ? "SIGILL" : "?",
		si ? si->si_addr : nullptr);
	Emit(f, line);

	Backtrace bt;
	_Unwind_Backtrace(Trace, &bt);
	for (int i = 0; i < bt.count; i++)
	{
		Dl_info info;
		memset(&info, 0, sizeof(info));
		const char* lib  = "?";
		uintptr_t offset = bt.pcs[i];
		const char* sym  = "";
		if (dladdr((void*) bt.pcs[i], &info) && info.dli_fname)
		{
			lib    = info.dli_fname;
			offset = bt.pcs[i] - (uintptr_t) info.dli_fbase;
			if (info.dli_sname)
				sym = info.dli_sname;
			const char* slash = strrchr(lib, '/');
			if (slash)
				lib = slash + 1;
		}
		snprintf(line, sizeof(line), "      #%02d pc %016llx %s %s", i, (unsigned long long) offset, lib, sym);
		Emit(f, line);
	}
	if (f)
		fclose(f);

	// Varsayılan davranışa dön (Android tombstone/logcat DEBUG satırları yine üretilir)
	signal(sig, SIG_DFL);
	raise(sig);
}
#endif
} // namespace

void KoInstallCrashHandler(const std::string& logPath)
{
	g_logPath = logPath;
#if defined(__ANDROID__) || defined(__linux__)
	stack_t ss;
	ss.ss_sp    = g_altStack;
	ss.ss_size  = sizeof(g_altStack);
	ss.ss_flags = 0;
	sigaltstack(&ss, nullptr);

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = CrashHandler;
	sa.sa_flags     = SA_SIGINFO | SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	const int sigs[] = { SIGSEGV, SIGABRT, SIGBUS, SIGFPE, SIGILL };
	for (int s : sigs)
		sigaction(s, &sa, nullptr);
#endif
}
