#include <cstdio>
// Resources.cpp — doku, yüzey, köşe/indeks tamponları ve piksel formatı dönüşümleri.
#include "Internal.h"

#include <algorithm>
#include <cstdlib>

namespace d3d9gles
{
bool IsCompressedFormat(D3DFORMAT fmt)
{
	return fmt == D3DFMT_DXT1 || fmt == D3DFMT_DXT2 || fmt == D3DFMT_DXT3 || fmt == D3DFMT_DXT4 || fmt == D3DFMT_DXT5;
}

UINT FormatBytesPerPixel(D3DFORMAT fmt)
{
	switch (fmt)
	{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8B8G8R8:
		case D3DFMT_X8B8G8R8:
		case D3DFMT_D32:
		case D3DFMT_D24S8:
		case D3DFMT_D24X8:
		case D3DFMT_D24X4S4:
		case D3DFMT_INDEX32: return 4;
		case D3DFMT_R8G8B8: return 3;
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_A4R4G4B4:
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A8L8:
		case D3DFMT_D16:
		case D3DFMT_D15S1:
		case D3DFMT_INDEX16: return 2;
		case D3DFMT_A8:
		case D3DFMT_L8:
		case D3DFMT_P8:
		case D3DFMT_A4L4: return 1;
		default: return 4;
	}
}

UINT LevelPitch(D3DFORMAT fmt, UINT w)
{
	if (IsCompressedFormat(fmt))
	{
		UINT blocks = (w + 3) / 4;
		return blocks * (fmt == D3DFMT_DXT1 ? 8 : 16);
	}
	return w * FormatBytesPerPixel(fmt);
}

UINT LevelByteSize(D3DFORMAT fmt, UINT w, UINT h)
{
	if (IsCompressedFormat(fmt))
	{
		UINT bw = (w + 3) / 4, bh = (h + 3) / 4;
		return bw * bh * (fmt == D3DFMT_DXT1 ? 8 : 16);
	}
	return LevelPitch(fmt, w) * h;
}

// ---------------------------------------------------------------------------
// DXT çözücü
// ---------------------------------------------------------------------------
namespace
{
inline void Unpack565(uint16_t c, uint8_t* rgb)
{
	uint8_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
	rgb[0]    = (uint8_t) ((r << 3) | (r >> 2));
	rgb[1]    = (uint8_t) ((g << 2) | (g >> 4));
	rgb[2]    = (uint8_t) ((b << 3) | (b >> 2));
}

void DecodeColorBlock(const uint8_t* block, bool dxt1, uint8_t out[16][4])
{
	uint16_t c0 = (uint16_t) (block[0] | (block[1] << 8));
	uint16_t c1 = (uint16_t) (block[2] | (block[3] << 8));
	uint8_t pal[4][4];
	Unpack565(c0, pal[0]);
	Unpack565(c1, pal[1]);
	pal[0][3] = pal[1][3] = 255;
	if (!dxt1 || c0 > c1)
	{
		for (int k = 0; k < 3; ++k)
		{
			pal[2][k] = (uint8_t) ((2 * pal[0][k] + pal[1][k] + 1) / 3);
			pal[3][k] = (uint8_t) ((pal[0][k] + 2 * pal[1][k] + 1) / 3);
		}
		pal[2][3] = pal[3][3] = 255;
	}
	else
	{
		for (int k = 0; k < 3; ++k)
		{
			pal[2][k] = (uint8_t) ((pal[0][k] + pal[1][k]) / 2);
			pal[3][k] = 0;
		}
		pal[2][3] = 255;
		pal[3][3] = 0;
	}
	uint32_t idx = (uint32_t) (block[4] | (block[5] << 8) | (block[6] << 16) | ((uint32_t) block[7] << 24));
	for (int i = 0; i < 16; ++i)
	{
		int p = (idx >> (2 * i)) & 3;
		out[i][0] = pal[p][0];
		out[i][1] = pal[p][1];
		out[i][2] = pal[p][2];
		out[i][3] = pal[p][3];
	}
}
} // namespace

void DecodeDXT(D3DFORMAT fmt, UINT w, UINT h, const uint8_t* src, uint8_t* dst)
{
	bool dxt1       = (fmt == D3DFMT_DXT1);
	bool dxt3       = (fmt == D3DFMT_DXT2 || fmt == D3DFMT_DXT3);
	UINT blockSize  = dxt1 ? 8 : 16;
	UINT bw         = (w + 3) / 4, bh = (h + 3) / 4;
	for (UINT by = 0; by < bh; ++by)
	{
		for (UINT bx = 0; bx < bw; ++bx)
		{
			const uint8_t* block = src + (by * bw + bx) * blockSize;
			uint8_t px[16][4];
			uint8_t alpha[16];
			const uint8_t* colorBlock = block;
			if (!dxt1)
			{
				colorBlock = block + 8;
				if (dxt3)
				{
					for (int i = 0; i < 16; ++i)
					{
						uint8_t a4 = (block[i / 2] >> ((i & 1) * 4)) & 0xF;
						alpha[i]   = (uint8_t) (a4 * 17);
					}
				}
				else
				{
					uint8_t a0 = block[0], a1 = block[1];
					uint8_t ap[8];
					ap[0] = a0;
					ap[1] = a1;
					if (a0 > a1)
						for (int k = 1; k < 7; ++k)
							ap[k + 1] = (uint8_t) (((7 - k) * a0 + k * a1) / 7);
					else
					{
						for (int k = 1; k < 5; ++k)
							ap[k + 1] = (uint8_t) (((5 - k) * a0 + k * a1) / 5);
						ap[6] = 0;
						ap[7] = 255;
					}
					uint64_t bits = 0;
					for (int k = 0; k < 6; ++k)
						bits |= (uint64_t) block[2 + k] << (8 * k);
					for (int i = 0; i < 16; ++i)
						alpha[i] = ap[(bits >> (3 * i)) & 7];
				}
			}
			DecodeColorBlock(colorBlock, dxt1, px);
			for (int i = 0; i < 16; ++i)
			{
				UINT x = bx * 4 + (i & 3), y = by * 4 + (i >> 2);
				if (x >= w || y >= h)
					continue;
				uint8_t* d = dst + (y * w + x) * 4;
				d[0]       = px[i][0];
				d[1]       = px[i][1];
				d[2]       = px[i][2];
				d[3]       = dxt1 ? px[i][3] : alpha[i];
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Piksel dönüşümleri
// ---------------------------------------------------------------------------
void PixelsToRGBA8(D3DFORMAT fmt, UINT w, UINT h, const uint8_t* src, std::vector<uint8_t>& rgba)
{
	rgba.resize((size_t) w * h * 4);
	if (IsCompressedFormat(fmt))
	{
		DecodeDXT(fmt, w, h, src, rgba.data());
		return;
	}
	UINT n = w * h;
	switch (fmt)
	{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			for (UINT i = 0; i < n; ++i)
			{
				rgba[i * 4 + 0] = src[i * 4 + 2];
				rgba[i * 4 + 1] = src[i * 4 + 1];
				rgba[i * 4 + 2] = src[i * 4 + 0];
				rgba[i * 4 + 3] = fmt == D3DFMT_A8R8G8B8 ? src[i * 4 + 3] : 255;
			}
			break;
		case D3DFMT_A8B8G8R8:
		case D3DFMT_X8B8G8R8:
			for (UINT i = 0; i < n; ++i)
			{
				rgba[i * 4 + 0] = src[i * 4 + 0];
				rgba[i * 4 + 1] = src[i * 4 + 1];
				rgba[i * 4 + 2] = src[i * 4 + 2];
				rgba[i * 4 + 3] = fmt == D3DFMT_A8B8G8R8 ? src[i * 4 + 3] : 255;
			}
			break;
		case D3DFMT_R8G8B8:
			for (UINT i = 0; i < n; ++i)
			{
				rgba[i * 4 + 0] = src[i * 3 + 2];
				rgba[i * 4 + 1] = src[i * 3 + 1];
				rgba[i * 4 + 2] = src[i * 3 + 0];
				rgba[i * 4 + 3] = 255;
			}
			break;
		case D3DFMT_R5G6B5:
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (src[i * 2] | (src[i * 2 + 1] << 8));
				Unpack565(c, &rgba[i * 4]);
				rgba[i * 4 + 3] = 255;
			}
			break;
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (src[i * 2] | (src[i * 2 + 1] << 8));
				uint8_t r = (c >> 10) & 31, g = (c >> 5) & 31, b = c & 31;
				rgba[i * 4 + 0] = (uint8_t) ((r << 3) | (r >> 2));
				rgba[i * 4 + 1] = (uint8_t) ((g << 3) | (g >> 2));
				rgba[i * 4 + 2] = (uint8_t) ((b << 3) | (b >> 2));
				rgba[i * 4 + 3] = (fmt == D3DFMT_A1R5G5B5) ? ((c & 0x8000) ? 255 : 0) : 255;
			}
			break;
		case D3DFMT_A4R4G4B4:
		case D3DFMT_X4R4G4B4:
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (src[i * 2] | (src[i * 2 + 1] << 8));
				uint8_t a = (c >> 12) & 15, r = (c >> 8) & 15, g = (c >> 4) & 15, b = c & 15;
				rgba[i * 4 + 0] = (uint8_t) (r * 17);
				rgba[i * 4 + 1] = (uint8_t) (g * 17);
				rgba[i * 4 + 2] = (uint8_t) (b * 17);
				rgba[i * 4 + 3] = (fmt == D3DFMT_A4R4G4B4) ? (uint8_t) (a * 17) : 255;
			}
			break;
		case D3DFMT_A8:
			for (UINT i = 0; i < n; ++i)
			{
				rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = 255;
				rgba[i * 4 + 3] = src[i];
			}
			break;
		case D3DFMT_L8:
			for (UINT i = 0; i < n; ++i)
			{
				rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = src[i];
				rgba[i * 4 + 3] = 255;
			}
			break;
		case D3DFMT_A8L8:
			for (UINT i = 0; i < n; ++i)
			{
				rgba[i * 4 + 0] = rgba[i * 4 + 1] = rgba[i * 4 + 2] = src[i * 2];
				rgba[i * 4 + 3] = src[i * 2 + 1];
			}
			break;
		default:
			std::memset(rgba.data(), 255, rgba.size());
			break;
	}
}

void RGBA8ToPixels(D3DFORMAT fmt, UINT w, UINT h, const uint8_t* rgba, uint8_t* dst)
{
	UINT n = w * h;
	switch (fmt)
	{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			for (UINT i = 0; i < n; ++i)
			{
				dst[i * 4 + 0] = rgba[i * 4 + 2];
				dst[i * 4 + 1] = rgba[i * 4 + 1];
				dst[i * 4 + 2] = rgba[i * 4 + 0];
				dst[i * 4 + 3] = fmt == D3DFMT_A8R8G8B8 ? rgba[i * 4 + 3] : 255;
			}
			break;
		case D3DFMT_R8G8B8:
			for (UINT i = 0; i < n; ++i)
			{
				dst[i * 3 + 0] = rgba[i * 4 + 2];
				dst[i * 3 + 1] = rgba[i * 4 + 1];
				dst[i * 3 + 2] = rgba[i * 4 + 0];
			}
			break;
		case D3DFMT_R5G6B5:
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (((rgba[i * 4] >> 3) << 11) | ((rgba[i * 4 + 1] >> 2) << 5) | (rgba[i * 4 + 2] >> 3));
				dst[i * 2]     = (uint8_t) (c & 0xFF);
				dst[i * 2 + 1] = (uint8_t) (c >> 8);
			}
			break;
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (((rgba[i * 4 + 3] >= 128 || fmt == D3DFMT_X1R5G5B5) ? 0x8000 : 0) | ((rgba[i * 4] >> 3) << 10) | ((rgba[i * 4 + 1] >> 3) << 5) | (rgba[i * 4 + 2] >> 3));
				dst[i * 2]     = (uint8_t) (c & 0xFF);
				dst[i * 2 + 1] = (uint8_t) (c >> 8);
			}
			break;
		case D3DFMT_A4R4G4B4:
		case D3DFMT_X4R4G4B4:
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (((rgba[i * 4 + 3] >> 4) << 12) | ((rgba[i * 4] >> 4) << 8) | ((rgba[i * 4 + 1] >> 4) << 4) | (rgba[i * 4 + 2] >> 4));
				dst[i * 2]     = (uint8_t) (c & 0xFF);
				dst[i * 2 + 1] = (uint8_t) (c >> 8);
			}
			break;
		case D3DFMT_A8:
			for (UINT i = 0; i < n; ++i)
				dst[i] = rgba[i * 4 + 3];
			break;
		case D3DFMT_L8:
			for (UINT i = 0; i < n; ++i)
				dst[i] = rgba[i * 4];
			break;
		default:
			break;
	}
}

bool ConvertLevelToGL(D3DFORMAT fmt, UINT w, UINT h, const std::vector<uint8_t>& src, bool s3tcOK,
	std::vector<uint8_t>& out, GLenum* outInternal, GLenum* outFormat, GLenum* outType, bool* outCompressed)
{
	*outCompressed = false;
	*outType       = GL_UNSIGNED_BYTE;
	UINT n         = w * h;
	switch (fmt)
	{
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			if (s3tcOK)
			{
				*outCompressed = true;
				*outInternal   = fmt == D3DFMT_DXT1 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
								 : (fmt == D3DFMT_DXT2 || fmt == D3DFMT_DXT3) ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
																			: GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
				*outFormat     = *outInternal;
				out            = src;
				return true;
			}
			PixelsToRGBA8(fmt, w, h, src.data(), out);
			*outInternal = GL_RGBA8;
			*outFormat   = GL_RGBA;
			return true;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A8B8G8R8:
		case D3DFMT_X8B8G8R8:
		case D3DFMT_R8G8B8:
			PixelsToRGBA8(fmt, w, h, src.data(), out);
			*outInternal = GL_RGBA8;
			*outFormat   = GL_RGBA;
			return true;
		case D3DFMT_R5G6B5:
			out = src;
			*outInternal = GL_RGB565;
			*outFormat   = GL_RGB;
			*outType     = GL_UNSIGNED_SHORT_5_6_5;
			return true;
		case D3DFMT_A1R5G5B5:
		case D3DFMT_X1R5G5B5:
			out.resize((size_t) n * 2);
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (src[i * 2] | (src[i * 2 + 1] << 8));
				uint16_t a = (fmt == D3DFMT_A1R5G5B5) ? ((c >> 15) & 1) : 1;
				uint16_t o = (uint16_t) ((((c >> 10) & 31) << 11) | (((c >> 5) & 31) << 6) | ((c & 31) << 1) | a);
				out[i * 2]     = (uint8_t) (o & 0xFF);
				out[i * 2 + 1] = (uint8_t) (o >> 8);
			}
			*outInternal = GL_RGB5_A1;
			*outFormat   = GL_RGBA;
			*outType     = GL_UNSIGNED_SHORT_5_5_5_1;
			return true;
		case D3DFMT_A4R4G4B4:
		case D3DFMT_X4R4G4B4:
			out.resize((size_t) n * 2);
			for (UINT i = 0; i < n; ++i)
			{
				uint16_t c = (uint16_t) (src[i * 2] | (src[i * 2 + 1] << 8));
				uint16_t a = (fmt == D3DFMT_A4R4G4B4) ? ((c >> 12) & 15) : 15;
				uint16_t o = (uint16_t) ((((c >> 8) & 15) << 12) | (((c >> 4) & 15) << 8) | ((c & 15) << 4) | a);
				out[i * 2]     = (uint8_t) (o & 0xFF);
				out[i * 2 + 1] = (uint8_t) (o >> 8);
			}
			*outInternal = GL_RGBA4;
			*outFormat   = GL_RGBA;
			*outType     = GL_UNSIGNED_SHORT_4_4_4_4;
			return true;
		case D3DFMT_A8:
			out = src;
			*outInternal = GL_ALPHA;
			*outFormat   = GL_ALPHA;
			return true;
		case D3DFMT_L8:
			out = src;
			*outInternal = GL_LUMINANCE;
			*outFormat   = GL_LUMINANCE;
			return true;
		case D3DFMT_A8L8:
			out = src;
			*outInternal = GL_LUMINANCE_ALPHA;
			*outFormat   = GL_LUMINANCE_ALPHA;
			return true;
		default:
			return false;
	}
}

} // namespace d3d9gles

using namespace d3d9gles;

// ---------------------------------------------------------------------------
// IDirect3DSurface9
// ---------------------------------------------------------------------------
IDirect3DSurface9::IDirect3DSurface9(IDirect3DDevice9* dev, IDirect3DTexture9* owner, UINT level, UINT w, UINT h, D3DFORMAT fmt) :
	IDirect3DResource9(dev), m_owner(owner), m_level(level), m_format(fmt)
{
	if (m_owner == nullptr)
	{
		m_ownData.width  = w;
		m_ownData.height = h;
		m_ownData.bytes.assign(LevelByteSize(fmt, w, h), 0);
	}
}

IDirect3DSurface9::~IDirect3DSurface9() = default;

D3D9GLES_LevelData& IDirect3DSurface9::Level()
{
	return m_owner ? m_owner->LevelData(m_level) : m_ownData;
}

HRESULT IDirect3DSurface9::GetContainer(REFIID, void** out)
{
	*out = m_owner;
	if (m_owner)
		m_owner->AddRef();
	return m_owner ? S_OK : D3DERR_INVALIDCALL;
}

HRESULT IDirect3DSurface9::GetDesc(D3DSURFACE_DESC* desc)
{
	auto& lv = Level();
	*desc    = {};
	desc->Format = m_format;
	desc->Type   = D3DRTYPE_SURFACE;
	desc->Pool   = D3DPOOL_MANAGED;
	desc->MultiSampleType = D3DMULTISAMPLE_NONE;
	desc->Width  = lv.width;
	desc->Height = lv.height;
	return S_OK;
}

HRESULT IDirect3DSurface9::LockRect(D3DLOCKED_RECT* locked, const RECT* rect, DWORD)
{
	auto& lv = Level();
	if (m_locked)
		return D3DERR_INVALIDCALL;
	m_locked      = true;
	locked->Pitch = (INT) LevelPitch(m_format, lv.width);
	uint8_t* base = lv.bytes.data();
	if (rect && !IsCompressedFormat(m_format))
		base += rect->top * locked->Pitch + rect->left * FormatBytesPerPixel(m_format);
	locked->pBits = base;
	return S_OK;
}

HRESULT IDirect3DSurface9::UnlockRect()
{
	if (!m_locked)
		return D3DERR_INVALIDCALL;
	m_locked = false;
	if (m_owner)
		m_owner->MarkDirty(m_level);
	return S_OK;
}

// ---------------------------------------------------------------------------
// IDirect3DTexture9
// ---------------------------------------------------------------------------
IDirect3DTexture9::IDirect3DTexture9(IDirect3DDevice9* dev, UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool) :
	IDirect3DBaseTexture9(dev), m_width(w), m_height(h), m_format(fmt), m_usage(usage), m_pool(pool)
{
	if (levels == 0)
	{
		levels = 1;
		for (UINT lw = w, lh = h; lw > 1 || lh > 1; lw = std::max(1u, lw / 2), lh = std::max(1u, lh / 2))
			++levels;
	}
	m_levels.resize(levels);
	m_surfaces.assign(levels, nullptr);
	UINT lw = w, lh = h;
	for (UINT i = 0; i < levels; ++i)
	{
		m_levels[i].width  = lw;
		m_levels[i].height = lh;
		m_levels[i].bytes.assign(LevelByteSize(fmt, lw, lh), 0);
		m_levels[i].dirty = true;
		lw = std::max(1u, lw / 2);
		lh = std::max(1u, lh / 2);
	}
}

IDirect3DTexture9::~IDirect3DTexture9()
{
	for (auto* s : m_surfaces)
		if (s)
			s->Release();
	if (m_glTex)
		glDeleteTextures(1, &m_glTex);
}

HRESULT IDirect3DTexture9::GetLevelDesc(UINT level, D3DSURFACE_DESC* desc)
{
	if (level >= m_levels.size())
		return D3DERR_INVALIDCALL;
	*desc        = {};
	desc->Format = m_format;
	desc->Type   = D3DRTYPE_SURFACE;
	desc->Usage  = m_usage;
	desc->Pool   = m_pool;
	desc->MultiSampleType = D3DMULTISAMPLE_NONE;
	desc->Width  = m_levels[level].width;
	desc->Height = m_levels[level].height;
	return S_OK;
}

HRESULT IDirect3DTexture9::GetSurfaceLevel(UINT level, IDirect3DSurface9** out)
{
	if (level >= m_levels.size())
		return D3DERR_INVALIDCALL;
	if (!m_surfaces[level])
		m_surfaces[level] = new IDirect3DSurface9(m_device, this, level, m_levels[level].width, m_levels[level].height, m_format);
	m_surfaces[level]->AddRef();
	*out = m_surfaces[level];
	return S_OK;
}

HRESULT IDirect3DTexture9::LockRect(UINT level, D3DLOCKED_RECT* locked, const RECT* rect, DWORD flags)
{
	if (level >= m_levels.size())
		return D3DERR_INVALIDCALL;
	IDirect3DSurface9* s = nullptr;
	GetSurfaceLevel(level, &s);
	HRESULT hr = s->LockRect(locked, rect, flags);
	s->Release();
	return hr;
}

HRESULT IDirect3DTexture9::UnlockRect(UINT level)
{
	if (level >= m_levels.size())
		return D3DERR_INVALIDCALL;
	IDirect3DSurface9* s = nullptr;
	GetSurfaceLevel(level, &s);
	HRESULT hr = s->UnlockRect();
	s->Release();
	return hr;
}

void IDirect3DTexture9::EnsureGLTexture()
{
	if (m_glTex == 0)
		glGenTextures(1, &m_glTex);
}

void IDirect3DTexture9::UploadLevel(UINT level)
{
	auto& lv = m_levels[level];
	std::vector<uint8_t> glData;
	GLenum internal = GL_RGBA8, format = GL_RGBA, type = GL_UNSIGNED_BYTE;
	bool compressed = false;
	bool s3tc       = m_device->SupportsS3TC();
	if (!ConvertLevelToGL(m_format, lv.width, lv.height, lv.bytes, s3tc, glData, &internal, &format, &type, &compressed))
	{
		Log("d3d9gles: desteklenmeyen doku formatı 0x%X", (unsigned) m_format);
		return;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (compressed)
		glCompressedTexImage2D(GL_TEXTURE_2D, (GLint) level, internal, (GLsizei) lv.width, (GLsizei) lv.height, 0, (GLsizei) glData.size(), glData.data());
	else
		glTexImage2D(GL_TEXTURE_2D, (GLint) level, (GLint) internal, (GLsizei) lv.width, (GLsizei) lv.height, 0, format, type, glData.data());
	{
		char where[96];
		snprintf(where, sizeof(where), "%s %ux%u seviye %u fmt 0x%X", compressed ? "glCompressedTexImage2D" : "glTexImage2D",
			(unsigned) lv.width, (unsigned) lv.height, (unsigned) level, (unsigned) m_format);
		d3d9gles::CheckGLError(where);
	}
	lv.dirty = false;
}

void IDirect3DTexture9::FlushToGL()
{
	EnsureGLTexture();
	if (!m_anyDirty && m_glAllocated)
		return;
	glBindTexture(GL_TEXTURE_2D, m_glTex);
	for (UINT i = 0; i < m_levels.size(); ++i)
		if (m_levels[i].dirty || !m_glAllocated)
			UploadLevel(i);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (GLint) m_levels.size() - 1);
	m_glAllocated = true;
	m_anyDirty    = false;
}

unsigned IDirect3DTexture9::GLName()
{
	FlushToGL();
	return m_glTex;
}

// ---------------------------------------------------------------------------
// Tamponlar
// ---------------------------------------------------------------------------
IDirect3DVertexBuffer9::IDirect3DVertexBuffer9(IDirect3DDevice9* dev, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool) :
	IDirect3DResource9(dev), m_data(length, 0), m_usage(usage), m_fvf(fvf), m_pool(pool)
{
}

IDirect3DVertexBuffer9::~IDirect3DVertexBuffer9()
{
	if (m_glBuf)
		glDeleteBuffers(1, &m_glBuf);
}

HRESULT IDirect3DVertexBuffer9::Lock(UINT offset, UINT size, void** data, DWORD)
{
	if (m_locked || offset > m_data.size())
		return D3DERR_INVALIDCALL;
	(void) size;
	m_locked = true;
	*data    = m_data.data() + offset;
	return S_OK;
}

HRESULT IDirect3DVertexBuffer9::Unlock()
{
	if (!m_locked)
		return D3DERR_INVALIDCALL;
	m_locked = false;
	m_dirty  = true;
	return S_OK;
}

HRESULT IDirect3DVertexBuffer9::GetDesc(D3DVERTEXBUFFER_DESC* desc)
{
	desc->Format = D3DFMT_VERTEXDATA;
	desc->Type   = D3DRTYPE_VERTEXBUFFER;
	desc->Usage  = m_usage;
	desc->Pool   = m_pool;
	desc->Size   = (UINT) m_data.size();
	desc->FVF    = m_fvf;
	return S_OK;
}

unsigned IDirect3DVertexBuffer9::GLName()
{
	if (!m_glBuf)
		glGenBuffers(1, &m_glBuf);
	if (m_dirty)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_glBuf);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) m_data.size(), m_data.data(), (m_usage & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		m_dirty = false;
	}
	return m_glBuf;
}

IDirect3DIndexBuffer9::IDirect3DIndexBuffer9(IDirect3DDevice9* dev, UINT length, DWORD usage, D3DFORMAT fmt, D3DPOOL pool) :
	IDirect3DResource9(dev), m_data(length, 0), m_usage(usage), m_format(fmt), m_pool(pool)
{
}

IDirect3DIndexBuffer9::~IDirect3DIndexBuffer9()
{
	if (m_glBuf)
		glDeleteBuffers(1, &m_glBuf);
}

HRESULT IDirect3DIndexBuffer9::Lock(UINT offset, UINT size, void** data, DWORD)
{
	if (m_locked || offset > m_data.size())
		return D3DERR_INVALIDCALL;
	(void) size;
	m_locked = true;
	*data    = m_data.data() + offset;
	return S_OK;
}

HRESULT IDirect3DIndexBuffer9::Unlock()
{
	if (!m_locked)
		return D3DERR_INVALIDCALL;
	m_locked = false;
	m_dirty  = true;
	return S_OK;
}

HRESULT IDirect3DIndexBuffer9::GetDesc(D3DINDEXBUFFER_DESC* desc)
{
	desc->Format = m_format;
	desc->Type   = D3DRTYPE_INDEXBUFFER;
	desc->Usage  = m_usage;
	desc->Pool   = m_pool;
	desc->Size   = (UINT) m_data.size();
	return S_OK;
}

unsigned IDirect3DIndexBuffer9::GLName()
{
	if (!m_glBuf)
		glGenBuffers(1, &m_glBuf);
	if (m_dirty)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glBuf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) m_data.size(), m_data.data(), (m_usage & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
		m_dirty = false;
	}
	return m_glBuf;
}
