// winsock.cpp — WSAAsyncSelect öykünmesi (poll ile okunabilirlik/kapanma tespiti).
#include <winsock2.h>

#include <poll.h>

#include <algorithm>
#include <vector>

namespace
{
struct Watch
{
	SOCKET s;
	HWND hwnd;
	unsigned msg;
	long events;
};
std::vector<Watch> g_watches;
} // namespace

int WSAAsyncSelect(SOCKET s, HWND hwnd, unsigned msg, long events)
{
	g_watches.erase(std::remove_if(g_watches.begin(), g_watches.end(), [&](const Watch& w) { return w.s == s; }), g_watches.end());
	if (events != 0)
		g_watches.push_back({s, hwnd, msg, events});
	return 0;
}

void KoWinsockForget(SOCKET s)
{
	// Yerel olarak kapatılan soket için Windows FD_CLOSE göndermez; biz de izlemeyi bırakıyoruz.
	// (Önceden kapatılan fd bir sonraki poll'da POLLNVAL verip sahte "Disconnected" üretiyordu.)
	g_watches.erase(std::remove_if(g_watches.begin(), g_watches.end(), [&](const Watch& w) { return w.s == s; }), g_watches.end());
}

void KoWinsockPoll(KoWinsockEventFn fn)
{
	if (g_watches.empty() || !fn)
		return;
	std::vector<pollfd> fds;
	for (const auto& w : g_watches)
		fds.push_back({w.s, POLLIN, 0});
	if (poll(fds.data(), (nfds_t) fds.size(), 0) <= 0)
		return;
	std::vector<Watch> snapshot = g_watches;
	for (size_t i = 0; i < snapshot.size(); ++i)
	{
		const Watch& w = snapshot[i];
		short re       = fds[i].revents;
		// Önce bekleyen veri (karşı taraf kapatmadan hemen önce yanıt göndermiş olabilir),
		// sonra kapanış.
		if (re & POLLIN)
		{
			char peek;
			ssize_t n = recv(w.s, &peek, 1, MSG_PEEK | MSG_DONTWAIT);
			if (n > 0)
			{
				if (w.events & FD_READ)
					fn(w.s, w.hwnd, w.msg, FD_READ);
				continue; // kapanış varsa bir sonraki turda (veri tükenince) bildirilir
			}
			if (n == 0 || (re & (POLLHUP | POLLERR | POLLNVAL)))
			{
				WSAAsyncSelect(w.s, w.hwnd, w.msg, 0);
				if (w.events & FD_CLOSE)
					fn(w.s, w.hwnd, w.msg, FD_CLOSE);
			}
			continue;
		}
		if (re & (POLLHUP | POLLERR | POLLNVAL))
		{
			WSAAsyncSelect(w.s, w.hwnd, w.msg, 0);
			if (w.events & FD_CLOSE)
				fn(w.s, w.hwnd, w.msg, FD_CLOSE);
			continue;
		}
	}
}
