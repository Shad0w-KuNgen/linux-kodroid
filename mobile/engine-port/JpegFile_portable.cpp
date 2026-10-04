// JpegFile_portable.cpp — ekran görüntüsü: GDI yerine OpenGL ES'ten piksel okuma.
// CJpegFile::EncryptJPEG/JpegFromDib'in beklediği 24-bit, alttan yukarı, DWORD hizalı DIB üretir.
#if !defined(_WIN32)

#include <windows.h>
#include <JpegFile/JpegFile.h>

#include <GLES3/gl3.h>

#include <cstring>

namespace
{
constexpr DWORD RowBytes(DWORD w)
{
	return ((w * 24 + 31) / 32) * 4;
}
} // namespace

HANDLE CJpegFile::CopyScreenToDIB(LPRECT lpRect)
{
	GLint vp[4] = {0, 0, 0, 0};
	glGetIntegerv(GL_VIEWPORT, vp);
	int w = vp[2], h = vp[3];
	if (lpRect && lpRect->right > lpRect->left && lpRect->bottom > lpRect->top)
	{
		w = lpRect->right - lpRect->left;
		h = lpRect->bottom - lpRect->top;
	}
	if (w <= 0 || h <= 0)
		return nullptr;

	std::vector<uint8_t> rgba((size_t) w * h * 4);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data()); // GL: alttan yukarı = DIB düzeni

	DWORD rowBytes = RowBytes((DWORD) w);
	size_t total   = sizeof(BITMAPINFOHEADER) + (size_t) rowBytes * h;
	HANDLE hDib    = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, total);
	if (!hDib)
		return nullptr;
	auto* bi            = static_cast<BITMAPINFOHEADER*>(hDib);
	bi->biSize          = sizeof(BITMAPINFOHEADER);
	bi->biWidth         = w;
	bi->biHeight        = h;
	bi->biPlanes        = 1;
	bi->biBitCount      = 24;
	bi->biCompression   = BI_RGB;
	bi->biSizeImage     = rowBytes * (DWORD) h;
	uint8_t* bits       = reinterpret_cast<uint8_t*>(bi + 1);
	for (int y = 0; y < h; ++y)
	{
		const uint8_t* src = rgba.data() + (size_t) y * w * 4;
		uint8_t* dst       = bits + (size_t) y * rowBytes;
		for (int x = 0; x < w; ++x)
		{
			dst[x * 3 + 0] = src[x * 4 + 2];
			dst[x * 3 + 1] = src[x * 4 + 1];
			dst[x * 3 + 2] = src[x * 4 + 0];
		}
	}
	return hDib;
}

HBITMAP CJpegFile::CopyScreenToBitmap(LPRECT)
{
	return nullptr;
}

HANDLE CJpegFile::BitmapToDIB(HBITMAP, HPALETTE)
{
	return nullptr;
}

HANDLE CJpegFile::AllocRoomForDIB(BITMAPINFOHEADER, HBITMAP)
{
	return nullptr;
}

HPALETTE CJpegFile::GetSystemPalette()
{
	return nullptr;
}

int CJpegFile::PalEntriesOnDevice(HDC)
{
	return 0;
}

#endif // !_WIN32
