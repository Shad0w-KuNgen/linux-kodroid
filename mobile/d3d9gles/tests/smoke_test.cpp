// smoke_test.cpp — d3d9gles için başsız (EGL pbuffer) doğrulama testi.
// Her test D3D9 API'siyle çizer, piksel geri okuyarak sonucu kontrol eder.
#include <d3dx9.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

static int g_failures = 0;
#define CHECK(cond, ...)                                              \
	do                                                                \
	{                                                                 \
		if (!(cond))                                                  \
		{                                                             \
			std::printf("  BAŞARISIZ %s:%d: ", __FILE__, __LINE__);   \
			std::printf(__VA_ARGS__);                                 \
			std::printf("\n");                                        \
			++g_failures;                                             \
		}                                                             \
	} while (0)

static const int W = 128, H = 96;

static void ReadPixel(int x, int y, unsigned char out[4])
{
	// D3D koordinatı (y yukarıdan) → GL (y aşağıdan)
	glReadPixels(x, H - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, out);
}

static bool Near(int a, int b, int tol = 8)
{
	return std::abs(a - b) <= tol;
}

struct VtxColor
{
	float x, y, z;
	D3DCOLOR color;
};
struct VtxRhwTex
{
	float x, y, z, rhw;
	D3DCOLOR color;
	float u, v;
};
struct VtxNormal
{
	float x, y, z;
	float nx, ny, nz;
};

static void SetOrthoCamera(IDirect3DDevice9* dev)
{
	D3DMATRIX id {};
	id._11 = id._22 = id._33 = id._44 = 1.0f;
	dev->SetTransform(D3DTS_WORLD, &id);
	dev->SetTransform(D3DTS_VIEW, &id);
	// Basit ortografik: x,y ∈ [-1,1] → NDC; z ∈ [0,1] → D3D z [0,1]
	D3DMATRIX p {};
	p._11 = 1; p._22 = 1; p._33 = 1; p._44 = 1;
	dev->SetTransform(D3DTS_PROJECTION, &p);
}

static void TestClear(IDirect3DDevice9* dev)
{
	std::printf("[test] Clear\n");
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[0] == 0 && px[1] == 0 && px[2] == 255, "mavi bekleniyordu, %d %d %d", px[0], px[1], px[2]);
}

static void TestColoredTriangle(IDirect3DDevice9* dev)
{
	std::printf("[test] DrawPrimitiveUP XYZ|DIFFUSE (aydınlatma kapalı), saat yönü sarma\n");
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	SetOrthoCamera(dev);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	dev->SetTexture(0, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	// Ekranda saat yönü (D3D ön yüz): üst, sağ-alt, sol-alt
	VtxColor tri[3] = {{0.0f, 0.9f, 0.5f, D3DCOLOR_XRGB(255, 0, 0)}, {0.9f, -0.9f, 0.5f, D3DCOLOR_XRGB(255, 0, 0)}, {-0.9f, -0.9f, 0.5f, D3DCOLOR_XRGB(255, 0, 0)}};
	dev->BeginScene();
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, tri, sizeof(VtxColor));
	dev->EndScene();
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[0] == 255 && px[1] == 0 && px[2] == 0, "kırmızı bekleniyordu, %d %d %d", px[0], px[1], px[2]);

	// Ters sarma (saat yönünün tersi) → D3DCULL_CCW ile atılmalı
	std::printf("[test] D3DCULL_CCW ters sarmayı atıyor\n");
	dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	VtxColor rev[3] = {tri[0], tri[2], tri[1]};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, rev, sizeof(VtxColor));
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[0] == 0, "ters sarma atılmalıydı, %d", px[0]);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, rev, sizeof(VtxColor));
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[0] == 255, "CULL_NONE ile çizilmeliydi, %d", px[0]);
}

static void TestRhwTexturedQuad(IDirect3DDevice9* dev, D3DFORMAT fmt, const char* name)
{
	std::printf("[test] XYZRHW dokulu dörtgen (%s)\n", name);
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	IDirect3DTexture9* tex = nullptr;
	HRESULT hr             = dev->CreateTexture(2, 2, 1, 0, fmt, D3DPOOL_MANAGED, &tex, nullptr);
	CHECK(SUCCEEDED(hr) && tex, "CreateTexture başarısız");
	if (!tex)
		return;
	D3DLOCKED_RECT lr;
	tex->LockRect(0, &lr, nullptr, 0);
	// Sol üst kırmızı, sağ üst yeşil, sol alt mavi, sağ alt beyaz
	if (fmt == D3DFMT_A8R8G8B8 || fmt == D3DFMT_X8R8G8B8)
	{
		uint32_t* p = (uint32_t*) lr.pBits;
		p[0] = 0xFFFF0000; p[1] = 0xFF00FF00;
		p    = (uint32_t*) ((uint8_t*) lr.pBits + lr.Pitch);
		p[0] = 0xFF0000FF; p[1] = 0xFFFFFFFF;
	}
	else if (fmt == D3DFMT_A4R4G4B4)
	{
		uint16_t* p = (uint16_t*) lr.pBits;
		p[0] = 0xFF00; p[1] = 0xF0F0;
		p    = (uint16_t*) ((uint8_t*) lr.pBits + lr.Pitch);
		p[0] = 0xF00F; p[1] = 0xFFFF;
	}
	else if (fmt == D3DFMT_A1R5G5B5)
	{
		uint16_t* p = (uint16_t*) lr.pBits;
		p[0] = 0xFC00; p[1] = 0x83E0;
		p    = (uint16_t*) ((uint8_t*) lr.pBits + lr.Pitch);
		p[0] = 0x801F; p[1] = 0xFFFF;
	}
	else if (fmt == D3DFMT_R5G6B5)
	{
		uint16_t* p = (uint16_t*) lr.pBits;
		p[0] = 0xF800; p[1] = 0x07E0;
		p    = (uint16_t*) ((uint8_t*) lr.pBits + lr.Pitch);
		p[0] = 0x001F; p[1] = 0xFFFF;
	}
	tex->UnlockRect(0);

	dev->SetTexture(0, tex);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	dev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	dev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	dev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	float x0 = 0, y0 = 0, x1 = (float) W, y1 = (float) H;
	D3DCOLOR white = 0xFFFFFFFF;
	VtxRhwTex quad[4] = {
		{x0 - 0.5f, y0 - 0.5f, 0.5f, 1.0f, white, 0, 0},
		{x1 - 0.5f, y0 - 0.5f, 0.5f, 1.0f, white, 1, 0},
		{x0 - 0.5f, y1 - 0.5f, 0.5f, 1.0f, white, 0, 1},
		{x1 - 0.5f, y1 - 0.5f, 0.5f, 1.0f, white, 1, 1},
	};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(VtxRhwTex));
	unsigned char px[4];
	ReadPixel(W / 4, H / 4, px);
	CHECK(Near(px[0], 255) && Near(px[1], 0) && Near(px[2], 0), "sol üst kırmızı olmalı: %d %d %d", px[0], px[1], px[2]);
	ReadPixel(3 * W / 4, H / 4, px);
	CHECK(Near(px[0], 0) && Near(px[1], 255) && Near(px[2], 0), "sağ üst yeşil olmalı: %d %d %d", px[0], px[1], px[2]);
	ReadPixel(W / 4, 3 * H / 4, px);
	CHECK(Near(px[0], 0) && Near(px[1], 0) && Near(px[2], 255), "sol alt mavi olmalı: %d %d %d", px[0], px[1], px[2]);
	ReadPixel(3 * W / 4, 3 * H / 4, px);
	CHECK(Near(px[0], 255) && Near(px[1], 255) && Near(px[2], 255), "sağ alt beyaz olmalı: %d %d %d", px[0], px[1], px[2]);
	dev->SetTexture(0, nullptr);
	tex->Release();
}

static void TestDXT1(IDirect3DDevice9* dev)
{
	std::printf("[test] DXT1 doku (S3TC=%d)\n", (int) dev->SupportsS3TC());
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	IDirect3DTexture9* tex = nullptr;
	dev->CreateTexture(4, 4, 1, 0, D3DFMT_DXT1, D3DPOOL_MANAGED, &tex, nullptr);
	CHECK(tex != nullptr, "DXT1 CreateTexture");
	if (!tex)
		return;
	D3DSURFACE_DESC sd;
	tex->GetLevelDesc(0, &sd);
	CHECK(sd.Format == D3DFMT_DXT1 && sd.Width == 4, "DXT1 level desc");
	D3DLOCKED_RECT lr;
	tex->LockRect(0, &lr, nullptr, 0);
	// Tek blok: color0 = saf yeşil (0x07E0), color1 = saf yeşil, tüm indeksler 0
	uint8_t* b = (uint8_t*) lr.pBits;
	b[0] = 0xE0; b[1] = 0x07; b[2] = 0xE0; b[3] = 0x07;
	b[4] = b[5] = b[6] = b[7] = 0;
	tex->UnlockRect(0);
	dev->SetTexture(0, tex);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	D3DCOLOR white    = 0xFFFFFFFF;
	VtxRhwTex quad[4] = {{-0.5f, -0.5f, 0.5f, 1, white, 0, 0}, {W - 0.5f, -0.5f, 0.5f, 1, white, 1, 0}, {-0.5f, H - 0.5f, 0.5f, 1, white, 0, 1}, {W - 0.5f, H - 0.5f, 0.5f, 1, white, 1, 1}};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(VtxRhwTex));
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	CHECK(Near(px[0], 0) && Near(px[1], 255) && Near(px[2], 0), "DXT1 yeşil olmalı: %d %d %d", px[0], px[1], px[2]);
	dev->SetTexture(0, nullptr);
	tex->Release();
}

static void TestLighting(IDirect3DDevice9* dev)
{
	std::printf("[test] Yönlü ışık + malzeme (köşe aydınlatması)\n");
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	SetOrthoCamera(dev);
	dev->SetRenderState(D3DRS_LIGHTING, TRUE);
	dev->SetRenderState(D3DRS_AMBIENT, 0);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetTexture(0, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	D3DMATERIAL9 mat {};
	mat.Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
	dev->SetMaterial(&mat);
	D3DLIGHT9 light {};
	light.Type      = D3DLIGHT_DIRECTIONAL;
	light.Diffuse   = {0.0f, 1.0f, 0.0f, 1.0f};
	light.Direction = {0.0f, 0.0f, 1.0f}; // kameradan sahneye (LH: +z ileri)
	dev->SetLight(0, &light);
	dev->LightEnable(0, TRUE);
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
	VtxNormal tri[3] = {{0.0f, 0.9f, 0.5f, 0, 0, -1}, {0.9f, -0.9f, 0.5f, 0, 0, -1}, {-0.9f, -0.9f, 0.5f, 0, 0, -1}};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, tri, sizeof(VtxNormal));
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	CHECK(Near(px[0], 0) && Near(px[1], 255) && Near(px[2], 0), "tam aydınlatılmış yeşil bekleniyordu: %d %d %d", px[0], px[1], px[2]);

	// Yarı açı: normal ışığa 60° → cos = 0.5 → 128 civarı
	for (auto& v : tri) { v.nx = 0.0f; v.ny = std::sqrt(3.0f) / 2.0f; v.nz = -0.5f; }
	dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, tri, sizeof(VtxNormal));
	ReadPixel(W / 2, H / 2, px);
	CHECK(Near(px[1], 128, 10), "60° için ~128 yeşil bekleniyordu: %d", px[1]);
	dev->LightEnable(0, FALSE);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
}

static void TestAlphaBlendAndTest(IDirect3DDevice9* dev)
{
	std::printf("[test] Alfa karıştırma ve alfa testi\n");
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	SetOrthoCamera(dev);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	dev->SetTexture(0, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	D3DCOLOR half = D3DCOLOR_ARGB(128, 255, 255, 255);
	VtxColor tri[3] = {{0.0f, 0.9f, 0.5f, half}, {0.9f, -0.9f, 0.5f, half}, {-0.9f, -0.9f, 0.5f, half}};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, tri, sizeof(VtxColor));
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	CHECK(Near(px[0], 128, 4), "yarı saydam beyaz → ~128: %d", px[0]);

	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	dev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	dev->SetRenderState(D3DRS_ALPHAREF, 200);
	dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, tri, sizeof(VtxColor));
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[0] == 0, "alfa testi (128 > 200 değil) pikseli atmalıydı: %d", px[0]);
	dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
}

static void TestVertexBufferDraw(IDirect3DDevice9* dev)
{
	std::printf("[test] CreateVertexBuffer + SetStreamSource + DrawPrimitive, indeksli çizim\n");
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	SetOrthoCamera(dev);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	IDirect3DVertexBuffer9* vb = nullptr;
	dev->CreateVertexBuffer(4 * sizeof(VtxColor), 0, D3DFVF_XYZ | D3DFVF_DIFFUSE, D3DPOOL_MANAGED, &vb, nullptr);
	void* p = nullptr;
	vb->Lock(0, 0, &p, 0);
	VtxColor* v = (VtxColor*) p;
	v[0] = {-0.9f, 0.9f, 0.5f, D3DCOLOR_XRGB(0, 255, 255)};
	v[1] = {0.9f, 0.9f, 0.5f, D3DCOLOR_XRGB(0, 255, 255)};
	v[2] = {-0.9f, -0.9f, 0.5f, D3DCOLOR_XRGB(0, 255, 255)};
	v[3] = {0.9f, -0.9f, 0.5f, D3DCOLOR_XRGB(0, 255, 255)};
	vb->Unlock();
	IDirect3DIndexBuffer9* ib = nullptr;
	dev->CreateIndexBuffer(6 * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, nullptr);
	ib->Lock(0, 0, &p, 0);
	uint16_t idx[6] = {0, 1, 2, 2, 1, 3};
	std::memcpy(p, idx, sizeof(idx));
	ib->Unlock();
	dev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	dev->SetStreamSource(0, vb, 0, sizeof(VtxColor));
	dev->SetIndices(ib);
	dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[0] == 0 && px[1] == 255 && px[2] == 255, "cam göbeği bekleniyordu: %d %d %d", px[0], px[1], px[2]);
	dev->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	dev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	ReadPixel(W / 2, H / 2, px);
	CHECK(px[1] == 255, "strip çizimi: %d", px[1]);
	dev->SetStreamSource(0, nullptr, 0, 0);
	dev->SetIndices(nullptr);
	vb->Release();
	ib->Release();
}

static void TestMipLoadSurface(IDirect3DDevice9* dev)
{
	std::printf("[test] D3DXLoadSurfaceFromSurface ile mip üretimi\n");
	IDirect3DTexture9* tex = nullptr;
	dev->CreateTexture(4, 4, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, nullptr);
	CHECK(tex && tex->GetLevelCount() == 3, "4x4 için 3 mip seviyesi bekleniyordu: %u", tex ? (unsigned) tex->GetLevelCount() : 0u);
	D3DLOCKED_RECT lr;
	tex->LockRect(0, &lr, nullptr, 0);
	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			((uint32_t*) ((uint8_t*) lr.pBits + y * lr.Pitch))[x] = ((x + y) & 1) ? 0xFFFFFFFF : 0xFF000000;
	tex->UnlockRect(0);
	IDirect3DSurface9 *s0 = nullptr, *s1 = nullptr;
	tex->GetSurfaceLevel(0, &s0);
	tex->GetSurfaceLevel(1, &s1);
	HRESULT hr = D3DXLoadSurfaceFromSurface(s1, nullptr, nullptr, s0, nullptr, nullptr, D3DX_FILTER_TRIANGLE, 0);
	CHECK(SUCCEEDED(hr), "LoadSurfaceFromSurface");
	s1->LockRect(&lr, nullptr, D3DLOCK_READONLY);
	uint32_t c = ((uint32_t*) lr.pBits)[0];
	s1->UnlockRect();
	CHECK(Near((c >> 16) & 0xFF, 127, 2), "damalı desenin ortalaması gri olmalı: 0x%08X", c);
	s0->Release();
	s1->Release();
	tex->Release();
}


// Arazi (CN3TerrainPatch) 3 aşamalı karışım: aşama0 SELECTARG1(TEXTURE), aşama1 ADD(TEXTURE, CURRENT),
// aşama2 MODULATE(CURRENT, DIFFUSE) ve aşama 2'de doku yok. İki doku koordinat seti (FVF TEX2).
struct VtxRhwTex2
{
	float x, y, z, rhw;
	D3DCOLOR color;
	float u0, v0, u1, v1;
};

static IDirect3DTexture9* SolidTexture(IDirect3DDevice9* dev, D3DCOLOR c)
{
	IDirect3DTexture9* tex = nullptr;
	dev->CreateTexture(2, 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, nullptr);
	if (!tex)
		return nullptr;
	D3DLOCKED_RECT lr;
	tex->LockRect(0, &lr, nullptr, 0);
	for (int y = 0; y < 2; ++y)
	{
		uint32_t* p = (uint32_t*) ((uint8_t*) lr.pBits + y * lr.Pitch);
		p[0] = p[1] = c;
	}
	tex->UnlockRect(0);
	return tex;
}

static void TestTerrainStages(IDirect3DDevice9* dev)
{
	std::printf("[test] Arazi 3 aşamalı doku karışımı (SELECTARG1 / ADD / MODULATE DIFFUSE, aşama 2 dokusuz)\n");
	D3DCAPS9 caps;
	dev->GetDeviceCaps(&caps);
	CHECK(caps.MaxTextureBlendStages >= 3, "MaxTextureBlendStages=%u (>=3 olmalı)", caps.MaxTextureBlendStages);
	CHECK((caps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) != 0, "D3DPMISCCAPS_BLENDOP yok (arazi iki geçişe düşer)");

	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	IDirect3DTexture9* t0 = SolidTexture(dev, 0xFF400000); // R 64
	IDirect3DTexture9* t1 = SolidTexture(dev, 0xFF004000); // G 64
	dev->SetTexture(0, t0);
	dev->SetTexture(1, t1);
	dev->SetTexture(2, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
	dev->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	dev->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	dev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
	dev->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	dev->SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	dev->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dev->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2);
	D3DCOLOR half = D3DCOLOR_XRGB(128, 128, 128); // diffuse = 0.5
	VtxRhwTex2 quad[4] = {
		{0, 0, 0.5f, 1, half, 0, 0, 0, 0}, {(float) W, 0, 0.5f, 1, half, 1, 0, 1, 0},
		{0, (float) H, 0.5f, 1, half, 0, 1, 0, 1}, {(float) W, (float) H, 0.5f, 1, half, 1, 1, 1, 1}};
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(VtxRhwTex2));
	unsigned char px[4];
	ReadPixel(W / 2, H / 2, px);
	// (64 + 0) * 0.5 = 32 kırmızı, (0 + 64) * 0.5 = 32 yeşil
	CHECK(Near(px[0], 32) && Near(px[1], 32) && Near(px[2], 0), "beklenen ~(32,32,0), okunan %d %d %d", px[0], px[1], px[2]);

	// Aynı durum, aşama 2 ile aynı sonucu vermeli; aşama 1 kapalıyken yalnız doku0 * diffuse
	dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(VtxRhwTex2));
	ReadPixel(W / 2, H / 2, px);
	CHECK(Near(px[0], 32) && Near(px[1], 0), "tek aşama: beklenen ~(32,0,0), okunan %d %d %d", px[0], px[1], px[2]);

	dev->SetTexture(0, nullptr);
	dev->SetTexture(1, nullptr);
	dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	dev->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	if (t0) t0->Release();
	if (t1) t1->Release();
}

int main()
{
	// EGL başsız bağlam
	PFNEGLGETPLATFORMDISPLAYEXTPROC getPlat = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
	EGLDisplay dpy = getPlat ? getPlat(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr) : eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (dpy == EGL_NO_DISPLAY || !eglInitialize(dpy, nullptr, nullptr))
	{
		std::printf("EGL yok; test atlandı\n");
		return 0;
	}
	EGLint attr[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8, EGL_NONE};
	EGLConfig cfg;
	EGLint n = 0;
	eglChooseConfig(dpy, attr, &cfg, 1, &n);
	eglBindAPI(EGL_OPENGL_ES_API);
	EGLint ca[]    = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
	EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ca);
	EGLint pa[]    = {EGL_WIDTH, W, EGL_HEIGHT, H, EGL_NONE};
	EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pa);
	eglMakeCurrent(dpy, surf, surf, ctx);

	d3d9gles::PlatformHooks hooks;
	hooks.getDrawableSize = [](void*, int* w, int* h) { *w = W; *h = H; };
	hooks.present         = [](void*) { glFinish(); };
	d3d9gles::SetPlatformHooks(hooks);

	IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS pp {};
	pp.Windowed               = TRUE;
	pp.BackBufferWidth        = W;
	pp.BackBufferHeight       = H;
	pp.BackBufferFormat       = D3DFMT_X8R8G8B8;
	pp.EnableAutoDepthStencil = TRUE;
	pp.AutoDepthStencilFormat = D3DFMT_D24S8;
	pp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	IDirect3DDevice9* dev     = nullptr;
	HRESULT hr                = d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev);
	CHECK(SUCCEEDED(hr) && dev, "CreateDevice");
	if (!dev)
		return 1;

	D3DCAPS9 caps;
	dev->GetDeviceCaps(&caps);
	std::printf("MaxTexture=%u, MaxLights=%u\n", caps.MaxTextureWidth, caps.MaxActiveLights);

	TestClear(dev);
	TestColoredTriangle(dev);
	TestRhwTexturedQuad(dev, D3DFMT_A8R8G8B8, "A8R8G8B8");
	TestRhwTexturedQuad(dev, D3DFMT_A4R4G4B4, "A4R4G4B4");
	TestRhwTexturedQuad(dev, D3DFMT_A1R5G5B5, "A1R5G5B5");
	TestRhwTexturedQuad(dev, D3DFMT_R5G6B5, "R5G6B5");
	TestDXT1(dev);
	TestLighting(dev);
	TestAlphaBlendAndTest(dev);
	TestVertexBufferDraw(dev);
	TestMipLoadSurface(dev);
	TestTerrainStages(dev);
	dev->Present(nullptr, nullptr, nullptr, nullptr);

	GLenum err = glGetError();
	CHECK(err == GL_NO_ERROR, "GL hatası 0x%X", err);

	dev->Release();
	d3d->Release();
	std::printf(g_failures ? "SONUÇ: %d hata\n" : "SONUÇ: tüm testler geçti\n", g_failures);
	return g_failures ? 1 : 0;
}
