#include "KoText.h"
// DFont_ft.cpp — CDFont'un FreeType ile taşınabilir uygulaması (Windows GDI yerine).
//
// Davranış orijinal DFont.cpp ile aynı tutuldu: metin, satır satır bir A4R4G4B4 dokuya
// rasterize edilir; her yazı parçası için 2 üçgenlik (6 köşe) XYZRHW dörtgen üretilir;
// DrawText yalnızca köşeleri kaydırır/renklendirir ve tek DrawPrimitive ile çizer.
#if !defined(_WIN32)

#include <N3Base/StdAfxBase.h>
#include <N3Base/DFont.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#if __has_include(<iconv.h>) && !defined(__ANDROID__)
#include <iconv.h>
#define KO_HAVE_ICONV 1
#endif

const int MAX_NUM_VERTICES = 50 * 6;
const float Z_DEFAULT      = 0.9f;
const float RHW_DEFAULT    = 1.0f;

int CDFont::s_iInstanceCount = 0;

namespace
{
FT_Library g_ftLib = nullptr;

struct FaceKey
{
	std::string path;
	int pixelHeight;
	bool operator<(const FaceKey& o) const
	{
		return path < o.path || (path == o.path && pixelHeight < o.pixelHeight);
	}
};
std::map<FaceKey, FT_Face> g_faces;

/// Yazı tipi dosyasını bul: KO_FONT_PATH, <oyun yolu>/fonts/<ad>.ttf, fonts/default.ttf,
/// ardından sistem yazı tipleri (Android / Linux).
std::string FindFontFile(const std::string& fontName)
{
	std::vector<std::string> candidates;
	if (const char* env = std::getenv("KO_FONT_PATH"))
		candidates.push_back(env);
	std::string base = CN3Base::PathGet();
	if (!base.empty() && base.back() != '/' && base.back() != '\\')
		base += '/';
	candidates.push_back(base + "fonts/" + fontName + ".ttf");
	candidates.push_back(base + "fonts/" + fontName + ".otf");
	candidates.push_back(base + "fonts/default.ttf");
	candidates.push_back(base + "Fonts/default.ttf");
#if defined(__ANDROID__)
	candidates.push_back("/system/fonts/NotoSansCJK-Regular.ttc");
	candidates.push_back("/system/fonts/NotoSansKR-Regular.otf");
	candidates.push_back("/system/fonts/Roboto-Regular.ttf");
	candidates.push_back("/system/fonts/DroidSans.ttf");
#else
	candidates.push_back("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc");
	candidates.push_back("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc");
	candidates.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
	candidates.push_back("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
	candidates.push_back("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
#endif
	for (const auto& c : candidates)
		if (std::filesystem::exists(c))
			return c;
	return {};
}

FT_Face AcquireFace(const std::string& fontName, int pixelHeight)
{
	if (!g_ftLib && FT_Init_FreeType(&g_ftLib) != 0)
		return nullptr;
	std::string path = FindFontFile(fontName);
	if (path.empty())
	{
		static bool warned = false;
		if (!warned)
		{
			std::fprintf(stderr, "DFont: yazı tipi dosyası bulunamadı (%s). KO_FONT_PATH ayarlayın veya <oyun>/fonts/default.ttf koyun.\n", fontName.c_str());
			warned = true;
		}
		return nullptr;
	}
	FaceKey key {path, pixelHeight};
	auto it = g_faces.find(key);
	if (it != g_faces.end())
		return it->second;
	FT_Face face = nullptr;
	if (FT_New_Face(g_ftLib, path.c_str(), 0, &face) != 0)
		return nullptr;
	FT_Set_Pixel_Sizes(face, 0, (FT_UInt) std::max(1, pixelHeight));
	g_faces[key] = face;
	return face;
}

/// Yedek yazı tipi: ana yazı tipinde (ör. NotoSansCJK) bulunmayan Latin Genişletilmiş glifler (ğ ş İ ı Ğ Ş) için.
FT_Face AcquireFallbackFace(const std::string& primaryPath, int pixelHeight)
{
	static const char* const kFallbacks[] = {
		"/system/fonts/Roboto-Regular.ttf", "/system/fonts/NotoSans-Regular.ttf", "/system/fonts/DroidSans.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
		"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"};
	if (const char* env = std::getenv("KO_FONT_FALLBACK"))
	{
		FaceKey key {env, pixelHeight};
		auto it = g_faces.find(key);
		if (it != g_faces.end())
			return it->second;
		FT_Face face = nullptr;
		if (FT_New_Face(g_ftLib, env, 0, &face) == 0)
		{
			FT_Set_Pixel_Sizes(face, 0, (FT_UInt) std::max(1, pixelHeight));
			g_faces[key] = face;
			return face;
		}
	}
	for (const char* path : kFallbacks)
	{
		if (primaryPath == path)
			continue;
		FaceKey key {path, pixelHeight};
		auto it = g_faces.find(key);
		if (it != g_faces.end())
			return it->second;
		FT_Face face = nullptr;
		if (FT_New_Face(g_ftLib, path, 0, &face) != 0)
			continue;
		// Yalnız Türkçe glifleri olan bir yedek işe yarar
		if (FT_Get_Char_Index(face, 0x011F) == 0 || FT_Get_Char_Index(face, 0x015F) == 0)
		{
			FT_Done_Face(face);
			continue;
		}
		FT_Set_Pixel_Sizes(face, 0, (FT_UInt) std::max(1, pixelHeight));
		g_faces[key] = face;
		return face;
	}
	return nullptr;
}

/// Oyun metni: ASCII + CP949 (Korece) çift bayt. Unicode kod noktalarına çevir.
std::vector<uint32_t> DecodeText(const std::string& s)
{
	std::vector<uint32_t> out;
	out.reserve(s.size());
	for (size_t i = 0; i < s.size();)
	{
		unsigned char c = (unsigned char) s[i];
		if ((c & 0x80) && KoTextCodePage() == 1254) // Türkçe tek bayt
		{
			out.push_back(KoText1254ToUnicode(c));
			++i;
		}
		else if (c & 0x80)
		{
			if (i + 1 >= s.size())
				break;
			uint32_t cp = 0xFFFD;
#if KO_HAVE_ICONV
			static iconv_t cd = iconv_open("UTF-32LE", "CP949");
			if (cd != (iconv_t) -1)
			{
				char in[2]   = {s[i], s[i + 1]};
				char outb[8] = {};
				char* pin    = in;
				char* pout   = outb;
				size_t inl   = 2, outl = sizeof(outb);
				iconv(cd, nullptr, nullptr, nullptr, nullptr);
				if (iconv(cd, &pin, &inl, &pout, &outl) != (size_t) -1)
					cp = (uint32_t) (unsigned char) outb[0] | ((uint32_t) (unsigned char) outb[1] << 8) | ((uint32_t) (unsigned char) outb[2] << 16);
			}
#endif
			out.push_back(cp);
			i += 2;
		}
		else
		{
			out.push_back(c);
			++i;
		}
	}
	return out;
}
} // namespace

// Yazı tipi yüksekliği: GDI'de -MulDiv(pt, 96, 72) piksel.
static int PixelHeightFor(uint32_t dwPointHeight, float scale)
{
	return std::max(1, MulDiv((int) dwPointHeight, (int) (96 * scale), 72));
}

CDFont::CDFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags)
{
	s_iInstanceCount++;
	__ASSERT(!szFontName.empty(), "??");

	m_szFontName      = szFontName;
	m_dwFontHeight    = dwHeight;
	m_dwFontFlags     = dwFlags;
	m_dwTexHeight     = 0;
	m_dwTexWidth      = 0;
	m_fTextScale      = 1.0f;
	m_pd3dDevice      = nullptr;
	m_pTexture        = nullptr;
	m_pVB             = nullptr;
	m_iPrimitiveCount = 0;
	m_PrevLeftTop.x = m_PrevLeftTop.y = 0;
	m_ftFace          = nullptr;
	m_iLineHeight     = 0;
	m_iAscender       = 0;
	m_dwFontColor     = 0xffffffff;
	m_Size.cx         = 0;
	m_Size.cy         = 0;
}

CDFont::~CDFont()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
	s_iInstanceCount--;
}

bool CDFont::LoadFace()
{
	FT_Face face = AcquireFace(m_szFontName, PixelHeightFor(m_dwFontHeight, m_fTextScale));
	m_ftFace     = face;
	if (!face)
		return false;
	// Ana yazı tipinde ğ/ş yoksa (NotoSansCJK) yedek yüz
	m_ftFaceFallback = nullptr;
	if (FT_Get_Char_Index(face, 0x011F) == 0 || FT_Get_Char_Index(face, 0x015F) == 0 || FT_Get_Char_Index(face, 0x0130) == 0)
	{
		m_ftFaceFallback = AcquireFallbackFace(FindFontFile(m_szFontName), PixelHeightFor(m_dwFontHeight, m_fTextScale));
		static bool s_bLogged = false;
		if (!s_bLogged)
		{
			s_bLogged = true;
			std::fprintf(stderr, "DFont: ana yazı tipinde Türkçe glifler yok; yedek %s\n", m_ftFaceFallback ? "bulundu" : "BULUNAMADI (KO_FONT_FALLBACK)");
		}
	}
	m_iLineHeight = (int) ((face->size->metrics.height + 63) >> 6);
	m_iAscender   = (int) ((face->size->metrics.ascender + 63) >> 6);
	if (m_iLineHeight <= 0)
		m_iLineHeight = PixelHeightFor(m_dwFontHeight, m_fTextScale);
	return true;
}

HRESULT CDFont::SetFont(const std::string& szFontName, uint32_t dwHeight, uint32_t dwFlags)
{
	__ASSERT(!szFontName.empty(), "");
	m_szFontName   = szFontName;
	m_dwFontHeight = dwHeight;
	m_dwFontFlags  = dwFlags;
	return LoadFace() ? S_OK : E_FAIL;
}

HRESULT CDFont::InitDeviceObjects(LPDIRECT3DDEVICE9 pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;
	m_fTextScale = 1.0f;
	return S_OK;
}

HRESULT CDFont::RestoreDeviceObjects()
{
	m_iPrimitiveCount = 0;
	if (!LoadFace())
		return E_FAIL;

	__ASSERT(m_pVB == nullptr, "??");
	int iVBSize    = MAX_NUM_VERTICES * sizeof(__VertexTransformed);
	uint32_t dwFVF = FVF_TRANSFORMED;
	HRESULT hr     = m_pd3dDevice->CreateVertexBuffer(iVBSize, 0, dwFVF, D3DPOOL_MANAGED, &m_pVB, nullptr);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

HRESULT CDFont::InvalidateDeviceObjects()
{
	if (m_pVB)
	{
		m_pVB->Release();
		m_pVB = nullptr;
	}
	m_ftFace         = nullptr; // yüzler önbellekte paylaşılır; burada serbest bırakılmaz
	m_ftFaceFallback = nullptr; // ana yüzle birlikte yeniden seçilir (LoadFace)
	m_iPrimitiveCount = 0;      // tampon gitti; yeniden SetText gelene kadar çizilecek bir şey yok
	return S_OK;
}

HRESULT CDFont::DeleteDeviceObjects()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}
	m_pd3dDevice = nullptr;
	return S_OK;
}

// Glif seçimi: ana yüz, yoksa yedek yüz, o da yoksa '?'
static FT_Face PickGlyph(FT_Face primary, FT_Face fallback, uint32_t cp, FT_UInt& gi)
{
	gi = 0;
	if (primary == nullptr)
		return fallback; // çağıran null denetimi yapar
	gi = FT_Get_Char_Index(primary, cp);
	if (gi != 0)
		return primary;
	if (fallback)
	{
		gi = FT_Get_Char_Index(fallback, cp);
		if (gi != 0)
			return fallback;
	}
	if (cp != '?')
		gi = FT_Get_Char_Index(primary, '?');
	return primary;
}

int CDFont::MeasureWidth(const uint32_t* cps, size_t n) const
{
	FT_Face face = (FT_Face) m_ftFace;
	if (!face)
		return 0;
	int w = 0;
	for (size_t i = 0; i < n; ++i)
	{
		FT_UInt gi = 0;
		FT_Face f  = PickGlyph(face, (FT_Face) m_ftFaceFallback, cps[i], gi);
		if (f == nullptr || f->glyph == nullptr || FT_Load_Glyph(f, gi, FT_LOAD_DEFAULT) != 0)
			continue;
		if (m_dwFontFlags & D3DFONT_BOLD)
			FT_GlyphSlot_Embolden(f->glyph);
		w += (int) ((f->glyph->advance.x + 32) >> 6);
	}
	return w;
}

void CDFont::DrawGlyphs(const uint32_t* cps, size_t n, int x, int y)
{
	FT_Face face = (FT_Face) m_ftFace;
	if (!face || m_coverage.empty())
		return;
	int penX = x;
	for (size_t i = 0; i < n; ++i)
	{
		FT_UInt gi = 0;
		FT_Face f  = PickGlyph(face, (FT_Face) m_ftFaceFallback, cps[i], gi);
		if (f == nullptr || f->glyph == nullptr || FT_Load_Glyph(f, gi, FT_LOAD_DEFAULT) != 0)
			continue;
		if (m_dwFontFlags & D3DFONT_BOLD)
			FT_GlyphSlot_Embolden(f->glyph);
		if (m_dwFontFlags & D3DFONT_ITALIC)
			FT_GlyphSlot_Oblique(f->glyph);
		if (FT_Render_Glyph(f->glyph, FT_RENDER_MODE_NORMAL) != 0)
			continue;
		const FT_Bitmap& bm = f->glyph->bitmap;
		if (bm.buffer == nullptr || bm.pixel_mode != FT_PIXEL_MODE_GRAY)
		{
			penX += (int) ((f->glyph->advance.x + 32) >> 6);
			continue;
		}
		int gx              = penX + f->glyph->bitmap_left;
		int gy              = y + m_iAscender - f->glyph->bitmap_top;
		for (unsigned r = 0; r < bm.rows; ++r)
		{
			int ty = gy + (int) r;
			if (ty < 0 || ty >= (int) m_dwTexHeight)
				continue;
			for (unsigned c = 0; c < bm.width; ++c)
			{
				int tx = gx + (int) c;
				if (tx < 0 || tx >= (int) m_dwTexWidth)
					continue;
				uint8_t v  = bm.buffer[r * bm.pitch + c];
				uint8_t& d = m_coverage[(size_t) ty * m_dwTexWidth + tx];
				d          = std::max(d, v);
			}
		}
		penX += (int) ((f->glyph->advance.x + 32) >> 6);
	}
}

HRESULT CDFont::SetText(const std::string& szText, uint32_t dwFlags)
{
	if (!m_ftFace && !LoadFace())
		return E_FAIL;

	if (szText.empty())
	{
		m_iPrimitiveCount = 0;
		if (m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
		return S_OK;
	}

	// '\n' hariç tek satır gibi ölçüp doku boyutunu seç (orijinal algoritma)
	std::string szTemp;
	szTemp.reserve(szText.size());
	for (char ch : szText)
		if (ch != '\n')
			szTemp.push_back(ch);
	std::vector<uint32_t> all = DecodeText(szTemp);
	SIZE size;
	size.cx = MeasureWidth(all.data(), all.size());
	size.cy = m_iLineHeight;
	if (size.cx <= 0 || size.cy <= 0)
	{
		__ASSERT(0, "Invalid Text Size - ?????");
		return E_FAIL;
	}
	int iExtent = size.cx * size.cy;

	// Yarım Korece karakter genişliği (orijinal: "진"in yarısı) ≈ satır yüksekliğinin yarısı
	int halfW        = (m_iLineHeight + 1) / 2;
	int iTexSizes[7] = {32, 64, 128, 256, 512, 1024, 2048};
	m_dwTexWidth = m_dwTexHeight = 2048;
	for (int i = 0; i < 7; ++i)
	{
		if (iExtent <= (iTexSizes[i] - halfW - size.cy - 1) * iTexSizes[i])
		{
			m_dwTexWidth = m_dwTexHeight = iTexSizes[i];
			break;
		}
	}

	m_fTextScale = 1.0f;
	D3DCAPS9 d3dCaps;
	m_pd3dDevice->GetDeviceCaps(&d3dCaps);
	if (m_dwTexWidth > d3dCaps.MaxTextureWidth)
	{
		m_fTextScale = (float) d3dCaps.MaxTextureWidth / (float) m_dwTexWidth;
		m_dwTexWidth = m_dwTexHeight = d3dCaps.MaxTextureWidth;
	}

	if (m_pTexture)
	{
		D3DSURFACE_DESC sd {};
		m_pTexture->GetLevelDesc(0, &sd);
		if (sd.Width != m_dwTexWidth)
		{
			m_pTexture->Release();
			m_pTexture = nullptr;
		}
	}
	if (nullptr == m_pTexture)
	{
		int iMipMapCount = (dwFlags & D3DFONT_FILTERED) ? 0 : 1;
		HRESULT hr = m_pd3dDevice->CreateTexture(m_dwTexWidth, m_dwTexHeight, iMipMapCount, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &m_pTexture, nullptr);
		if (FAILED(hr) || m_pTexture == nullptr)
		{
			m_pTexture        = nullptr;
			m_iPrimitiveCount = 0; // doku yok: eski üçgenler çizilmesin
			return FAILED(hr) ? hr : E_FAIL;
		}
	}

	// Kapsama tamponu (GDI DIB'inin karşılığı)
	m_coverage.assign((size_t) m_dwTexWidth * m_dwTexHeight, 0);
	Make2DVertex(size.cy, szText);

	// Alfa dokusuna çevir: 4 bitlik yoğunluk
	D3DLOCKED_RECT d3dlr;
	if (SUCCEEDED(m_pTexture->LockRect(0, &d3dlr, nullptr, 0)))
	{
		for (uint32_t y = 0; y < m_dwTexHeight; y++)
		{
			uint16_t* pDst16 = (uint16_t*) ((uint8_t*) d3dlr.pBits + y * d3dlr.Pitch);
			for (uint32_t x = 0; x < m_dwTexWidth; x++)
			{
				uint8_t bAlpha = (uint8_t) (m_coverage[(size_t) y * m_dwTexWidth + x] >> 4);
				*pDst16++      = bAlpha > 0 ? (uint16_t) ((bAlpha << 12) | 0x0fff) : 0;
			}
		}
		m_pTexture->UnlockRect(0);
	}
	m_coverage.clear();
	m_coverage.shrink_to_fit();

	if (dwFlags & D3DFONT_FILTERED)
	{
		int iMMC = m_pTexture->GetLevelCount();
		for (int i = 1; i < iMMC; i++)
		{
			LPDIRECT3DSURFACE9 lpSurfSrc = nullptr, lpSurfDest = nullptr;
			m_pTexture->GetSurfaceLevel(i - 1, &lpSurfSrc);
			m_pTexture->GetSurfaceLevel(i, &lpSurfDest);
			if (lpSurfSrc && lpSurfDest)
				::D3DXLoadSurfaceFromSurface(lpSurfDest, nullptr, nullptr, lpSurfSrc, nullptr, nullptr, D3DX_FILTER_TRIANGLE, 0);
			if (lpSurfSrc) lpSurfSrc->Release();
			if (lpSurfDest) lpSurfDest->Release();
		}
	}
	return S_OK;
}

void CDFont::Make2DVertex(const int iFontHeight, const std::string& szText)
{
	if (m_pVB == nullptr || m_ftFace == nullptr)
	{
		__ASSERT(0, "NULL Vertex Buffer or Font");
		return;
	}
	if (szText.empty())
		return;

	__VertexTransformed* pVertices = nullptr;
	uint32_t dwNumTriangles        = 0;
	if (FAILED(m_pVB->Lock(0, 0, (void**) &pVertices, 0)))
		return;

	uint32_t sx = 0, x = 0, y = 0;
	float vtx_sx = 0, vtx_sy = 0;
	uint32_t dwColor = 0xffffffff;
	m_dwFontColor    = 0xffffffff;
	float fMaxX = 0.0f, fMaxY = 0.0f;

	auto emitRun = [&]() -> bool {
		if (sx == x)
			return true;
		float tx1 = ((float) sx) / m_dwTexWidth;
		float ty1 = ((float) y) / m_dwTexHeight;
		float tx2 = ((float) x) / m_dwTexWidth;
		float ty2 = ((float) (y + iFontHeight)) / m_dwTexHeight;
		float w   = (tx2 - tx1) * m_dwTexWidth / m_fTextScale;
		float h   = (ty2 - ty1) * m_dwTexHeight / m_fTextScale;
		if ((dwNumTriangles + 2) * 3 > MAX_NUM_VERTICES) // tampon köşe sayısıyla sınırlı (üçgen × 3)
			return false;
		float fLeft = vtx_sx - 0.5f, fRight = vtx_sx + w - 0.5f, fTop = vtx_sy - 0.5f, fBottom = vtx_sy + h - 0.5f;
		pVertices->Set(fLeft, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty2); ++pVertices;
		pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1); ++pVertices;
		pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2); ++pVertices;
		pVertices->Set(fRight, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty1); ++pVertices;
		pVertices->Set(fRight, fBottom, Z_DEFAULT, RHW_DEFAULT, dwColor, tx2, ty2); ++pVertices;
		pVertices->Set(fLeft, fTop, Z_DEFAULT, RHW_DEFAULT, dwColor, tx1, ty1); ++pVertices;
		dwNumTriangles += 2;
		fMaxX = std::max(fMaxX, fRight);
		fMaxY = std::max(fMaxY, fBottom);
		return true;
	};

	size_t iCount = 0, iStrLen = szText.size();
	while (iCount < iStrLen)
	{
		if ('\n' == szText[iCount])
		{
			++iCount;
			if (!emitRun())
				break;
			// Ekranda alt satıra geç; dokuda aynı satırda devam
			sx     = x;
			vtx_sx = 0;
			vtx_sy = vtx_sy + ((float) iFontHeight) / m_fTextScale;
			continue;
		}
		std::string chunk;
		if (KoTextCharLen(szText[iCount]) == 2)
		{
			if (iCount + 1 >= iStrLen)
				break;
			chunk = szText.substr(iCount, 2);
			iCount += 2;
		}
		else
		{
			chunk = szText.substr(iCount, 1);
			iCount += 1;
		}
		std::vector<uint32_t> cps = DecodeText(chunk);
		int cw                    = MeasureWidth(cps.data(), cps.size());
		if ((x + cw) > m_dwTexWidth)
		{
			// Çalışan parçayı bitir, dokuda bir alt satıra geç
			float runW = (float) (x - sx) / m_fTextScale;
			if (!emitRun())
				break;
			x = sx = 0;
			y += iFontHeight;
			vtx_sx = vtx_sx + runW;
			if (y + iFontHeight > m_dwTexHeight)
				break; // doku doldu
		}
		DrawGlyphs(cps.data(), cps.size(), (int) x, (int) y);
		x += cw;
	}
	emitRun();

	m_pVB->Unlock();
	m_iPrimitiveCount = dwNumTriangles;
	m_PrevLeftTop     = {};
	m_Size.cx         = (long) fMaxX;
	m_Size.cy         = (long) fMaxY;
}

HRESULT CDFont::DrawText(FLOAT sx, FLOAT sy, uint32_t dwColor, uint32_t dwFlags)
{
	if (m_pVB == nullptr || m_ftFace == nullptr)
		return E_FAIL;
	if (m_iPrimitiveCount <= 0)
		return S_OK;
	if (m_pd3dDevice == nullptr || m_pTexture == nullptr)
		return E_FAIL; // doku yokken (SetText başarısız / aygıt sıfırlandı) çizim yapılmaz
	if (m_iPrimitiveCount * 3 > MAX_NUM_VERTICES)
	{
		m_iPrimitiveCount = 0;
		return E_FAIL;
	}

	__Vector2 vDiff = __Vector2(sx, sy) - m_PrevLeftTop;
	if (fabs(vDiff.x) > 0.5f || fabs(vDiff.y) > 0.5f || dwColor != m_dwFontColor)
	{
		__VertexTransformed* pVertices = nullptr;
		if (FAILED(m_pVB->Lock(0, 0, (void**) &pVertices, 0)))
			return E_FAIL;
		int iVC = m_iPrimitiveCount * 3;
		if (fabs(vDiff.x) > 0.5f)
		{
			m_PrevLeftTop.x = sx;
			for (int i = 0; i < iVC; i++)
				pVertices[i].x += vDiff.x;
		}
		if (fabs(vDiff.y) > 0.5f)
		{
			m_PrevLeftTop.y = sy;
			for (int i = 0; i < iVC; i++)
				pVertices[i].y += vDiff.y;
		}
		if (dwColor != m_dwFontColor)
		{
			m_dwFontColor   = dwColor;
			m_PrevLeftTop.y = sy;
			for (int i = 0; i < iVC; i++)
				pVertices[i].color = m_dwFontColor;
		}
		m_pVB->Unlock();
	}

	DWORD dwAlphaBlend = 0, dwSrcBlend = 0, dwDestBlend = 0, dwZEnable = 0, dwFog = 0;
	DWORD dwColorOp = 0, dwColorArg1 = 0, dwColorArg2 = 0, dwAlphaOp = 0, dwAlphaArg1 = 0, dwAlphaArg2 = 0, dwMinFilter = 0, dwMagFilter = 0;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend);
	m_pd3dDevice->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	m_pd3dDevice->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable);
	m_pd3dDevice->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwColorOp);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg1);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLORARG2, &dwColorArg2);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAlphaOp);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAlphaArg1);
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_ALPHAARG2, &dwAlphaArg2);
	m_pd3dDevice->GetSamplerState(0, D3DSAMP_MINFILTER, &dwMinFilter);
	m_pd3dDevice->GetSamplerState(0, D3DSAMP_MAGFILTER, &dwMagFilter);

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	DWORD filter = (dwFlags & D3DFONT_FILTERED) ? D3DTEXF_LINEAR : D3DTEXF_POINT;
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, filter);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, filter);

	m_pd3dDevice->SetFVF(FVF_TRANSFORMED);
	m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(__VertexTransformed));
	m_pd3dDevice->SetTexture(0, m_pTexture);
	m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_iPrimitiveCount);

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend);
	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable);
	m_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, dwFog);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwColorOp);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg1);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, dwColorArg2);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAlphaOp);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAlphaArg1);
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, dwAlphaArg2);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, dwMinFilter);
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, dwMagFilter);
	return S_OK;
}

BOOL CDFont::GetTextExtent(const std::string& szString, int iStrLen, SIZE* pSize)
{
	if (!m_ftFace && !LoadFace())
		return FALSE;
	if (iStrLen < 0 || (size_t) iStrLen > szString.size())
		iStrLen = (int) szString.size();
	std::vector<uint32_t> cps = DecodeText(szString.substr(0, (size_t) iStrLen));
	pSize->cx                 = MeasureWidth(cps.data(), cps.size());
	pSize->cy                 = m_iLineHeight;
	return TRUE;
}

HRESULT CDFont::SetFontColor(uint32_t dwColor)
{
	if (m_iPrimitiveCount <= 0 || m_pVB == nullptr)
		return E_FAIL;
	if (dwColor == m_dwFontColor)
		return S_OK;
	__VertexTransformed* pVertices = nullptr;
	HRESULT hr                     = m_pVB->Lock(0, 0, (void**) &pVertices, 0);
	if (FAILED(hr))
		return hr;
	m_dwFontColor = dwColor;
	int iVC       = m_iPrimitiveCount * 3;
	for (int i = 0; i < iVC; ++i)
		pVertices[i].color = m_dwFontColor;
	m_pVB->Unlock();
	return S_OK;
}

#endif // !_WIN32
