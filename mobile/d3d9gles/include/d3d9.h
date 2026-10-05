// d3d9.h — Direct3D 9 arayüzleri (d3d9gles: OpenGL ES 3 üzerinde çalışan uygulama).
//
// N3Base/WarFare kodunun çağırdığı IDirect3D9 / IDirect3DDevice9 / IDirect3DTexture9 /
// IDirect3DSurface9 / IDirect3DVertexBuffer9 / IDirect3DIndexBuffer9 yöntemleri burada
// gerçek COM arayüzleriyle aynı imzalarla sunulur; gövdeleri GLES3 ile yazılmıştır.
#ifndef D3D9GLES_D3D9_H
#define D3D9GLES_D3D9_H

#include "d3d9types.h"

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class IDirect3D9;
class IDirect3DDevice9;
class IDirect3DResource9;
class IDirect3DBaseTexture9;
class IDirect3DTexture9;
class IDirect3DSurface9;
class IDirect3DVertexBuffer9;
class IDirect3DIndexBuffer9;
class IDirect3DVertexShader9;
class IDirect3DPixelShader9;
class IDirect3DStateBlock9;
struct RGNDATA; // kullanılmıyor; imza uyumu için

typedef IDirect3D9* LPDIRECT3D9;
typedef IDirect3DDevice9* LPDIRECT3DDEVICE9;
typedef IDirect3DBaseTexture9* LPDIRECT3DBASETEXTURE9;
typedef IDirect3DTexture9* LPDIRECT3DTEXTURE9;
typedef IDirect3DSurface9* LPDIRECT3DSURFACE9;
typedef IDirect3DVertexBuffer9* LPDIRECT3DVERTEXBUFFER9;
typedef IDirect3DIndexBuffer9* LPDIRECT3DINDEXBUFFER9;
typedef IDirect3DVertexShader9* LPDIRECT3DVERTEXSHADER9;
typedef IDirect3DPixelShader9* LPDIRECT3DPIXELSHADER9;
// D3D8 kalıntısı adlar (OpenKO kodunda hâlâ birkaç yerde var)
typedef IDirect3DDevice9* LPDIRECT3DDEVICE8;
typedef IDirect3DTexture9* LPDIRECT3DTEXTURE8;
typedef IDirect3DVertexBuffer9* LPDIRECT3DVERTEXBUFFER8;
typedef IDirect3DIndexBuffer9* LPDIRECT3DINDEXBUFFER8;

namespace d3d9gles
{
/// Platform katmanının sağladığı kancalar: tampon takası ve çizim alanı boyutu.
struct PlatformHooks
{
	void (*beforePresent)(void* user)                = nullptr; // çizim hedefi hâlâ bağlıyken (kaplama çizimi)
	void (*present)(void* user)                      = nullptr;
	void (*getDrawableSize)(void* user, int* w, int* h) = nullptr;
	void* user                                       = nullptr;
};
void SetPlatformHooks(const PlatformHooks& hooks);
const PlatformHooks& GetPlatformHooks();

/// Arka tampon (bw×bh) fiziksel çizim alanına (dw×dh) en-boy oranı korunarak yerleştirilir
/// (yanlarda/üstte siyah bant). D3D9GLES_STRETCH=1 ile tam ekrana gerilir. Platform katmanı
/// dokunma koordinatlarını mantıksal çözünürlüğe çevirmek için aynı dikdörtgeni kullanır.
void ComputePresentRect(int bw, int bh, int dw, int dh, int* x, int* y, int* w, int* h);

/// Hata/uyarı günlüğü (varsayılan: stderr; Android'de logcat). Geri çağrı verilirse ona da iletilir
/// (platform katmanı Log.txt'ye yazar).
void Log(const char* fmt, ...);
void SetLogCallback(void (*fn)(const char* msg, void* user), void* user);

/// Aygıt bilgisi (GL_VENDOR/RENDERER/VERSION/GLSL, S3TC, azami doku, uzantılar); aygıt
/// oluşturulduktan sonra dolu. Hata raporu ve gpu.txt için.
std::string GetDeviceInfo();

/// Çizim ölçeği: 3D sahne mantıksal çözünürlüğün bu katı kadar bir FBO'ya çizilir, ekrana
/// büyütülür (0.25..1.0). Görünüm alanı/makas/temizleme dikdörtgenleri otomatik ölçeklenir.
void SetRenderScale(float scale);
float GetRenderScale();

/// glGetError denetimi; hata varsa yerini loglar (hız sınırlı). true = hata vardı.
bool CheckGLError(const char* where);
} // namespace d3d9gles

// ---------------------------------------------------------------------------
// Referans sayımı (COM IUnknown'un yerine)
// ---------------------------------------------------------------------------
class IUnknownLite
{
public:
	virtual ~IUnknownLite() = default;
	ULONG AddRef()
	{
		return ++m_refCount;
	}
	ULONG Release()
	{
		ULONG rc = --m_refCount;
		if (rc == 0)
			delete this;
		return rc;
	}
	HRESULT QueryInterface(REFIID, void** out)
	{
		*out = this;
		AddRef();
		return S_OK;
	}

protected:
	ULONG m_refCount = 1;
};

// ---------------------------------------------------------------------------
// Kaynaklar
// ---------------------------------------------------------------------------
class IDirect3DResource9 : public IUnknownLite
{
public:
	explicit IDirect3DResource9(IDirect3DDevice9* dev) : m_device(dev)
	{
	}
	HRESULT GetDevice(IDirect3DDevice9** out)
	{
		*out = m_device;
		return S_OK;
	}
	virtual D3DRESOURCETYPE GetType() = 0;
	DWORD SetPriority(DWORD)
	{
		return 0;
	}
	DWORD GetPriority()
	{
		return 0;
	}
	void PreLoad()
	{
	}

protected:
	IDirect3DDevice9* m_device;
};

/// Tek bir doku seviyesinin (mip) CPU gölgesi ve GL yükleme durumu.
struct D3D9GLES_LevelData
{
	UINT width  = 0;
	UINT height = 0;
	std::vector<uint8_t> bytes; // D3DFORMAT düzeninde (D3D LockRect'in beklediği gibi)
	bool dirty  = false;        // CPU → GL yüklenmesi gerekiyor
};

class IDirect3DSurface9 : public IDirect3DResource9
{
public:
	IDirect3DSurface9(IDirect3DDevice9* dev, IDirect3DTexture9* owner, UINT level, UINT w, UINT h, D3DFORMAT fmt);
	~IDirect3DSurface9() override;
	D3DRESOURCETYPE GetType() override
	{
		return D3DRTYPE_SURFACE;
	}
	HRESULT GetContainer(REFIID, void** out);
	HRESULT GetDesc(D3DSURFACE_DESC* desc);
	HRESULT LockRect(D3DLOCKED_RECT* locked, const RECT* rect, DWORD flags);
	HRESULT UnlockRect();
	HRESULT GetDC(HDC*)
	{
		return D3DERR_INVALIDCALL;
	}
	HRESULT ReleaseDC(HDC)
	{
		return D3DERR_INVALIDCALL;
	}

	D3D9GLES_LevelData& Level();
	D3DFORMAT Format() const
	{
		return m_format;
	}

private:
	IDirect3DTexture9* m_owner; // null ise bağımsız (offscreen plain) yüzey
	UINT m_level;
	D3DFORMAT m_format;
	D3D9GLES_LevelData m_ownData;
	bool m_locked = false;
};

class IDirect3DBaseTexture9 : public IDirect3DResource9
{
public:
	using IDirect3DResource9::IDirect3DResource9;
	virtual DWORD GetLevelCount() = 0;
	DWORD SetLOD(DWORD)
	{
		return 0;
	}
	DWORD GetLOD()
	{
		return 0;
	}
	HRESULT SetAutoGenFilterType(D3DTEXTUREFILTERTYPE)
	{
		return S_OK;
	}
	D3DTEXTUREFILTERTYPE GetAutoGenFilterType()
	{
		return D3DTEXF_LINEAR;
	}
	void GenerateMipSubLevels()
	{
	}
	virtual unsigned GLName() = 0;
	virtual void FlushToGL()  = 0;
};

class IDirect3DTexture9 : public IDirect3DBaseTexture9
{
public:
	IDirect3DTexture9(IDirect3DDevice9* dev, UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool);
	~IDirect3DTexture9() override;

	D3DRESOURCETYPE GetType() override
	{
		return D3DRTYPE_TEXTURE;
	}
	DWORD GetLevelCount() override
	{
		return (DWORD) m_levels.size();
	}
	HRESULT GetLevelDesc(UINT level, D3DSURFACE_DESC* desc);
	HRESULT GetSurfaceLevel(UINT level, IDirect3DSurface9** out);
	HRESULT LockRect(UINT level, D3DLOCKED_RECT* locked, const RECT* rect, DWORD flags);
	HRESULT UnlockRect(UINT level);
	HRESULT AddDirtyRect(const RECT*)
	{
		return S_OK;
	}

	unsigned GLName() override;
	void FlushToGL() override;

	D3DFORMAT Format() const
	{
		return m_format;
	}
	UINT Width() const
	{
		return m_width;
	}
	UINT Height() const
	{
		return m_height;
	}
	D3D9GLES_LevelData& LevelData(UINT level)
	{
		return m_levels[level];
	}
	void MarkDirty(UINT level)
	{
		m_levels[level].dirty = true;
		m_anyDirty            = true;
	}

private:
	void EnsureGLTexture();
	void UploadLevel(UINT level);

	UINT m_width, m_height;
	D3DFORMAT m_format;
	DWORD m_usage;
	D3DPOOL m_pool;
	std::vector<D3D9GLES_LevelData> m_levels;
	std::vector<IDirect3DSurface9*> m_surfaces; // tembel oluşturulan seviye yüzeyleri
	unsigned m_glTex   = 0;
	bool m_anyDirty    = true;
	bool m_glAllocated = false;
};

class IDirect3DVertexBuffer9 : public IDirect3DResource9
{
public:
	IDirect3DVertexBuffer9(IDirect3DDevice9* dev, UINT length, DWORD usage, DWORD fvf, D3DPOOL pool);
	~IDirect3DVertexBuffer9() override;
	D3DRESOURCETYPE GetType() override
	{
		return D3DRTYPE_VERTEXBUFFER;
	}
	HRESULT Lock(UINT offset, UINT size, void** data, DWORD flags);
	HRESULT Unlock();
	HRESULT GetDesc(D3DVERTEXBUFFER_DESC* desc);

	unsigned GLName();
	UINT Length() const
	{
		return (UINT) m_data.size();
	}
	DWORD FVF() const
	{
		return m_fvf;
	}
	const std::vector<uint8_t>& Data() const
	{
		return m_data;
	}

private:
	std::vector<uint8_t> m_data;
	DWORD m_usage, m_fvf;
	D3DPOOL m_pool;
	unsigned m_glBuf = 0;
	bool m_dirty     = true;
	bool m_locked    = false;
};

class IDirect3DIndexBuffer9 : public IDirect3DResource9
{
public:
	IDirect3DIndexBuffer9(IDirect3DDevice9* dev, UINT length, DWORD usage, D3DFORMAT fmt, D3DPOOL pool);
	~IDirect3DIndexBuffer9() override;
	D3DRESOURCETYPE GetType() override
	{
		return D3DRTYPE_INDEXBUFFER;
	}
	HRESULT Lock(UINT offset, UINT size, void** data, DWORD flags);
	HRESULT Unlock();
	HRESULT GetDesc(D3DINDEXBUFFER_DESC* desc);

	unsigned GLName();
	D3DFORMAT Format() const
	{
		return m_format;
	}
	const std::vector<uint8_t>& Data() const
	{
		return m_data;
	}

private:
	std::vector<uint8_t> m_data;
	DWORD m_usage;
	D3DFORMAT m_format;
	D3DPOOL m_pool;
	unsigned m_glBuf = 0;
	bool m_dirty     = true;
	bool m_locked    = false;
};

class IDirect3DVertexShader9 : public IUnknownLite
{
};
class IDirect3DPixelShader9 : public IUnknownLite
{
};
class IDirect3DStateBlock9 : public IUnknownLite
{
public:
	HRESULT Capture()
	{
		return S_OK;
	}
	HRESULT Apply()
	{
		return S_OK;
	}
};

// ---------------------------------------------------------------------------
// Aygıt
// ---------------------------------------------------------------------------
namespace d3d9gles
{
struct DeviceImpl; // Device.cpp içinde
}

class IDirect3DDevice9 : public IUnknownLite
{
public:
	IDirect3DDevice9(IDirect3D9* parent, const D3DPRESENT_PARAMETERS& pp);
	~IDirect3DDevice9() override;

	// --- Aygıt durumu ---
	HRESULT TestCooperativeLevel();
	UINT GetAvailableTextureMem();
	HRESULT EvictManagedResources();
	HRESULT GetDirect3D(IDirect3D9** out);
	HRESULT GetDeviceCaps(D3DCAPS9* caps);
	HRESULT GetDisplayMode(UINT swapChain, D3DDISPLAYMODE* mode);
	HRESULT GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* p);
	HRESULT SetCursorProperties(UINT, UINT, IDirect3DSurface9*)
	{
		return S_OK;
	}
	void SetCursorPosition(int, int, DWORD)
	{
	}
	BOOL ShowCursor(BOOL)
	{
		return TRUE;
	}
	HRESULT Reset(D3DPRESENT_PARAMETERS* pp);
	HRESULT Present(const RECT* src, const RECT* dst, HWND wnd, const RGNDATA* dirty);
	HRESULT GetBackBuffer(UINT swapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE type, IDirect3DSurface9** out);
	HRESULT GetRasterStatus(UINT, D3DRASTER_STATUS* st)
	{
		st->InVBlank = FALSE;
		st->ScanLine = 0;
		return S_OK;
	}
	HRESULT SetDialogBoxMode(BOOL)
	{
		return S_OK;
	}
	void SetGammaRamp(UINT, DWORD, const D3DGAMMARAMP*)
	{
	}
	void GetGammaRamp(UINT, D3DGAMMARAMP*)
	{
	}

	// --- Kaynak oluşturma ---
	HRESULT CreateTexture(UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DTexture9** out, HANDLE* shared);
	HRESULT CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer9** out, HANDLE* shared);
	HRESULT CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DIndexBuffer9** out, HANDLE* shared);
	HRESULT CreateOffscreenPlainSurface(UINT w, UINT h, D3DFORMAT fmt, D3DPOOL pool, IDirect3DSurface9** out, HANDLE* shared);
	HRESULT CreateRenderTarget(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT CreateDepthStencilSurface(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT UpdateSurface(IDirect3DSurface9* src, const RECT* srcRect, IDirect3DSurface9* dst, const POINT* dstPoint);
	HRESULT UpdateTexture(IDirect3DBaseTexture9*, IDirect3DBaseTexture9*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT GetRenderTargetData(IDirect3DSurface9*, IDirect3DSurface9*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT GetFrontBufferData(UINT, IDirect3DSurface9* dst);
	HRESULT StretchRect(IDirect3DSurface9*, const RECT*, IDirect3DSurface9*, const RECT*, D3DTEXTUREFILTERTYPE)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT ColorFill(IDirect3DSurface9* surf, const RECT* rect, D3DCOLOR color);
	HRESULT SetRenderTarget(DWORD, IDirect3DSurface9*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT GetRenderTarget(DWORD, IDirect3DSurface9** out)
	{
		*out = nullptr;
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT SetDepthStencilSurface(IDirect3DSurface9*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT GetDepthStencilSurface(IDirect3DSurface9** out)
	{
		*out = nullptr;
		return D3DERR_NOTAVAILABLE;
	}

	// --- Sahne ---
	HRESULT BeginScene();
	HRESULT EndScene();
	HRESULT Clear(DWORD count, const D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil);

	// --- Sabit işlev durumu ---
	HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* m);
	HRESULT GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX* m);
	HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* m);
	HRESULT SetViewport(const D3DVIEWPORT9* vp);
	HRESULT GetViewport(D3DVIEWPORT9* vp);
	HRESULT SetMaterial(const D3DMATERIAL9* m);
	HRESULT GetMaterial(D3DMATERIAL9* m);
	HRESULT SetLight(DWORD index, const D3DLIGHT9* l);
	HRESULT GetLight(DWORD index, D3DLIGHT9* l);
	HRESULT LightEnable(DWORD index, BOOL enable);
	HRESULT GetLightEnable(DWORD index, BOOL* enable);
	HRESULT SetClipPlane(DWORD, const float*)
	{
		return S_OK;
	}
	HRESULT GetClipPlane(DWORD, float*)
	{
		return S_OK;
	}
	HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
	HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD* value);
	HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE, IDirect3DStateBlock9** out)
	{
		*out = new IDirect3DStateBlock9();
		return S_OK;
	}
	HRESULT BeginStateBlock()
	{
		return S_OK;
	}
	HRESULT EndStateBlock(IDirect3DStateBlock9** out)
	{
		*out = new IDirect3DStateBlock9();
		return S_OK;
	}
	HRESULT SetClipStatus(const D3DCLIPSTATUS9* cs);
	HRESULT GetClipStatus(D3DCLIPSTATUS9* cs);
	HRESULT GetTexture(DWORD stage, IDirect3DBaseTexture9** out);
	HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture9* tex);
	HRESULT GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD* value);
	HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
	HRESULT GetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD* value);
	HRESULT SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);
	HRESULT ValidateDevice(DWORD* passes)
	{
		if (passes)
			*passes = 1;
		return S_OK;
	}
	HRESULT SetPaletteEntries(UINT, const void*)
	{
		return S_OK;
	}
	HRESULT GetPaletteEntries(UINT, void*)
	{
		return S_OK;
	}
	HRESULT SetCurrentTexturePalette(UINT)
	{
		return S_OK;
	}
	HRESULT GetCurrentTexturePalette(UINT* p)
	{
		*p = 0;
		return S_OK;
	}
	HRESULT SetScissorRect(const RECT* rect);
	HRESULT GetScissorRect(RECT* rect);
	HRESULT SetSoftwareVertexProcessing(BOOL)
	{
		return S_OK;
	}
	BOOL GetSoftwareVertexProcessing()
	{
		return FALSE;
	}
	HRESULT SetNPatchMode(float)
	{
		return S_OK;
	}
	float GetNPatchMode()
	{
		return 0.0f;
	}

	// --- Çizim ---
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primCount);
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minIndex, UINT numVertices, UINT startIndex, UINT primCount);
	HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT primCount, const void* vertexData, UINT stride);
	HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type, UINT minIndex, UINT numVertices, UINT primCount, const void* indexData, D3DFORMAT indexFormat, const void* vertexData, UINT stride);
	HRESULT ProcessVertices(UINT, UINT, UINT, IDirect3DVertexBuffer9*, void*, DWORD)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT CreateVertexDeclaration(const void*, void**)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT SetVertexDeclaration(void*)
	{
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT SetFVF(DWORD fvf);
	HRESULT GetFVF(DWORD* fvf);
	HRESULT CreateVertexShader(const DWORD*, IDirect3DVertexShader9** out)
	{
		*out = nullptr;
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT SetVertexShader(IDirect3DVertexShader9*)
	{
		return S_OK;
	}
	HRESULT GetVertexShader(IDirect3DVertexShader9** out)
	{
		*out = nullptr;
		return S_OK;
	}
	HRESULT SetVertexShaderConstantF(UINT, const float*, UINT)
	{
		return S_OK;
	}
	HRESULT SetStreamSource(UINT stream, IDirect3DVertexBuffer9* vb, UINT offset, UINT stride);
	HRESULT GetStreamSource(UINT stream, IDirect3DVertexBuffer9** vb, UINT* offset, UINT* stride);
	HRESULT SetStreamSourceFreq(UINT, UINT)
	{
		return S_OK;
	}
	HRESULT SetIndices(IDirect3DIndexBuffer9* ib);
	HRESULT GetIndices(IDirect3DIndexBuffer9** ib);
	HRESULT CreatePixelShader(const DWORD*, IDirect3DPixelShader9** out)
	{
		*out = nullptr;
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT SetPixelShader(IDirect3DPixelShader9*)
	{
		return S_OK;
	}
	HRESULT GetPixelShader(IDirect3DPixelShader9** out)
	{
		*out = nullptr;
		return S_OK;
	}

	// d3d9gles iç kullanım
	d3d9gles::DeviceImpl& Impl()
	{
		return *m_impl;
	}
	bool SupportsS3TC() const;

private:
	IDirect3D9* m_parent;
	std::unique_ptr<d3d9gles::DeviceImpl> m_impl;
};

// ---------------------------------------------------------------------------
// IDirect3D9
// ---------------------------------------------------------------------------
class IDirect3D9 : public IUnknownLite
{
public:
	IDirect3D9();
	HRESULT RegisterSoftwareDevice(void*)
	{
		return S_OK;
	}
	UINT GetAdapterCount()
	{
		return 1;
	}
	HRESULT GetAdapterIdentifier(UINT adapter, DWORD flags, D3DADAPTER_IDENTIFIER9* id);
	UINT GetAdapterModeCount(UINT adapter, D3DFORMAT fmt);
	HRESULT EnumAdapterModes(UINT adapter, D3DFORMAT fmt, UINT mode, D3DDISPLAYMODE* out);
	HRESULT GetAdapterDisplayMode(UINT adapter, D3DDISPLAYMODE* out);
	HRESULT CheckDeviceType(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL)
	{
		return D3D_OK;
	}
	HRESULT CheckDeviceFormat(UINT adapter, D3DDEVTYPE devType, D3DFORMAT adapterFmt, DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT checkFmt);
	HRESULT CheckDeviceMultiSampleType(UINT, D3DDEVTYPE, D3DFORMAT, BOOL, D3DMULTISAMPLE_TYPE type, DWORD* q)
	{
		if (q)
			*q = 0;
		return type == D3DMULTISAMPLE_NONE ? D3D_OK : D3DERR_NOTAVAILABLE;
	}
	HRESULT CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT dsFmt);
	HRESULT CheckDeviceFormatConversion(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT)
	{
		return D3D_OK;
	}
	HRESULT GetDeviceCaps(UINT adapter, D3DDEVTYPE devType, D3DCAPS9* caps);
	HWND GetAdapterMonitor(UINT)
	{
		return nullptr;
	}
	HRESULT CreateDevice(UINT adapter, D3DDEVTYPE devType, HWND focus, DWORD behavior, D3DPRESENT_PARAMETERS* pp, IDirect3DDevice9** out);
};

IDirect3D9* Direct3DCreate9(UINT sdkVersion);

#endif // D3D9GLES_D3D9_H
