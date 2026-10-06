// D3DX.cpp — D3DX yardımcılarının taşınabilir karşılıkları.
#include "Internal.h"
#include <d3dx9.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

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

namespace
{
// Basit BMP (24/32 bit, sıkıştırmasız, alt-üst / üst-alt) ve TGA (tür 2/3/10/11, 8/24/32 bit) çözücü → RGBA8
bool DecodeBMP(const std::vector<uint8_t>& f, UINT& w, UINT& h, std::vector<uint8_t>& rgba)
{
	if (f.size() < 54 || f[0] != 'B' || f[1] != 'M')
		return false;
	auto u32 = [&](size_t o) { return (uint32_t) f[o] | (uint32_t) f[o + 1] << 8 | (uint32_t) f[o + 2] << 16 | (uint32_t) f[o + 3] << 24; };
	auto u16 = [&](size_t o) { return (uint16_t) (f[o] | f[o + 1] << 8); };
	uint32_t off = u32(10), hdr = u32(14);
	int32_t bw = (int32_t) u32(18), bh = (int32_t) u32(22);
	uint16_t bpp = u16(28);
	uint32_t comp = hdr >= 40 ? u32(30) : 0;
	if (bw <= 0 || bh == 0 || (bpp != 24 && bpp != 32) || (comp != 0 && comp != 3))
		return false;
	bool topDown = bh < 0;
	UINT ah = (UINT) (bh < 0 ? -bh : bh);
	size_t stride = ((size_t) bw * bpp / 8 + 3) & ~(size_t) 3;
	if (off + stride * ah > f.size())
		return false;
	w = (UINT) bw;
	h = ah;
	rgba.resize((size_t) w * h * 4);
	for (UINT y = 0; y < h; ++y)
	{
		const uint8_t* row = f.data() + off + stride * (topDown ? y : (h - 1 - y));
		uint8_t* d         = rgba.data() + (size_t) y * w * 4;
		for (UINT x = 0; x < w; ++x)
		{
			const uint8_t* s = row + (size_t) x * bpp / 8;
			d[x * 4 + 0] = s[2];
			d[x * 4 + 1] = s[1];
			d[x * 4 + 2] = s[0];
			d[x * 4 + 3] = bpp == 32 ? s[3] : 255;
		}
	}
	return true;
}

bool DecodeTGA(const std::vector<uint8_t>& f, UINT& w, UINT& h, std::vector<uint8_t>& rgba)
{
	if (f.size() < 18)
		return false;
	uint8_t idLen = f[0], cmapType = f[1], type = f[2];
	uint16_t tw = (uint16_t) (f[12] | f[13] << 8), th = (uint16_t) (f[14] | f[15] << 8);
	uint8_t bpp = f[16], desc = f[17];
	if (cmapType != 0 || tw == 0 || th == 0)
		return false;
	bool rle = (type == 10 || type == 11);
	if (type != 2 && type != 3 && !rle)
		return false;
	if (bpp != 8 && bpp != 24 && bpp != 32)
		return false;
	size_t pos = 18 + idLen;
	w = tw;
	h = th;
	size_t n = (size_t) w * h, bytes = bpp / 8;
	std::vector<uint8_t> px(n * bytes);
	if (!rle)
	{
		if (pos + n * bytes > f.size())
			return false;
		std::memcpy(px.data(), f.data() + pos, n * bytes);
	}
	else
	{
		size_t i = 0;
		while (i < n)
		{
			if (pos >= f.size())
				return false;
			uint8_t hdr = f[pos++];
			size_t cnt  = (hdr & 0x7F) + 1;
			if (hdr & 0x80)
			{
				if (pos + bytes > f.size())
					return false;
				for (size_t k = 0; k < cnt && i < n; ++k, ++i)
					std::memcpy(px.data() + i * bytes, f.data() + pos, bytes);
				pos += bytes;
			}
			else
			{
				if (pos + cnt * bytes > f.size())
					return false;
				for (size_t k = 0; k < cnt && i < n; ++k, ++i, pos += bytes)
					std::memcpy(px.data() + i * bytes, f.data() + pos, bytes);
			}
		}
	}
	bool topDown = (desc & 0x20) != 0;
	rgba.resize(n * 4);
	for (UINT y = 0; y < h; ++y)
	{
		const uint8_t* row = px.data() + (size_t) (topDown ? y : (h - 1 - y)) * w * bytes;
		uint8_t* d         = rgba.data() + (size_t) y * w * 4;
		for (UINT x = 0; x < w; ++x)
		{
			const uint8_t* s = row + (size_t) x * bytes;
			if (bytes == 1)
			{
				d[x * 4 + 0] = d[x * 4 + 1] = d[x * 4 + 2] = s[0];
				d[x * 4 + 3] = 255;
			}
			else
			{
				d[x * 4 + 0] = s[2];
				d[x * 4 + 1] = s[1];
				d[x * 4 + 2] = s[0];
				d[x * 4 + 3] = bytes == 4 ? s[3] : 255;
			}
		}
	}
	return true;
}
} // namespace

HRESULT D3DXCreateTextureFromFileEx(IDirect3DDevice9* dev, LPCSTR file, UINT, UINT, UINT mipLevels, DWORD usage, D3DFORMAT fmt,
	D3DPOOL pool, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO* info, PALETTEENTRY*, IDirect3DTexture9** out)
{
	// BMP/TGA (misc\terrain_base.bmp, misc\sky\*.bmp, phases.tga ...). JPG için N3Texture kendi yolunu kullanır.
	if (out)
		*out = nullptr;
	if (!dev || !file || !out)
		return D3DERR_INVALIDCALL;
	std::vector<uint8_t> data;
	{
		FILE* fp = std::fopen(file, "rb");
		if (!fp)
		{
			Log("d3d9gles: D3DXCreateTextureFromFileEx dosya açılamadı (%s)", file);
			return D3DERR_NOTFOUND;
		}
		std::fseek(fp, 0, SEEK_END);
		long n = std::ftell(fp);
		std::fseek(fp, 0, SEEK_SET);
		data.resize(n > 0 ? (size_t) n : 0);
		size_t rd = data.empty() ? 0 : std::fread(data.data(), 1, data.size(), fp);
		std::fclose(fp);
		if (rd != data.size())
			return D3DERR_NOTAVAILABLE;
	}
	UINT w = 0, h = 0;
	std::vector<uint8_t> rgba;
	bool ok = DecodeBMP(data, w, h, rgba) || DecodeTGA(data, w, h, rgba);
	if (!ok)
	{
		Log("d3d9gles: D3DXCreateTextureFromFileEx desteklenmeyen biçim (%s)", file);
		return D3DERR_NOTAVAILABLE;
	}
	if (fmt == D3DFMT_UNKNOWN)
		fmt = D3DFMT_A8R8G8B8;
	if (IsCompressedFormat(fmt))
		fmt = D3DFMT_A8R8G8B8;
	UINT levels = (mipLevels == 0 || mipLevels == D3DX_DEFAULT) ? 0 : mipLevels; // 0 = tam zincir
	IDirect3DTexture9* tex = nullptr;
	if (FAILED(dev->CreateTexture(w, h, levels, usage, fmt, pool, &tex, nullptr)) || !tex)
		return D3DERR_NOTAVAILABLE;
	UINT lvCount = tex->GetLevelCount();
	std::vector<uint8_t> cur = rgba, next;
	UINT cw = w, ch = h;
	UINT bpp = FormatBytesPerPixel(fmt);
	for (UINT lv = 0; lv < lvCount; ++lv)
	{
		if (lv > 0)
		{
			UINT nw = std::max(1u, cw / 2), nh = std::max(1u, ch / 2);
			next.resize((size_t) nw * nh * 4);
			ResampleRGBA(cur.data(), cw, ch, next.data(), nw, nh);
			cur.swap(next);
			cw = nw;
			ch = nh;
		}
		D3DLOCKED_RECT lr;
		if (FAILED(tex->LockRect(lv, &lr, nullptr, 0)))
			break;
		std::vector<uint8_t> row((size_t) cw * bpp);
		for (UINT y = 0; y < ch; ++y)
		{
			RGBA8ToPixels(fmt, cw, 1, cur.data() + (size_t) y * cw * 4, row.data());
			std::memcpy(static_cast<uint8_t*>(lr.pBits) + (size_t) y * lr.Pitch, row.data(), row.size());
		}
		tex->UnlockRect(lv);
	}
	if (info)
	{
		info->Width = w;
		info->Height = h;
		info->Depth = 1;
		info->MipLevels = lvCount;
		info->Format = fmt;
		info->ResourceType = D3DRTYPE_TEXTURE;
		info->ImageFileFormat = (data.size() > 1 && data[0] == 'B' && data[1] == 'M') ? D3DXIFF_BMP : D3DXIFF_TGA;
	}
	*out = tex;
	return S_OK;
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
