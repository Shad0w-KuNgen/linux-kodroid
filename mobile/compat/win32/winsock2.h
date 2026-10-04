// winsock2.h uyumluluk sarmalayıcısı: BSD soketleri Winsock adlarıyla sunar.
#pragma once
#if defined(_WIN32)
#include_next <winsock2.h>
#else
#include <windows.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct hostent HOSTENT;
typedef unsigned short u_short;
typedef struct WSAData
{
	WORD wVersion;
	WORD wHighVersion;
} WSAData, WSADATA, *LPWSADATA;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAECONNRESET  ECONNRESET
#define WSAETIMEDOUT   ETIMEDOUT

#define FD_READ    0x01
#define FD_WRITE   0x02
#define FD_OOB     0x04
#define FD_ACCEPT  0x08
#define FD_CONNECT 0x10
#define FD_CLOSE   0x20
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
#define WSAGETSELECTERROR(lParam) HIWORD(lParam)

/// Windows'ta WSAAsyncSelect soket olaylarını pencere mesajı olarak gönderir. Burada soket
/// kaydedilir; platform döngüsü her karede KoWinsockPoll() çağırarak FD_READ/FD_CLOSE üretir.
int WSAAsyncSelect(SOCKET s, HWND hwnd, unsigned msg, long events);
typedef void (*KoWinsockEventFn)(SOCKET s, HWND hwnd, unsigned msg, long event);
void KoWinsockPoll(KoWinsockEventFn fn);

inline int WSAStartup(WORD, WSAData* d) { if (d) { d->wVersion = 0x0101; d->wHighVersion = 0x0202; } return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }
inline int closesocket(SOCKET s) { return close(s); }
inline int ioctlsocket(SOCKET s, long cmd, unsigned long* arg)
{
	if (cmd == FIONBIO)
	{
		int flags = fcntl(s, F_GETFL, 0);
		if (flags < 0) return -1;
		flags = *arg ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
		return fcntl(s, F_SETFL, flags);
	}
	int v = 0;
	int r = ioctl(s, cmd, &v);
	*arg  = (unsigned long) v;
	return r;
}
#endif
