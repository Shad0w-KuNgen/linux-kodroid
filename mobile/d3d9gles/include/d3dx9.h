// d3dx9.h — D3DX yardımcı alt kümesi (d3d9gles).
// OpenKO yalnızca D3DXLoadSurfaceFromSurface, D3DXCreateTextureFromFileEx, D3DXGetErrorString
// ve birkaç sabiti kullanıyor. Matris/vektör işleri zaten MathUtils ile yapılıyor.
#ifndef D3D9GLES_D3DX9_H
#define D3D9GLES_D3DX9_H

#include "d3d9.h"

#define D3DX_DEFAULT         ((UINT) -1)
#define D3DX_DEFAULT_NONPOW2 ((UINT) -2)
#define D3DX_FROM_FILE       ((UINT) -3)
#define D3DFMT_FROM_FILE     ((D3DFORMAT) -3)

#define D3DX_FILTER_NONE     (1 << 0)
#define D3DX_FILTER_POINT    (2 << 0)
#define D3DX_FILTER_LINEAR   (3 << 0)
#define D3DX_FILTER_TRIANGLE (4 << 0)
#define D3DX_FILTER_BOX      (5 << 0)
#define D3DX_FILTER_MIRROR_U (1 << 16)
#define D3DX_FILTER_MIRROR_V (2 << 16)
#define D3DX_FILTER_MIRROR_W (4 << 16)
#define D3DX_FILTER_MIRROR   (7 << 16)
#define D3DX_FILTER_DITHER   (1 << 19)
#define D3DX_FILTER_SRGB     (3 << 21)

typedef enum _D3DXIMAGE_FILEFORMAT
{
	D3DXIFF_BMP = 0,
	D3DXIFF_JPG = 1,
	D3DXIFF_TGA = 2,
	D3DXIFF_PNG = 3,
	D3DXIFF_DDS = 4,
	D3DXIFF_PPM = 5,
	D3DXIFF_DIB = 6,
	D3DXIFF_HDR = 7,
	D3DXIFF_PFM = 8,
	D3DXIFF_FORCE_DWORD = 0x7fffffff
} D3DXIMAGE_FILEFORMAT;

typedef struct _D3DXIMAGE_INFO
{
	UINT Width;
	UINT Height;
	UINT Depth;
	UINT MipLevels;
	D3DFORMAT Format;
	D3DRESOURCETYPE ResourceType;
	D3DXIMAGE_FILEFORMAT ImageFileFormat;
} D3DXIMAGE_INFO;

typedef struct _PALETTEENTRY
{
	BYTE peRed, peGreen, peBlue, peFlags;
} PALETTEENTRY;

HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface9* dst, const PALETTEENTRY* dstPal, const RECT* dstRect,
	IDirect3DSurface9* src, const PALETTEENTRY* srcPal, const RECT* srcRect, DWORD filter, D3DCOLOR colorKey);

HRESULT D3DXLoadSurfaceFromMemory(IDirect3DSurface9* dst, const PALETTEENTRY* dstPal, const RECT* dstRect,
	const void* srcMem, D3DFORMAT srcFmt, UINT srcPitch, const PALETTEENTRY* srcPal, const RECT* srcRect, DWORD filter, D3DCOLOR colorKey);

HRESULT D3DXCreateTextureFromFileEx(IDirect3DDevice9* dev, LPCSTR file, UINT w, UINT h, UINT mipLevels, DWORD usage,
	D3DFORMAT fmt, D3DPOOL pool, DWORD filter, DWORD mipFilter, D3DCOLOR colorKey, D3DXIMAGE_INFO* info,
	PALETTEENTRY* pal, IDirect3DTexture9** out);

HRESULT D3DXCreateTextureFromFileExA(IDirect3DDevice9* dev, LPCSTR file, UINT w, UINT h, UINT mipLevels, DWORD usage,
	D3DFORMAT fmt, D3DPOOL pool, DWORD filter, DWORD mipFilter, D3DCOLOR colorKey, D3DXIMAGE_INFO* info,
	PALETTEENTRY* pal, IDirect3DTexture9** out);

HRESULT D3DXSaveSurfaceToFile(LPCSTR file, D3DXIMAGE_FILEFORMAT fmt, IDirect3DSurface9* surf, const PALETTEENTRY* pal, const RECT* rect);

const char* D3DXGetErrorString(HRESULT hr);
inline const char* D3DXGetErrorStringA(HRESULT hr)
{
	return D3DXGetErrorString(hr);
}

#endif // D3D9GLES_D3DX9_H
