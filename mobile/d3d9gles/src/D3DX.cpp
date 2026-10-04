// D3DX.cpp — D3DX yardımcılarının taşınabilir karşılıkları.
#include "Internal.h"
#include <d3dx9.h>

#include <algorithm>

using namespace d3d9gles;

namespace
{
// Kutu filtresi ile RGBA8 yeniden boyutlandırma (mip üretimi için yeterli).
void ResampleRGBA(const uint8_t* src, UINT sw, UINT sh, uint8_t* dst, UINT dw, UINT dh)
{
	for (UINT y = 0; y < dh; ++y)
	{
		UINT sy0 = y * sh / dh, sy1 = std::max(sy0 + 1, (y + 1) * sh / dh);
		for (UINT x = 0; x < dw; ++x)
		{
			UINT sx0 = x * sw / dw, sx1 = std::max(sx0 + 1, (x + 1) * sw / dw);
			UINT acc[4] = {0, 0, 0, 0}, n = 0;
			for (UINT yy = sy0; yy < sy1 && yy < sh; ++yy)
				for (UINT xx = sx0; xx < sx1 && xx < sw; ++xx)
				{
					const uint8_t* p = src + (yy * sw + xx) * 4;
					for (int k = 0; k < 4; ++k)
						acc[k] += p[k];
					++n;
				}
			uint8_t* d = dst + (y * dw + x) * 4;
			for (int k = 0; k < 4; ++k)
				d[k] = n ? (uint8_t) (acc[k] / n) : 0;
		}
	}
}
} // namespace

HRESULT D3DXLoadSurfaceFromMemory(IDirect3DSurface9* dst, const PALETTEENTRY*, const RECT* dstRect, const void* srcMem,
	D3DFORMAT srcFmt, UINT srcPitch, const PALETTEENTRY*, const RECT* srcRect, DWORD, D3DCOLOR)
{
	if (!dst || !srcMem || !srcRect)
		return D3DERR_INVALIDCALL;
	D3DSURFACE_DESC dd;
	dst->GetDesc(&dd);
	if (IsCompressedFormat(dd.Format))
		return D3DERR_NOTAVAILABLE; // sıkıştırılmış hedefe yazma desteklenmiyor

	UINT sw = srcRect ? (UINT) (srcRect->right - srcRect->left) : dd.Width;
	UINT sh = srcRect ? (UINT) (srcRect->bottom - srcRect->top) : dd.Height;
	UINT bpp = FormatBytesPerPixel(srcFmt);
	std::vector<uint8_t> packed((size_t) sw * sh * bpp);
	const uint8_t* s = static_cast<const uint8_t*>(srcMem);
	if (srcRect)
		s += srcRect->top * srcPitch + srcRect->left * bpp;
	for (UINT y = 0; y < sh; ++y)
		std::memcpy(packed.data() + (size_t) y * sw * bpp, s + (size_t) y * srcPitch, (size_t) sw * bpp);

	std::vector<uint8_t> rgba;
	PixelsToRGBA8(srcFmt, sw, sh, packed.data(), rgba);

	UINT dw = dstRect ? (UINT) (dstRect->right - dstRect->left) : dd.Width;
	UINT dh = dstRect ? (UINT) (dstRect->bottom - dstRect->top) : dd.Height;
	std::vector<uint8_t> resized;
	const uint8_t* srcRGBA = rgba.data();
	if (dw != sw || dh != sh)
	{
		resized.resize((size_t) dw * dh * 4);
		ResampleRGBA(rgba.data(), sw, sh, resized.data(), dw, dh);
		srcRGBA = resized.data();
	}

	D3DLOCKED_RECT lr;
	if (FAILED(dst->LockRect(&lr, dstRect, 0)))
		return D3DERR_INVALIDCALL;
	UINT dbpp = FormatBytesPerPixel(dd.Format);
	std::vector<uint8_t> row((size_t) dw * dbpp);
	for (UINT y = 0; y < dh; ++y)
	{
		RGBA8ToPixels(dd.Format, dw, 1, srcRGBA + (size_t) y * dw * 4, row.data());
		std::memcpy(static_cast<uint8_t*>(lr.pBits) + (size_t) y * lr.Pitch, row.data(), row.size());
	}
	dst->UnlockRect();
	return S_OK;
}

HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface9* dst, const PALETTEENTRY* dstPal, const RECT* dstRect,
	IDirect3DSurface9* src, const PALETTEENTRY* srcPal, const RECT* srcRect, DWORD filter, D3DCOLOR colorKey)
{
	if (!dst || !src)
		return D3DERR_INVALIDCALL;
	D3DSURFACE_DESC sd;
	src->GetDesc(&sd);
	auto& lv = src->Level();
	// D3DXLoadSurfaceFromMemory kaynak dikdörtgenini zorunlu tutar; null ise tüm yüzey.
	RECT full = {0, 0, (LONG) lv.width, (LONG) lv.height};
	if (srcRect == nullptr)
		srcRect = &full;
	if (IsCompressedFormat(sd.Format))
	{
		std::vector<uint8_t> rgba;
		PixelsToRGBA8(sd.Format, lv.width, lv.height, lv.bytes.data(), rgba);
		return D3DXLoadSurfaceFromMemory(dst, dstPal, dstRect, rgba.data(), D3DFMT_A8B8G8R8, lv.width * 4, srcPal, srcRect, filter, colorKey);
	}
	return D3DXLoadSurfaceFromMemory(dst, dstPal, dstRect, lv.bytes.data(), sd.Format, LevelPitch(sd.Format, lv.width), srcPal, srcRect, filter, colorKey);
}

HRESULT D3DXCreateTextureFromFileEx(IDirect3DDevice9*, LPCSTR file, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO*, PALETTEENTRY*, IDirect3DTexture9** out)
{
	// BMP/JPG/TGA yükleme henüz taşınmadı (oyun dokuları .DXT formatında; bu yol nadiren kullanılır).
	Log("d3d9gles: D3DXCreateTextureFromFileEx desteklenmiyor (%s)", file ? file : "");
	if (out)
		*out = nullptr;
	return D3DERR_NOTAVAILABLE;
}

HRESULT D3DXCreateTextureFromFileExA(IDirect3DDevice9* dev, LPCSTR file, UINT w, UINT h, UINT mip, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, DWORD f, DWORD mf, D3DCOLOR ck, D3DXIMAGE_INFO* info, PALETTEENTRY* pal, IDirect3DTexture9** out)
{
	return D3DXCreateTextureFromFileEx(dev, file, w, h, mip, usage, fmt, pool, f, mf, ck, info, pal, out);
}

HRESULT D3DXSaveSurfaceToFile(LPCSTR, D3DXIMAGE_FILEFORMAT, IDirect3DSurface9*, const PALETTEENTRY*, const RECT*)
{
	return D3DERR_NOTAVAILABLE;
}

const char* D3DXGetErrorString(HRESULT hr)
{
	switch ((DWORD) hr)
	{
		case (DWORD) D3D_OK: return "D3D_OK";
		case (DWORD) D3DERR_INVALIDCALL: return "D3DERR_INVALIDCALL";
		case (DWORD) D3DERR_NOTAVAILABLE: return "D3DERR_NOTAVAILABLE";
		case (DWORD) D3DERR_OUTOFVIDEOMEMORY: return "D3DERR_OUTOFVIDEOMEMORY";
		case (DWORD) D3DERR_DEVICELOST: return "D3DERR_DEVICELOST";
		case (DWORD) D3DERR_DEVICENOTRESET: return "D3DERR_DEVICENOTRESET";
		case (DWORD) E_FAIL: return "E_FAIL";
		case (DWORD) E_OUTOFMEMORY: return "E_OUTOFMEMORY";
		default: return "Unknown D3D error";
	}
}
