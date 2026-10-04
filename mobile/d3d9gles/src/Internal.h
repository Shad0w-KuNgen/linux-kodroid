// Internal.h — d3d9gles iç yapıları (dışarıya açılmaz).
#ifndef D3D9GLES_INTERNAL_H
#define D3D9GLES_INTERNAL_H

#include <d3d9.h>

#include <GLES3/gl3.h>

#include <array>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

namespace d3d9gles
{
inline constexpr int MAX_STAGES  = 8; // D3D doku aşaması sayısı (izlenen)
inline constexpr int MAX_FF_STAGES = 4; // shader'da gerçekten birleştirilen aşama sayısı
inline constexpr int MAX_LIGHTS  = 8;

// Köşe öznitelik yuvaları (glBindAttribLocation ile sabitlenir)
enum Attrib : GLuint
{
	ATTR_POSITION = 0,
	ATTR_NORMAL   = 1,
	ATTR_DIFFUSE  = 2,
	ATTR_SPECULAR = 3,
	ATTR_PSIZE    = 4,
	ATTR_TEX0     = 5, // ATTR_TEX0 + i
};

struct FvfLayout
{
	DWORD fvf        = 0;
	UINT stride      = 0;
	bool rhw         = false;
	bool hasNormal   = false;
	bool hasDiffuse  = false;
	bool hasSpecular = false;
	bool hasPSize    = false;
	UINT texCount    = 0;
	UINT posOffset   = 0;
	UINT normalOffset = 0, diffuseOffset = 0, specularOffset = 0, psizeOffset = 0;
	UINT texOffset[8] = {};
};

FvfLayout ComputeFvfLayout(DWORD fvf);

struct StageState
{
	DWORD colorOp = D3DTOP_DISABLE, colorArg1 = D3DTA_TEXTURE, colorArg2 = D3DTA_CURRENT;
	DWORD alphaOp = D3DTOP_DISABLE, alphaArg1 = D3DTA_TEXTURE, alphaArg2 = D3DTA_CURRENT;
	DWORD texCoordIndex = 0;
	DWORD resultArg = D3DTA_CURRENT;
	DWORD textureTransformFlags = D3DTTFF_DISABLE;
	DWORD bumpEnv[4] = {};
};

struct SamplerState
{
	DWORD addressU = D3DTADDRESS_WRAP, addressV = D3DTADDRESS_WRAP, addressW = D3DTADDRESS_WRAP;
	DWORD borderColor = 0;
	DWORD magFilter = D3DTEXF_POINT, minFilter = D3DTEXF_POINT, mipFilter = D3DTEXF_NONE;
	DWORD lodBias = 0, maxMipLevel = 0, maxAnisotropy = 1;
};

/// Shader varyantını belirleyen anahtar. Işık tipleri vb. uniform olarak geçer; burada
/// yalnızca kod yapısını değiştiren alanlar var.
struct ProgramKey
{
	uint8_t rhw = 0, hasNormal = 0, hasDiffuse = 0, hasSpecular = 0, hasPSize = 0, texCount = 0;
	uint8_t lighting = 0, specularEnable = 0, colorVertex = 0;
	uint8_t diffuseSrc = 0, specularSrc = 0, ambientSrc = 0, emissiveSrc = 0;
	uint8_t fogMode = 0;   // 0 yok, 1 vertex, 2 table
	uint8_t fogKind = 0;   // D3DFOG_*
	uint8_t rangeFog = 0;
	uint8_t alphaTest = 0;
	uint8_t pointSprite = 0;
	uint8_t stageCount = 0; // etkin aşama sayısı
	struct Stage
	{
		uint8_t colorOp, colorArg1, colorArg2, alphaOp, alphaArg1, alphaArg2, texCoordIndex;
	} stage[MAX_FF_STAGES] = {};

	bool operator==(const ProgramKey& o) const
	{
		return std::memcmp(this, &o, sizeof(ProgramKey)) == 0;
	}
};

struct ProgramKeyHash
{
	size_t operator()(const ProgramKey& k) const
	{
		const uint8_t* p = reinterpret_cast<const uint8_t*>(&k);
		size_t h         = 1469598103934665603ull;
		for (size_t i = 0; i < sizeof(ProgramKey); ++i)
			h = (h ^ p[i]) * 1099511628211ull;
		return h;
	}
};

struct Program
{
	GLuint id = 0;
	// Uniform konumları
	GLint uWorld = -1, uView = -1, uWVP = -1, uViewportSize = -1, uEyePos = -1;
	GLint uGlobalAmbient = -1, uMatDiffuse = -1, uMatAmbient = -1, uMatSpecular = -1, uMatEmissive = -1, uMatPower = -1;
	GLint uLightCount = -1;
	GLint uLightType[MAX_LIGHTS], uLightDiffuse[MAX_LIGHTS], uLightSpecular[MAX_LIGHTS], uLightAmbient[MAX_LIGHTS];
	GLint uLightPos[MAX_LIGHTS], uLightDir[MAX_LIGHTS], uLightAtt[MAX_LIGHTS], uLightSpot[MAX_LIGHTS]; // att: range,a0,a1,a2 ; spot: cosHalfTheta, cosHalfPhi, falloff
	GLint uFogColor = -1, uFogParams = -1; // start, end, density
	GLint uTFactor = -1;
	GLint uAlphaRef = -1, uAlphaFunc = -1;
	GLint uPointSize = -1;
	GLint uTex[MAX_FF_STAGES];
};

struct LightState
{
	D3DLIGHT9 light {};
	bool enabled = false;
	bool set     = false;
};

struct DeviceImpl
{
	D3DPRESENT_PARAMETERS pp {};
	D3DCAPS9 caps {};
	bool s3tc = false;
	GLint maxTextureSize = 2048;

	// Durum
	std::array<DWORD, D3DRS_MAXSTATE> rs {};
	std::array<StageState, MAX_STAGES> stages {};
	std::array<SamplerState, MAX_STAGES> samplers {};
	std::array<IDirect3DBaseTexture9*, MAX_STAGES> textures {};
	D3DMATRIX world {}, view {}, proj {};
	std::array<D3DMATRIX, 8> texMatrix {};
	std::array<LightState, MAX_LIGHTS> lights {};
	D3DMATERIAL9 material {};
	D3DVIEWPORT9 viewport {};
	RECT scissor {};
	D3DCLIPSTATUS9 clipStatus {};
	DWORD fvf = 0;
	IDirect3DVertexBuffer9* streamVB = nullptr;
	UINT streamOffset = 0, streamStride = 0;
	IDirect3DIndexBuffer9* indices = nullptr;
	bool inScene = false;

	// İsteğe bağlı ölçekli çizim hedefi: arka tampon (mantıksal çözünürlük) fiziksel çizim
	// alanından farklıysa bir FBO'ya çizilir ve Present'te ekrana büyütülür (mobil performans/UI boyutu).
	GLuint fbo = 0, fboColor = 0, fboDepth = 0;
	int fboW = 0, fboH = 0;
	bool useFbo = false;
	void EnsureRenderTarget();
	void RenderTargetSize(int* w, int* h) const;

	// GL nesneleri
	GLuint vao = 0;
	GLuint streamVbo = 0, streamIbo = 0;
	std::array<GLuint, MAX_STAGES> glSamplers {};
	std::unordered_map<ProgramKey, Program, ProgramKeyHash> programs;
	GLuint whiteTexture = 0;

	// İstatistik
	unsigned drawCalls = 0;

	void InitGL();
	void ShutdownGL();
	void ApplyGLState(const FvfLayout& layout);
	Program& GetProgram(const ProgramKey& key);
	ProgramKey BuildKey(const FvfLayout& layout);
	void SetUniforms(Program& prog, const ProgramKey& key, const FvfLayout& layout);
	void BindVertexLayout(const FvfLayout& layout, GLuint vbo, size_t baseOffset);
	void DrawInternal(D3DPRIMITIVETYPE type, UINT primCount, GLuint vbo, size_t vertexBaseOffset, const FvfLayout& layout,
		GLuint ibo, GLenum indexType, size_t indexByteOffset, bool indexed);
	void GetDrawableSize(int* w, int* h) const;
};

// Yardımcılar
GLenum PrimitiveToGL(D3DPRIMITIVETYPE type, UINT primCount, UINT* vertexCount);
bool IsCompressedFormat(D3DFORMAT fmt);
UINT FormatBytesPerPixel(D3DFORMAT fmt);
UINT LevelByteSize(D3DFORMAT fmt, UINT w, UINT h);
UINT LevelPitch(D3DFORMAT fmt, UINT w);

/// D3D biçimindeki satır verisini GL'nin kabul ettiği biçime çevirir. Çıktı formatı
/// outInternal/outFormat/outType ile belirlenir; dönen vektör GL'ye verilecek veridir.
bool ConvertLevelToGL(D3DFORMAT fmt, UINT w, UINT h, const std::vector<uint8_t>& src, bool s3tcOK,
	std::vector<uint8_t>& out, GLenum* outInternal, GLenum* outFormat, GLenum* outType, bool* outCompressed);

/// DXT bloklarını RGBA8'e açar (S3TC desteklemeyen mobil GPU'lar için).
void DecodeDXT(D3DFORMAT fmt, UINT w, UINT h, const uint8_t* src, uint8_t* dstRGBA);

/// Herhangi bir desteklenen D3D formatından RGBA8 (float olmayan) satıra çevirme ve geri yazma.
void PixelsToRGBA8(D3DFORMAT fmt, UINT w, UINT h, const uint8_t* src, std::vector<uint8_t>& rgba);
void RGBA8ToPixels(D3DFORMAT fmt, UINT w, UINT h, const uint8_t* rgba, uint8_t* dst);

std::string BuildVertexShader(const ProgramKey& key);
std::string BuildFragmentShader(const ProgramKey& key);

void MatMul(const D3DMATRIX& a, const D3DMATRIX& b, D3DMATRIX& out); // D3D sırası: out = a * b
bool MatInverse(const D3DMATRIX& m, D3DMATRIX& out);
void MatIdentity(D3DMATRIX& m);

} // namespace d3d9gles

#endif // D3D9GLES_INTERNAL_H
