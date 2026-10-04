// Device.cpp — IDirect3D9 ve IDirect3DDevice9'un OpenGL ES 3 uygulaması.
#include "Internal.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#if defined(__ANDROID__)
#include <android/log.h>
#endif

namespace d3d9gles
{
namespace
{
PlatformHooks g_hooks;
}

void SetPlatformHooks(const PlatformHooks& hooks)
{
	g_hooks = hooks;
}

const PlatformHooks& GetPlatformHooks()
{
	return g_hooks;
}

void Log(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
#if defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_INFO, "d3d9gles", "%s", buf);
#else
	fprintf(stderr, "%s\n", buf);
#endif
}

// ---------------------------------------------------------------------------
// Matris yardımcıları (D3D satır-major, satır vektörü: v' = v * M)
// ---------------------------------------------------------------------------
void MatIdentity(D3DMATRIX& m)
{
	std::memset(&m, 0, sizeof(m));
	m._11 = m._22 = m._33 = m._44 = 1.0f;
}

void MatMul(const D3DMATRIX& a, const D3DMATRIX& b, D3DMATRIX& out)
{
	D3DMATRIX r;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
	out = r;
}

bool MatInverse(const D3DMATRIX& mm, D3DMATRIX& out)
{
	const float* m = &mm.m[0][0];
	float inv[16];
	inv[0]  = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
	inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
	inv[8]  = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
	inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
	inv[5]  = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
	inv[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
	inv[2]  = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
	inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
	inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
	inv[7]  = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	if (std::fabs(det) < 1e-20f)
		return false;
	det = 1.0f / det;
	float* o = &out.m[0][0];
	for (int i = 0; i < 16; ++i)
		o[i] = inv[i] * det;
	return true;
}

// ---------------------------------------------------------------------------
// FVF
// ---------------------------------------------------------------------------
FvfLayout ComputeFvfLayout(DWORD fvf)
{
	FvfLayout l;
	l.fvf       = fvf;
	UINT off    = 0;
	l.posOffset = 0;
	DWORD pos   = fvf & D3DFVF_POSITION_MASK;
	if (pos == D3DFVF_XYZRHW)
	{
		l.rhw = true;
		off += 16;
	}
	else if (pos == D3DFVF_XYZW)
		off += 16;
	else
	{
		off += 12;
		// Harman ağırlıkları (XYZB1..5) — kullanılmıyor ama atla
		if (pos >= D3DFVF_XYZB1 && pos <= D3DFVF_XYZB5)
			off += 4 * ((pos - D3DFVF_XYZ) / 2);
	}
	if (fvf & D3DFVF_NORMAL)
	{
		l.hasNormal    = true;
		l.normalOffset = off;
		off += 12;
	}
	if (fvf & D3DFVF_PSIZE)
	{
		l.hasPSize    = true;
		l.psizeOffset = off;
		off += 4;
	}
	if (fvf & D3DFVF_DIFFUSE)
	{
		l.hasDiffuse    = true;
		l.diffuseOffset = off;
		off += 4;
	}
	if (fvf & D3DFVF_SPECULAR)
	{
		l.hasSpecular    = true;
		l.specularOffset = off;
		off += 4;
	}
	l.texCount = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	if (l.texCount > 8)
		l.texCount = 8;
	for (UINT i = 0; i < l.texCount; ++i)
	{
		l.texOffset[i] = off;
		// D3DFVF_TEXCOORDSIZEn bayrakları: varsayılan 2 bileşen
		DWORD sizeBits = (fvf >> (16 + i * 2)) & 3;
		UINT comps     = sizeBits == D3DFVF_TEXTUREFORMAT1 ? 1 : sizeBits == D3DFVF_TEXTUREFORMAT3 ? 3 : sizeBits == D3DFVF_TEXTUREFORMAT4 ? 4 : 2;
		off += comps * 4;
	}
	l.stride = off;
	return l;
}

GLenum PrimitiveToGL(D3DPRIMITIVETYPE type, UINT primCount, UINT* vertexCount)
{
	switch (type)
	{
		case D3DPT_POINTLIST: *vertexCount = primCount; return GL_POINTS;
		case D3DPT_LINELIST: *vertexCount = primCount * 2; return GL_LINES;
		case D3DPT_LINESTRIP: *vertexCount = primCount + 1; return GL_LINE_STRIP;
		case D3DPT_TRIANGLELIST: *vertexCount = primCount * 3; return GL_TRIANGLES;
		case D3DPT_TRIANGLESTRIP: *vertexCount = primCount + 2; return GL_TRIANGLE_STRIP;
		case D3DPT_TRIANGLEFAN: *vertexCount = primCount + 2; return GL_TRIANGLE_FAN;
		default: *vertexCount = 0; return GL_TRIANGLES;
	}
}

// ---------------------------------------------------------------------------
// DeviceImpl
// ---------------------------------------------------------------------------
void DeviceImpl::GetDrawableSize(int* w, int* h) const
{
	if (g_hooks.getDrawableSize)
	{
		g_hooks.getDrawableSize(g_hooks.user, w, h);
		if (*w > 0 && *h > 0)
			return;
	}
	*w = (int) pp.BackBufferWidth;
	*h = (int) pp.BackBufferHeight;
}

void DeviceImpl::InitGL()
{
	const char* ext = (const char*) glGetString(GL_EXTENSIONS);
	s3tc            = ext && std::strstr(ext, "GL_EXT_texture_compression_s3tc") != nullptr;
	if (std::getenv("D3D9GLES_NO_S3TC"))
		s3tc = false;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &streamVbo);
	glGenBuffers(1, &streamIbo);
	glGenSamplers(MAX_STAGES, glSamplers.data());

	// Doku bağlı değilken örneklenen beyaz 1x1 doku
	glGenTextures(1, &whiteTexture);
	glBindTexture(GL_TEXTURE_2D, whiteTexture);
	const uint8_t white[4] = {255, 255, 255, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	Log("d3d9gles: GL %s | %s | S3TC=%d | maxTex=%d", (const char*) glGetString(GL_RENDERER), (const char*) glGetString(GL_VERSION), (int) s3tc, (int) maxTextureSize);

	// Yetenekler
	caps                   = {};
	caps.DeviceType        = D3DDEVTYPE_HAL;
	caps.DevCaps           = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
	caps.PrimitiveMiscCaps = D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW;
	caps.RasterCaps        = D3DPRASTERCAPS_FOGVERTEX | D3DPRASTERCAPS_FOGTABLE | D3DPRASTERCAPS_ZBIAS;
	caps.TextureCaps       = D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_MIPMAP;
	caps.TextureOpCaps     = D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 | D3DTEXOPCAPS_MODULATE | D3DTEXOPCAPS_ADD;
	caps.MaxTextureWidth = caps.MaxTextureHeight = (DWORD) maxTextureSize;
	caps.MaxTextureAspectRatio  = (DWORD) maxTextureSize;
	caps.MaxTextureRepeat       = 8192;
	caps.MaxAnisotropy          = 1;
	caps.MaxTextureBlendStages  = MAX_FF_STAGES;
	caps.MaxSimultaneousTextures = MAX_FF_STAGES;
	caps.VertexProcessingCaps   = D3DVTXPCAPS_DIRECTIONALLIGHTS | D3DVTXPCAPS_POSITIONALLIGHTS | D3DVTXPCAPS_LOCALVIEWER;
	caps.MaxActiveLights        = MAX_LIGHTS;
	caps.MaxPointSize           = 64.0f;
	caps.MaxPrimitiveCount      = 0xFFFFF;
	caps.MaxVertexIndex         = 0xFFFFF;
	caps.MaxStreams             = 1;
	caps.MaxStreamStride        = 256;
	caps.MaxVertexW             = 1e10f;
	caps.PresentationIntervals  = D3DPRESENT_INTERVAL_ONE | D3DPRESENT_INTERVAL_IMMEDIATE;
}

void DeviceImpl::ShutdownGL()
{
	for (auto& kv : programs)
		glDeleteProgram(kv.second.id);
	programs.clear();
	if (whiteTexture) glDeleteTextures(1, &whiteTexture);
	if (streamVbo) glDeleteBuffers(1, &streamVbo);
	if (streamIbo) glDeleteBuffers(1, &streamIbo);
	if (vao) glDeleteVertexArrays(1, &vao);
	glDeleteSamplers(MAX_STAGES, glSamplers.data());
	whiteTexture = streamVbo = streamIbo = vao = 0;
}

ProgramKey DeviceImpl::BuildKey(const FvfLayout& layout)
{
	ProgramKey k;
	k.rhw         = layout.rhw;
	k.hasNormal   = layout.hasNormal;
	k.hasDiffuse  = layout.hasDiffuse;
	k.hasSpecular = layout.hasSpecular;
	k.hasPSize    = layout.hasPSize;
	k.texCount    = (uint8_t) layout.texCount;
	k.lighting    = rs[D3DRS_LIGHTING] ? 1 : 0;
	k.specularEnable = rs[D3DRS_SPECULARENABLE] ? 1 : 0;
	k.colorVertex = rs[D3DRS_COLORVERTEX] ? 1 : 0;
	k.diffuseSrc  = (uint8_t) rs[D3DRS_DIFFUSEMATERIALSOURCE];
	k.specularSrc = (uint8_t) rs[D3DRS_SPECULARMATERIALSOURCE];
	k.ambientSrc  = (uint8_t) rs[D3DRS_AMBIENTMATERIALSOURCE];
	k.emissiveSrc = (uint8_t) rs[D3DRS_EMISSIVEMATERIALSOURCE];
	if (rs[D3DRS_FOGENABLE])
	{
		if (rs[D3DRS_FOGTABLEMODE] != D3DFOG_NONE)
		{
			k.fogMode = 2;
			k.fogKind = (uint8_t) rs[D3DRS_FOGTABLEMODE];
		}
		else if (rs[D3DRS_FOGVERTEXMODE] != D3DFOG_NONE)
		{
			k.fogMode = 1;
			k.fogKind = (uint8_t) rs[D3DRS_FOGVERTEXMODE];
		}
	}
	k.rangeFog  = rs[D3DRS_RANGEFOGENABLE] ? 1 : 0;
	k.alphaTest = (rs[D3DRS_ALPHATESTENABLE] && rs[D3DRS_ALPHAFUNC] != D3DCMP_ALWAYS) ? 1 : 0;

	int n = 0;
	for (int i = 0; i < MAX_FF_STAGES; ++i)
	{
		const StageState& st = stages[i];
		if (st.colorOp == D3DTOP_DISABLE)
			break;
		bool refsTex = ((st.colorArg1 & D3DTA_SELECTMASK) == D3DTA_TEXTURE) || ((st.colorArg2 & D3DTA_SELECTMASK) == D3DTA_TEXTURE)
					   || (st.alphaOp != D3DTOP_DISABLE && (((st.alphaArg1 & D3DTA_SELECTMASK) == D3DTA_TEXTURE) || ((st.alphaArg2 & D3DTA_SELECTMASK) == D3DTA_TEXTURE)));
		if (refsTex && textures[i] == nullptr)
			break; // D3D (ve Wine) davranışı: dokusu olmayan aşamada zincir biter
		k.stage[n].colorOp       = (uint8_t) st.colorOp;
		k.stage[n].colorArg1     = (uint8_t) st.colorArg1;
		k.stage[n].colorArg2     = (uint8_t) st.colorArg2;
		k.stage[n].alphaOp       = (uint8_t) st.alphaOp;
		k.stage[n].alphaArg1     = (uint8_t) st.alphaArg1;
		k.stage[n].alphaArg2     = (uint8_t) st.alphaArg2;
		k.stage[n].texCoordIndex = (uint8_t) (st.texCoordIndex & 0xFF);
		++n;
	}
	k.stageCount = (uint8_t) n;
	return k;
}

namespace
{
GLuint CompileShader(GLenum type, const std::string& src)
{
	GLuint sh      = glCreateShader(type);
	const char* cs = src.c_str();
	glShaderSource(sh, 1, &cs, nullptr);
	glCompileShader(sh);
	GLint ok = 0;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[2048];
		glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
		Log("d3d9gles: shader derleme hatası:\n%s\n--- kaynak ---\n%s", log, src.c_str());
		glDeleteShader(sh);
		return 0;
	}
	return sh;
}
} // namespace

Program& DeviceImpl::GetProgram(const ProgramKey& key)
{
	auto it = programs.find(key);
	if (it != programs.end())
		return it->second;

	Program p;
	std::string vsSrc = BuildVertexShader(key);
	std::string fsSrc = BuildFragmentShader(key);
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);
	p.id      = glCreateProgram();
	glAttachShader(p.id, vs);
	glAttachShader(p.id, fs);
	glLinkProgram(p.id);
	GLint ok = 0;
	glGetProgramiv(p.id, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[2048];
		glGetProgramInfoLog(p.id, sizeof(log), nullptr, log);
		Log("d3d9gles: program bağlama hatası: %s", log);
	}
	glDeleteShader(vs);
	glDeleteShader(fs);

	auto U = [&](const char* n) { return glGetUniformLocation(p.id, n); };
	p.uWorld = U("uWorld"); p.uView = U("uView"); p.uWVP = U("uWVP");
	p.uViewportSize = U("uViewportSize"); p.uEyePos = U("uEyePos");
	p.uGlobalAmbient = U("uGlobalAmbient"); p.uMatDiffuse = U("uMatDiffuse"); p.uMatAmbient = U("uMatAmbient");
	p.uMatSpecular = U("uMatSpecular"); p.uMatEmissive = U("uMatEmissive"); p.uMatPower = U("uMatPower");
	p.uLightCount = U("uLightCount");
	char name[64];
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		snprintf(name, sizeof(name), "uLightType[%d]", i); p.uLightType[i] = U(name);
		snprintf(name, sizeof(name), "uLightDiffuse[%d]", i); p.uLightDiffuse[i] = U(name);
		snprintf(name, sizeof(name), "uLightSpecular[%d]", i); p.uLightSpecular[i] = U(name);
		snprintf(name, sizeof(name), "uLightAmbient[%d]", i); p.uLightAmbient[i] = U(name);
		snprintf(name, sizeof(name), "uLightPos[%d]", i); p.uLightPos[i] = U(name);
		snprintf(name, sizeof(name), "uLightDir[%d]", i); p.uLightDir[i] = U(name);
		snprintf(name, sizeof(name), "uLightAtt[%d]", i); p.uLightAtt[i] = U(name);
		snprintf(name, sizeof(name), "uLightSpot[%d]", i); p.uLightSpot[i] = U(name);
	}
	p.uFogColor = U("uFogColor"); p.uFogParams = U("uFogParams"); p.uTFactor = U("uTFactor");
	p.uAlphaRef = U("uAlphaRef"); p.uAlphaFunc = U("uAlphaFunc"); p.uPointSize = U("uPointSize");
	for (int i = 0; i < MAX_FF_STAGES; ++i)
	{
		snprintf(name, sizeof(name), "uTex%d", i);
		p.uTex[i] = U(name);
	}
	auto res = programs.emplace(key, p);
	return res.first->second;
}

namespace
{
inline void ColorToFloat4(D3DCOLOR c, float* f)
{
	f[0] = ((c >> 16) & 0xFF) / 255.0f;
	f[1] = ((c >> 8) & 0xFF) / 255.0f;
	f[2] = (c & 0xFF) / 255.0f;
	f[3] = ((c >> 24) & 0xFF) / 255.0f;
}
inline float AsFloat(DWORD v)
{
	return std::bit_cast<float>(v);
}
} // namespace

void DeviceImpl::SetUniforms(Program& p, const ProgramKey& key, const FvfLayout&)
{
	int dw, dh;
	GetDrawableSize(&dw, &dh);

	D3DMATRIX wv, wvp;
	MatMul(world, view, wv);
	MatMul(wv, proj, wvp);
	if (p.uWorld >= 0) glUniformMatrix4fv(p.uWorld, 1, GL_FALSE, &world.m[0][0]);
	if (p.uView >= 0) glUniformMatrix4fv(p.uView, 1, GL_FALSE, &view.m[0][0]);
	if (p.uWVP >= 0) glUniformMatrix4fv(p.uWVP, 1, GL_FALSE, &wvp.m[0][0]);
	if (p.uViewportSize >= 0) glUniform2f(p.uViewportSize, (float) (viewport.Width ? viewport.Width : (DWORD) dw), (float) (viewport.Height ? viewport.Height : (DWORD) dh));
	if (p.uEyePos >= 0)
	{
		D3DMATRIX inv;
		if (MatInverse(view, inv))
			glUniform3f(p.uEyePos, inv._41, inv._42, inv._43);
		else
			glUniform3f(p.uEyePos, 0, 0, 0);
	}
	if (key.lighting)
	{
		float amb[4];
		ColorToFloat4(rs[D3DRS_AMBIENT], amb);
		if (p.uGlobalAmbient >= 0) glUniform4fv(p.uGlobalAmbient, 1, amb);
		if (p.uMatDiffuse >= 0) glUniform4fv(p.uMatDiffuse, 1, &material.Diffuse.r);
		if (p.uMatAmbient >= 0) glUniform4fv(p.uMatAmbient, 1, &material.Ambient.r);
		if (p.uMatSpecular >= 0) glUniform4fv(p.uMatSpecular, 1, &material.Specular.r);
		if (p.uMatEmissive >= 0) glUniform4fv(p.uMatEmissive, 1, &material.Emissive.r);
		if (p.uMatPower >= 0) glUniform1f(p.uMatPower, material.Power);
		int count = 0;
		for (int i = 0; i < MAX_LIGHTS; ++i)
			if (lights[i].enabled && lights[i].set)
				count = i + 1;
		if (p.uLightCount >= 0) glUniform1i(p.uLightCount, count);
		for (int i = 0; i < count; ++i)
		{
			const LightState& ls = lights[i];
			int type             = (ls.enabled && ls.set) ? (int) ls.light.Type : 0;
			if (p.uLightType[i] >= 0) glUniform1i(p.uLightType[i], type);
			if (type == 0)
				continue;
			const D3DLIGHT9& L = ls.light;
			if (p.uLightDiffuse[i] >= 0) glUniform4fv(p.uLightDiffuse[i], 1, &L.Diffuse.r);
			if (p.uLightSpecular[i] >= 0) glUniform4fv(p.uLightSpecular[i], 1, &L.Specular.r);
			if (p.uLightAmbient[i] >= 0) glUniform4fv(p.uLightAmbient[i], 1, &L.Ambient.r);
			if (p.uLightPos[i] >= 0) glUniform3f(p.uLightPos[i], L.Position.x, L.Position.y, L.Position.z);
			if (p.uLightDir[i] >= 0) glUniform3f(p.uLightDir[i], L.Direction.x, L.Direction.y, L.Direction.z);
			if (p.uLightAtt[i] >= 0) glUniform4f(p.uLightAtt[i], L.Range > 0 ? L.Range : 1e30f, L.Attenuation0, L.Attenuation1, L.Attenuation2);
			if (p.uLightSpot[i] >= 0) glUniform3f(p.uLightSpot[i], std::cos(L.Theta * 0.5f), std::cos(L.Phi * 0.5f), L.Falloff);
		}
	}
	if (key.fogMode)
	{
		float fc[4];
		ColorToFloat4(rs[D3DRS_FOGCOLOR], fc);
		if (p.uFogColor >= 0) glUniform4fv(p.uFogColor, 1, fc);
		if (p.uFogParams >= 0) glUniform3f(p.uFogParams, AsFloat(rs[D3DRS_FOGSTART]), AsFloat(rs[D3DRS_FOGEND]), AsFloat(rs[D3DRS_FOGDENSITY]));
	}
	float tf[4];
	ColorToFloat4(rs[D3DRS_TEXTUREFACTOR], tf);
	if (p.uTFactor >= 0) glUniform4fv(p.uTFactor, 1, tf);
	if (key.alphaTest)
	{
		if (p.uAlphaRef >= 0) glUniform1f(p.uAlphaRef, (rs[D3DRS_ALPHAREF] & 0xFF) / 255.0f);
		if (p.uAlphaFunc >= 0) glUniform1i(p.uAlphaFunc, (int) rs[D3DRS_ALPHAFUNC]);
	}
	if (p.uPointSize >= 0) glUniform1f(p.uPointSize, std::max(1.0f, AsFloat(rs[D3DRS_POINTSIZE])));
	for (int i = 0; i < key.stageCount; ++i)
		if (p.uTex[i] >= 0) glUniform1i(p.uTex[i], i);
}

namespace
{
GLenum BlendToGL(DWORD b)
{
	switch (b)
	{
		case D3DBLEND_ZERO: return GL_ZERO;
		case D3DBLEND_ONE: return GL_ONE;
		case D3DBLEND_SRCCOLOR: return GL_SRC_COLOR;
		case D3DBLEND_INVSRCCOLOR: return GL_ONE_MINUS_SRC_COLOR;
		case D3DBLEND_SRCALPHA: return GL_SRC_ALPHA;
		case D3DBLEND_INVSRCALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case D3DBLEND_DESTALPHA: return GL_DST_ALPHA;
		case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case D3DBLEND_DESTCOLOR: return GL_DST_COLOR;
		case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
		case D3DBLEND_SRCALPHASAT: return GL_SRC_ALPHA_SATURATE;
		case D3DBLEND_BOTHSRCALPHA: return GL_SRC_ALPHA;
		case D3DBLEND_BOTHINVSRCALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case D3DBLEND_BLENDFACTOR: return GL_CONSTANT_COLOR;
		case D3DBLEND_INVBLENDFACTOR: return GL_ONE_MINUS_CONSTANT_COLOR;
		default: return GL_ONE;
	}
}
GLenum CmpToGL(DWORD c)
{
	switch (c)
	{
		case D3DCMP_NEVER: return GL_NEVER;
		case D3DCMP_LESS: return GL_LESS;
		case D3DCMP_EQUAL: return GL_EQUAL;
		case D3DCMP_LESSEQUAL: return GL_LEQUAL;
		case D3DCMP_GREATER: return GL_GREATER;
		case D3DCMP_NOTEQUAL: return GL_NOTEQUAL;
		case D3DCMP_GREATEREQUAL: return GL_GEQUAL;
		default: return GL_ALWAYS;
	}
}
GLenum StencilOpToGL(DWORD o)
{
	switch (o)
	{
		case D3DSTENCILOP_ZERO: return GL_ZERO;
		case D3DSTENCILOP_REPLACE: return GL_REPLACE;
		case D3DSTENCILOP_INCRSAT: return GL_INCR;
		case D3DSTENCILOP_DECRSAT: return GL_DECR;
		case D3DSTENCILOP_INVERT: return GL_INVERT;
		case D3DSTENCILOP_INCR: return GL_INCR_WRAP;
		case D3DSTENCILOP_DECR: return GL_DECR_WRAP;
		default: return GL_KEEP;
	}
}
GLenum AddressToGL(DWORD a)
{
	switch (a)
	{
		case D3DTADDRESS_MIRROR:
		case D3DTADDRESS_MIRRORONCE: return GL_MIRRORED_REPEAT;
		case D3DTADDRESS_CLAMP:
		case D3DTADDRESS_BORDER: return GL_CLAMP_TO_EDGE;
		default: return GL_REPEAT;
	}
}
GLenum MinFilterToGL(DWORD minf, DWORD mipf)
{
	bool linear = minf != D3DTEXF_POINT && minf != D3DTEXF_NONE;
	switch (mipf)
	{
		case D3DTEXF_NONE: return linear ? GL_LINEAR : GL_NEAREST;
		case D3DTEXF_POINT: return linear ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
		default: return linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
	}
}
} // namespace

void DeviceImpl::ApplyGLState(const FvfLayout&)
{
	int dw, dh;
	GetDrawableSize(&dw, &dh);

	// Görünüm alanı (D3D: y yukarıdan; GL: y aşağıdan)
	glViewport((GLint) viewport.X, (GLint) (dh - (int) (viewport.Y + viewport.Height)), (GLsizei) viewport.Width, (GLsizei) viewport.Height);
	glDepthRangef(viewport.MinZ, viewport.MaxZ);

	// Derinlik
	if (rs[D3DRS_ZENABLE] != D3DZB_FALSE)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(CmpToGL(rs[D3DRS_ZFUNC]));
	}
	else
		glDisable(GL_DEPTH_TEST);
	glDepthMask(rs[D3DRS_ZWRITEENABLE] ? GL_TRUE : GL_FALSE);

	// Karıştırma
	if (rs[D3DRS_ALPHABLENDENABLE])
	{
		glEnable(GL_BLEND);
		glBlendFunc(BlendToGL(rs[D3DRS_SRCBLEND]), BlendToGL(rs[D3DRS_DESTBLEND]));
		switch (rs[D3DRS_BLENDOP])
		{
			case D3DBLENDOP_SUBTRACT: glBlendEquation(GL_FUNC_SUBTRACT); break;
			case D3DBLENDOP_REVSUBTRACT: glBlendEquation(GL_FUNC_REVERSE_SUBTRACT); break;
			case D3DBLENDOP_MIN: glBlendEquation(GL_MIN); break;
			case D3DBLENDOP_MAX: glBlendEquation(GL_MAX); break;
			default: glBlendEquation(GL_FUNC_ADD); break;
		}
		float bf[4];
		ColorToFloat4(rs[D3DRS_BLENDFACTOR], bf);
		glBlendColor(bf[0], bf[1], bf[2], bf[3]);
	}
	else
		glDisable(GL_BLEND);

	// Yüz ayıklama: D3DCULL_CCW → görsel olarak saat yönünün tersi üçgenleri at (D3D ön yüz = saat yönü)
	switch (rs[D3DRS_CULLMODE])
	{
		case D3DCULL_CW:
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CCW);
			glCullFace(GL_BACK);
			break;
		case D3DCULL_CCW:
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CW);
			glCullFace(GL_BACK);
			break;
		default:
			glDisable(GL_CULL_FACE);
			break;
	}

	// Makas
	if (rs[D3DRS_SCISSORTESTENABLE])
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(scissor.left, dh - scissor.bottom, std::max(0, (int) (scissor.right - scissor.left)), std::max(0, (int) (scissor.bottom - scissor.top)));
	}
	else
		glDisable(GL_SCISSOR_TEST);

	// Derinlik ofseti (ZBIAS: D3D8 tamsayı 0..16; DEPTHBIAS: float)
	float zbias = (float) (int) rs[D3DRS_ZBIAS];
	float dbias = AsFloat(rs[D3DRS_DEPTHBIAS]);
	float sbias = AsFloat(rs[D3DRS_SLOPESCALEDEPTHBIAS]);
	if (zbias != 0.0f || dbias != 0.0f || sbias != 0.0f)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(sbias, -zbias + dbias * 16777215.0f);
	}
	else
		glDisable(GL_POLYGON_OFFSET_FILL);

	// Renk yazma maskesi
	DWORD cw = rs[D3DRS_COLORWRITEENABLE];
	glColorMask((cw & 1) != 0, (cw & 2) != 0, (cw & 4) != 0, (cw & 8) != 0);

	// Dither
	if (rs[D3DRS_DITHERENABLE]) glEnable(GL_DITHER); else glDisable(GL_DITHER);

	// Stencil
	if (rs[D3DRS_STENCILENABLE])
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(CmpToGL(rs[D3DRS_STENCILFUNC]), (GLint) rs[D3DRS_STENCILREF], rs[D3DRS_STENCILMASK]);
		glStencilOp(StencilOpToGL(rs[D3DRS_STENCILFAIL]), StencilOpToGL(rs[D3DRS_STENCILZFAIL]), StencilOpToGL(rs[D3DRS_STENCILPASS]));
		glStencilMask(rs[D3DRS_STENCILWRITEMASK]);
	}
	else
		glDisable(GL_STENCIL_TEST);
}

void DeviceImpl::BindVertexLayout(const FvfLayout& l, GLuint vbo, size_t base)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	auto ptr = [&](UINT off) { return reinterpret_cast<const void*>(base + off); };
	glEnableVertexAttribArray(ATTR_POSITION);
	glVertexAttribPointer(ATTR_POSITION, l.rhw ? 4 : 3, GL_FLOAT, GL_FALSE, (GLsizei) l.stride, ptr(l.posOffset));
	if (l.hasNormal)
	{
		glEnableVertexAttribArray(ATTR_NORMAL);
		glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, (GLsizei) l.stride, ptr(l.normalOffset));
	}
	else
		glDisableVertexAttribArray(ATTR_NORMAL);
	if (l.hasDiffuse)
	{
		glEnableVertexAttribArray(ATTR_DIFFUSE);
		glVertexAttribPointer(ATTR_DIFFUSE, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei) l.stride, ptr(l.diffuseOffset));
	}
	else
		glDisableVertexAttribArray(ATTR_DIFFUSE);
	if (l.hasSpecular)
	{
		glEnableVertexAttribArray(ATTR_SPECULAR);
		glVertexAttribPointer(ATTR_SPECULAR, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei) l.stride, ptr(l.specularOffset));
	}
	else
		glDisableVertexAttribArray(ATTR_SPECULAR);
	if (l.hasPSize)
	{
		glEnableVertexAttribArray(ATTR_PSIZE);
		glVertexAttribPointer(ATTR_PSIZE, 1, GL_FLOAT, GL_FALSE, (GLsizei) l.stride, ptr(l.psizeOffset));
	}
	else
		glDisableVertexAttribArray(ATTR_PSIZE);
	for (UINT i = 0; i < 8; ++i)
	{
		if (i < l.texCount)
		{
			glEnableVertexAttribArray(ATTR_TEX0 + i);
			glVertexAttribPointer(ATTR_TEX0 + i, 2, GL_FLOAT, GL_FALSE, (GLsizei) l.stride, ptr(l.texOffset[i]));
		}
		else
			glDisableVertexAttribArray(ATTR_TEX0 + i);
	}
}

void DeviceImpl::DrawInternal(D3DPRIMITIVETYPE type, UINT primCount, GLuint vbo, size_t vertexBase, const FvfLayout& layout,
	GLuint ibo, GLenum indexType, size_t indexByteOffset, bool indexed)
{
	if (primCount == 0)
		return;
	UINT vertexCount = 0;
	GLenum mode      = PrimitiveToGL(type, primCount, &vertexCount);
	if (vertexCount == 0)
		return;

	ProgramKey key = BuildKey(layout);
	Program& prog  = GetProgram(key);
	if (prog.id == 0)
		return;
	glUseProgram(prog.id);

	glBindVertexArray(vao);
	ApplyGLState(layout);
	SetUniforms(prog, key, layout);
	BindVertexLayout(layout, vbo, vertexBase);

	// Dokular ve örnekleyiciler
	for (int i = 0; i < key.stageCount; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		IDirect3DBaseTexture9* t = textures[i];
		glBindTexture(GL_TEXTURE_2D, t ? t->GLName() : whiteTexture);
		const SamplerState& ss = samplers[i];
		GLuint smp             = glSamplers[i];
		glSamplerParameteri(smp, GL_TEXTURE_WRAP_S, (GLint) AddressToGL(ss.addressU));
		glSamplerParameteri(smp, GL_TEXTURE_WRAP_T, (GLint) AddressToGL(ss.addressV));
		glSamplerParameteri(smp, GL_TEXTURE_MAG_FILTER, (ss.magFilter == D3DTEXF_POINT || ss.magFilter == D3DTEXF_NONE) ? GL_NEAREST : GL_LINEAR);
		glSamplerParameteri(smp, GL_TEXTURE_MIN_FILTER, (GLint) MinFilterToGL(ss.minFilter, ss.mipFilter));
		glBindSampler((GLuint) i, smp);
	}
	for (int i = key.stageCount; i < MAX_FF_STAGES; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindSampler((GLuint) i, 0);
	}

	if (indexed)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(mode, (GLsizei) vertexCount, indexType, reinterpret_cast<const void*>(indexByteOffset));
	}
	else
		glDrawArrays(mode, 0, (GLsizei) vertexCount);
	++drawCalls;
}

} // namespace d3d9gles

using namespace d3d9gles;

// ---------------------------------------------------------------------------
// IDirect3D9
// ---------------------------------------------------------------------------
IDirect3D9* Direct3DCreate9(UINT)
{
	return new IDirect3D9();
}

IDirect3D9::IDirect3D9() = default;

HRESULT IDirect3D9::GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER9* id)
{
	*id = {};
	std::snprintf(id->Driver, sizeof(id->Driver), "d3d9gles");
	std::snprintf(id->Description, sizeof(id->Description), "Direct3D 9 on OpenGL ES 3 (d3d9gles)");
	std::snprintf(id->DeviceName, sizeof(id->DeviceName), "\\\\.\\DISPLAY1");
	return D3D_OK;
}

UINT IDirect3D9::GetAdapterModeCount(UINT, D3DFORMAT fmt)
{
	return fmt == D3DFMT_X8R8G8B8 ? 1 : 0;
}

HRESULT IDirect3D9::GetAdapterDisplayMode(UINT, D3DDISPLAYMODE* out)
{
	int w = 1024, h = 768;
	if (g_hooks.getDrawableSize)
		g_hooks.getDrawableSize(g_hooks.user, &w, &h);
	out->Width       = (UINT) w;
	out->Height      = (UINT) h;
	out->RefreshRate = 60;
	out->Format      = D3DFMT_X8R8G8B8;
	return D3D_OK;
}

HRESULT IDirect3D9::EnumAdapterModes(UINT adapter, D3DFORMAT fmt, UINT mode, D3DDISPLAYMODE* out)
{
	if (fmt != D3DFMT_X8R8G8B8 || mode != 0)
		return D3DERR_INVALIDCALL;
	return GetAdapterDisplayMode(adapter, out);
}

HRESULT IDirect3D9::CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD usage, D3DRESOURCETYPE rtype, D3DFORMAT fmt)
{
	if (usage & D3DUSAGE_DEPTHSTENCIL)
		return (fmt == D3DFMT_D16 || fmt == D3DFMT_D24S8 || fmt == D3DFMT_D24X8) ? D3D_OK : D3DERR_NOTAVAILABLE;
	if (rtype == D3DRTYPE_TEXTURE || rtype == D3DRTYPE_SURFACE)
	{
		switch (fmt)
		{
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
			case D3DFMT_R8G8B8:
			case D3DFMT_R5G6B5:
			case D3DFMT_A1R5G5B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A4R4G4B4:
			case D3DFMT_A8:
			case D3DFMT_L8:
			case D3DFMT_A8L8:
			case D3DFMT_DXT1:
			case D3DFMT_DXT2:
			case D3DFMT_DXT3:
			case D3DFMT_DXT4:
			case D3DFMT_DXT5: return D3D_OK; // DXT: donanım yoksa CPU'da açılır
			default: return D3DERR_NOTAVAILABLE;
		}
	}
	return D3D_OK;
}

HRESULT IDirect3D9::CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT dsFmt)
{
	return (dsFmt == D3DFMT_D16 || dsFmt == D3DFMT_D24S8 || dsFmt == D3DFMT_D24X8) ? D3D_OK : D3DERR_NOTAVAILABLE;
}

HRESULT IDirect3D9::GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS9* caps)
{
	*caps                 = {};
	caps->DeviceType      = D3DDEVTYPE_HAL;
	caps->MaxTextureWidth = caps->MaxTextureHeight = 2048;
	caps->TextureCaps     = D3DPTEXTURECAPS_MIPMAP | D3DPTEXTURECAPS_ALPHA;
	caps->MaxActiveLights = MAX_LIGHTS;
	return D3D_OK;
}

HRESULT IDirect3D9::CreateDevice(UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS* pp, IDirect3DDevice9** out)
{
	if (!pp || !out)
		return D3DERR_INVALIDCALL;
	AddRef();
	*out = new IDirect3DDevice9(this, *pp);
	return D3D_OK;
}

// ---------------------------------------------------------------------------
// IDirect3DDevice9
// ---------------------------------------------------------------------------
IDirect3DDevice9::IDirect3DDevice9(IDirect3D9* parent, const D3DPRESENT_PARAMETERS& pp) : m_parent(parent), m_impl(new DeviceImpl())
{
	DeviceImpl& d = *m_impl;
	d.pp          = pp;
	int w, h;
	d.GetDrawableSize(&w, &h);
	if (d.pp.BackBufferWidth == 0) d.pp.BackBufferWidth = (UINT) w;
	if (d.pp.BackBufferHeight == 0) d.pp.BackBufferHeight = (UINT) h;

	// D3D9 varsayılan durumları
	auto& rs                        = d.rs;
	rs.fill(0);
	rs[D3DRS_ZENABLE]               = pp.EnableAutoDepthStencil ? D3DZB_TRUE : D3DZB_FALSE;
	rs[D3DRS_FILLMODE]              = D3DFILL_SOLID;
	rs[D3DRS_SHADEMODE]             = D3DSHADE_GOURAUD;
	rs[D3DRS_ZWRITEENABLE]          = TRUE;
	rs[D3DRS_ALPHATESTENABLE]       = FALSE;
	rs[D3DRS_LASTPIXEL]             = TRUE;
	rs[D3DRS_SRCBLEND]              = D3DBLEND_ONE;
	rs[D3DRS_DESTBLEND]             = D3DBLEND_ZERO;
	rs[D3DRS_CULLMODE]              = D3DCULL_CCW;
	rs[D3DRS_ZFUNC]                 = D3DCMP_LESSEQUAL;
	rs[D3DRS_ALPHAREF]              = 0;
	rs[D3DRS_ALPHAFUNC]             = D3DCMP_ALWAYS;
	rs[D3DRS_DITHERENABLE]          = FALSE;
	rs[D3DRS_ALPHABLENDENABLE]      = FALSE;
	rs[D3DRS_FOGENABLE]             = FALSE;
	rs[D3DRS_SPECULARENABLE]        = FALSE;
	rs[D3DRS_FOGCOLOR]              = 0;
	rs[D3DRS_FOGTABLEMODE]          = D3DFOG_NONE;
	rs[D3DRS_FOGSTART]              = std::bit_cast<DWORD>(0.0f);
	rs[D3DRS_FOGEND]                = std::bit_cast<DWORD>(1.0f);
	rs[D3DRS_FOGDENSITY]            = std::bit_cast<DWORD>(1.0f);
	rs[D3DRS_RANGEFOGENABLE]        = FALSE;
	rs[D3DRS_STENCILENABLE]         = FALSE;
	rs[D3DRS_STENCILFAIL]           = D3DSTENCILOP_KEEP;
	rs[D3DRS_STENCILZFAIL]          = D3DSTENCILOP_KEEP;
	rs[D3DRS_STENCILPASS]           = D3DSTENCILOP_KEEP;
	rs[D3DRS_STENCILFUNC]           = D3DCMP_ALWAYS;
	rs[D3DRS_STENCILREF]            = 0;
	rs[D3DRS_STENCILMASK]           = 0xFFFFFFFF;
	rs[D3DRS_STENCILWRITEMASK]      = 0xFFFFFFFF;
	rs[D3DRS_TEXTUREFACTOR]         = 0xFFFFFFFF;
	rs[D3DRS_CLIPPING]              = TRUE;
	rs[D3DRS_LIGHTING]              = TRUE;
	rs[D3DRS_AMBIENT]               = 0;
	rs[D3DRS_FOGVERTEXMODE]         = D3DFOG_NONE;
	rs[D3DRS_COLORVERTEX]           = TRUE;
	rs[D3DRS_LOCALVIEWER]           = TRUE;
	rs[D3DRS_NORMALIZENORMALS]      = FALSE;
	rs[D3DRS_DIFFUSEMATERIALSOURCE] = D3DMCS_COLOR1;
	rs[D3DRS_SPECULARMATERIALSOURCE] = D3DMCS_COLOR2;
	rs[D3DRS_AMBIENTMATERIALSOURCE] = D3DMCS_MATERIAL;
	rs[D3DRS_EMISSIVEMATERIALSOURCE] = D3DMCS_MATERIAL;
	rs[D3DRS_VERTEXBLEND]           = D3DVBF_DISABLE;
	rs[D3DRS_POINTSIZE]             = std::bit_cast<DWORD>(1.0f);
	rs[D3DRS_POINTSIZE_MIN]         = std::bit_cast<DWORD>(1.0f);
	rs[D3DRS_POINTSIZE_MAX]         = std::bit_cast<DWORD>(64.0f);
	rs[D3DRS_COLORWRITEENABLE]      = 0xF;
	rs[D3DRS_BLENDOP]               = D3DBLENDOP_ADD;
	rs[D3DRS_SCISSORTESTENABLE]     = FALSE;
	rs[D3DRS_BLENDFACTOR]           = 0xFFFFFFFF;

	for (int i = 0; i < MAX_STAGES; ++i)
	{
		d.stages[i]               = StageState {};
		d.stages[i].texCoordIndex = (DWORD) i;
		d.samplers[i]             = SamplerState {};
	}
	d.stages[0].colorOp   = D3DTOP_MODULATE;
	d.stages[0].colorArg1 = D3DTA_TEXTURE;
	d.stages[0].colorArg2 = D3DTA_CURRENT;
	d.stages[0].alphaOp   = D3DTOP_SELECTARG1;
	d.stages[0].alphaArg1 = D3DTA_TEXTURE;
	d.stages[0].alphaArg2 = D3DTA_CURRENT;

	MatIdentity(d.world);
	MatIdentity(d.view);
	MatIdentity(d.proj);
	for (auto& m : d.texMatrix)
		MatIdentity(m);

	// Varsayılan malzeme: D3D'de hepsi sıfırdır; ancak oyun kodu malzeme ayarlamadan çizim
	// yaparsa her şey siyah görünür. Beyaz difüz/ortam daha güvenli bir başlangıçtır.
	d.material          = {};
	d.material.Diffuse  = {1, 1, 1, 1};
	d.material.Ambient  = {1, 1, 1, 1};

	d.viewport = {0, 0, d.pp.BackBufferWidth, d.pp.BackBufferHeight, 0.0f, 1.0f};
	SetRect(&d.scissor, 0, 0, (int) d.pp.BackBufferWidth, (int) d.pp.BackBufferHeight);
	d.clipStatus = {D3DCS_ALL, D3DCS_ALL};

	d.InitGL();
}

IDirect3DDevice9::~IDirect3DDevice9()
{
	for (auto*& t : m_impl->textures)
		if (t)
		{
			t->Release();
			t = nullptr;
		}
	if (m_impl->streamVB) m_impl->streamVB->Release();
	if (m_impl->indices) m_impl->indices->Release();
	m_impl->ShutdownGL();
	if (m_parent)
		m_parent->Release();
}

bool IDirect3DDevice9::SupportsS3TC() const
{
	return m_impl->s3tc;
}

HRESULT IDirect3DDevice9::TestCooperativeLevel()
{
	return D3D_OK;
}

UINT IDirect3DDevice9::GetAvailableTextureMem()
{
	return 512 * 1024 * 1024;
}

HRESULT IDirect3DDevice9::EvictManagedResources()
{
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetDirect3D(IDirect3D9** out)
{
	*out = m_parent;
	m_parent->AddRef();
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetDeviceCaps(D3DCAPS9* caps)
{
	*caps = m_impl->caps;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetDisplayMode(UINT, D3DDISPLAYMODE* mode)
{
	return m_parent->GetAdapterDisplayMode(0, mode);
}

HRESULT IDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* p)
{
	*p              = {};
	p->DeviceType   = D3DDEVTYPE_HAL;
	p->hFocusWindow = m_impl->pp.hDeviceWindow;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pp)
{
	if (pp)
		m_impl->pp = *pp;
	int w, h;
	m_impl->GetDrawableSize(&w, &h);
	if (m_impl->pp.BackBufferWidth == 0) m_impl->pp.BackBufferWidth = (UINT) w;
	if (m_impl->pp.BackBufferHeight == 0) m_impl->pp.BackBufferHeight = (UINT) h;
	m_impl->viewport = {0, 0, m_impl->pp.BackBufferWidth, m_impl->pp.BackBufferHeight, 0.0f, 1.0f};
	return D3D_OK;
}

HRESULT IDirect3DDevice9::Present(const RECT*, const RECT*, HWND, const RGNDATA*)
{
	if (g_hooks.present)
		g_hooks.present(g_hooks.user);
	m_impl->drawCalls = 0;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetBackBuffer(UINT, UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9** out)
{
	*out = nullptr;
	return D3DERR_NOTAVAILABLE;
}

HRESULT IDirect3DDevice9::CreateTexture(UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DTexture9** out, HANDLE*)
{
	if (!out || w == 0 || h == 0)
		return D3DERR_INVALIDCALL;
	if (IsCompressedFormat(fmt) && (w % 4 || h % 4) && (w >= 4 || h >= 4))
	{
		// D3D de DXT için 4'ün katı olmayan boyutları reddeder; blok sayısına yuvarlıyoruz.
	}
	*out = new IDirect3DTexture9(this, w, h, levels, usage, fmt, pool);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer9** out, HANDLE*)
{
	if (!out || length == 0)
		return D3DERR_INVALIDCALL;
	*out = new IDirect3DVertexBuffer9(this, length, usage, fvf, pool);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DIndexBuffer9** out, HANDLE*)
{
	if (!out || length == 0)
		return D3DERR_INVALIDCALL;
	*out = new IDirect3DIndexBuffer9(this, length, usage, fmt, pool);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateOffscreenPlainSurface(UINT w, UINT h, D3DFORMAT fmt, D3DPOOL, IDirect3DSurface9** out, HANDLE*)
{
	if (!out || w == 0 || h == 0)
		return D3DERR_INVALIDCALL;
	*out = new IDirect3DSurface9(this, nullptr, 0, w, h, fmt);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::UpdateSurface(IDirect3DSurface9* src, const RECT* srcRect, IDirect3DSurface9* dst, const POINT* dstPoint)
{
	if (!src || !dst)
		return D3DERR_INVALIDCALL;
	D3DSURFACE_DESC sd, dd;
	src->GetDesc(&sd);
	dst->GetDesc(&dd);
	if (sd.Format != dd.Format || IsCompressedFormat(sd.Format))
		return D3DERR_INVALIDCALL;
	UINT bpp = FormatBytesPerPixel(sd.Format);
	RECT sr  = srcRect ? *srcRect : RECT {0, 0, (LONG) sd.Width, (LONG) sd.Height};
	POINT dp = dstPoint ? *dstPoint : POINT {0, 0};
	D3DLOCKED_RECT ls, ld;
	src->LockRect(&ls, nullptr, D3DLOCK_READONLY);
	dst->LockRect(&ld, nullptr, 0);
	for (LONG y = sr.top; y < sr.bottom; ++y)
	{
		LONG dy = dp.y + (y - sr.top);
		if (dy < 0 || dy >= (LONG) dd.Height)
			continue;
		std::memcpy(static_cast<uint8_t*>(ld.pBits) + dy * ld.Pitch + dp.x * bpp, static_cast<uint8_t*>(ls.pBits) + y * ls.Pitch + sr.left * bpp, (size_t) (sr.right - sr.left) * bpp);
	}
	dst->UnlockRect();
	src->UnlockRect();
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetFrontBufferData(UINT, IDirect3DSurface9* dst)
{
	if (!dst)
		return D3DERR_INVALIDCALL;
	D3DSURFACE_DESC dd;
	dst->GetDesc(&dd);
	int w, h;
	m_impl->GetDrawableSize(&w, &h);
	std::vector<uint8_t> rgba((size_t) w * h * 4);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	// GL alttan yukarı okur; D3D üstten aşağı bekler
	std::vector<uint8_t> flipped(rgba.size());
	for (int y = 0; y < h; ++y)
		std::memcpy(flipped.data() + (size_t) y * w * 4, rgba.data() + (size_t) (h - 1 - y) * w * 4, (size_t) w * 4);
	D3DLOCKED_RECT lr;
	dst->LockRect(&lr, nullptr, 0);
	UINT cw = std::min<UINT>((UINT) w, dd.Width), ch = std::min<UINT>((UINT) h, dd.Height);
	std::vector<uint8_t> row(cw * FormatBytesPerPixel(dd.Format));
	for (UINT y = 0; y < ch; ++y)
	{
		RGBA8ToPixels(dd.Format, cw, 1, flipped.data() + (size_t) y * w * 4, row.data());
		std::memcpy(static_cast<uint8_t*>(lr.pBits) + y * lr.Pitch, row.data(), row.size());
	}
	dst->UnlockRect();
	return D3D_OK;
}

HRESULT IDirect3DDevice9::ColorFill(IDirect3DSurface9* surf, const RECT* rect, D3DCOLOR color)
{
	if (!surf)
		return D3DERR_INVALIDCALL;
	D3DSURFACE_DESC sd;
	surf->GetDesc(&sd);
	if (IsCompressedFormat(sd.Format))
		return D3DERR_INVALIDCALL;
	uint8_t rgba[4] = {(uint8_t) ((color >> 16) & 0xFF), (uint8_t) ((color >> 8) & 0xFF), (uint8_t) (color & 0xFF), (uint8_t) ((color >> 24) & 0xFF)};
	uint8_t px[4];
	RGBA8ToPixels(sd.Format, 1, 1, rgba, px);
	UINT bpp = FormatBytesPerPixel(sd.Format);
	RECT r   = rect ? *rect : RECT {0, 0, (LONG) sd.Width, (LONG) sd.Height};
	D3DLOCKED_RECT lr;
	surf->LockRect(&lr, nullptr, 0);
	for (LONG y = r.top; y < r.bottom; ++y)
		for (LONG x = r.left; x < r.right; ++x)
			std::memcpy(static_cast<uint8_t*>(lr.pBits) + y * lr.Pitch + x * bpp, px, bpp);
	surf->UnlockRect();
	return D3D_OK;
}

HRESULT IDirect3DDevice9::BeginScene()
{
	m_impl->inScene = true;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::EndScene()
{
	m_impl->inScene = false;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::Clear(DWORD count, const D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	DeviceImpl& d = *m_impl;
	int dw, dh;
	d.GetDrawableSize(&dw, &dh);
	GLbitfield mask = 0;
	if (flags & D3DCLEAR_TARGET)
	{
		float c[4];
		ColorToFloat4(color, c);
		glClearColor(c[0], c[1], c[2], c[3]);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		mask |= GL_COLOR_BUFFER_BIT;
	}
	if (flags & D3DCLEAR_ZBUFFER)
	{
		glClearDepthf(z);
		glDepthMask(GL_TRUE);
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (flags & D3DCLEAR_STENCIL)
	{
		glClearStencil((GLint) stencil);
		glStencilMask(0xFFFFFFFF);
		mask |= GL_STENCIL_BUFFER_BIT;
	}
	if (!mask)
		return D3D_OK;

	// D3D Clear yalnızca görünüm alanını (ve verilen dikdörtgenleri) temizler.
	glEnable(GL_SCISSOR_TEST);
	RECT vp = {(LONG) d.viewport.X, (LONG) d.viewport.Y, (LONG) (d.viewport.X + d.viewport.Width), (LONG) (d.viewport.Y + d.viewport.Height)};
	if (d.rs[D3DRS_SCISSORTESTENABLE])
		IntersectRect(&vp, &vp, &d.scissor);
	auto clearRect = [&](const RECT& r) {
		int w = std::max(0, (int) (r.right - r.left)), h = std::max(0, (int) (r.bottom - r.top));
		glScissor(r.left, dh - r.bottom, w, h);
		glClear(mask);
	};
	if (count == 0 || rects == nullptr)
		clearRect(vp);
	else
		for (DWORD i = 0; i < count; ++i)
		{
			RECT r = {rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2}, c;
			if (IntersectRect(&c, &r, &vp))
				clearRect(c);
		}
	glDisable(GL_SCISSOR_TEST);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* m)
{
	if (!m)
		return D3DERR_INVALIDCALL;
	DeviceImpl& d = *m_impl;
	if (state == D3DTS_VIEW) d.view = *m;
	else if (state == D3DTS_PROJECTION) d.proj = *m;
	else if ((int) state >= 256) { if (state == D3DTS_WORLD) d.world = *m; }
	else if (state >= D3DTS_TEXTURE0 && state <= D3DTS_TEXTURE7) d.texMatrix[state - D3DTS_TEXTURE0] = *m;
	else return D3DERR_INVALIDCALL;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE state, D3DMATRIX* m)
{
	DeviceImpl& d = *m_impl;
	if (state == D3DTS_VIEW) *m = d.view;
	else if (state == D3DTS_PROJECTION) *m = d.proj;
	else if (state == D3DTS_WORLD) *m = d.world;
	else if (state >= D3DTS_TEXTURE0 && state <= D3DTS_TEXTURE7) *m = d.texMatrix[state - D3DTS_TEXTURE0];
	else return D3DERR_INVALIDCALL;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* m)
{
	D3DMATRIX cur;
	if (FAILED(GetTransform(state, &cur)))
		return D3DERR_INVALIDCALL;
	D3DMATRIX r;
	MatMul(cur, *m, r);
	return SetTransform(state, &r);
}

HRESULT IDirect3DDevice9::SetViewport(const D3DVIEWPORT9* vp)
{
	if (!vp)
		return D3DERR_INVALIDCALL;
	m_impl->viewport = *vp;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetViewport(D3DVIEWPORT9* vp)
{
	*vp = m_impl->viewport;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetMaterial(const D3DMATERIAL9* m)
{
	if (!m)
		return D3DERR_INVALIDCALL;
	m_impl->material = *m;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetMaterial(D3DMATERIAL9* m)
{
	*m = m_impl->material;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetLight(DWORD index, const D3DLIGHT9* l)
{
	if (index >= MAX_LIGHTS || !l)
		return D3DERR_INVALIDCALL;
	m_impl->lights[index].light = *l;
	m_impl->lights[index].set   = true;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetLight(DWORD index, D3DLIGHT9* l)
{
	if (index >= MAX_LIGHTS)
		return D3DERR_INVALIDCALL;
	*l = m_impl->lights[index].light;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::LightEnable(DWORD index, BOOL enable)
{
	if (index >= MAX_LIGHTS)
		return D3DERR_INVALIDCALL;
	auto& ls = m_impl->lights[index];
	if (!ls.set)
	{
		// D3D: ayarlanmamış ışık etkinleştirilirse varsayılan yönlü beyaz ışık oluşur
		ls.light           = {};
		ls.light.Type      = D3DLIGHT_DIRECTIONAL;
		ls.light.Diffuse   = {1, 1, 1, 0};
		ls.light.Direction = {0, 0, 1};
		ls.set             = true;
	}
	ls.enabled = enable != FALSE;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetLightEnable(DWORD index, BOOL* enable)
{
	if (index >= MAX_LIGHTS)
		return D3DERR_INVALIDCALL;
	*enable = m_impl->lights[index].enabled;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	if ((DWORD) state >= D3DRS_MAXSTATE)
		return D3DERR_INVALIDCALL;
	m_impl->rs[state] = value;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE state, DWORD* value)
{
	if ((DWORD) state >= D3DRS_MAXSTATE || !value)
		return D3DERR_INVALIDCALL;
	*value = m_impl->rs[state];
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetClipStatus(const D3DCLIPSTATUS9* cs)
{
	if (cs)
		m_impl->clipStatus = *cs;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* cs)
{
	*cs = m_impl->clipStatus;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetTexture(DWORD stage, IDirect3DBaseTexture9** out)
{
	if (stage >= MAX_STAGES)
		return D3DERR_INVALIDCALL;
	*out = m_impl->textures[stage];
	if (*out)
		(*out)->AddRef();
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetTexture(DWORD stage, IDirect3DBaseTexture9* tex)
{
	if (stage >= MAX_STAGES)
		return D3DERR_INVALIDCALL;
	auto& slot = m_impl->textures[stage];
	if (slot == tex)
		return D3D_OK;
	if (tex)
		tex->AddRef();
	if (slot)
		slot->Release();
	slot = tex;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
	if (stage >= MAX_STAGES)
		return D3DERR_INVALIDCALL;
	StageState& st = m_impl->stages[stage];
	switch (type)
	{
		case D3DTSS_COLOROP: st.colorOp = value; break;
		case D3DTSS_COLORARG1: st.colorArg1 = value; break;
		case D3DTSS_COLORARG2: st.colorArg2 = value; break;
		case D3DTSS_ALPHAOP: st.alphaOp = value; break;
		case D3DTSS_ALPHAARG1: st.alphaArg1 = value; break;
		case D3DTSS_ALPHAARG2: st.alphaArg2 = value; break;
		case D3DTSS_TEXCOORDINDEX: st.texCoordIndex = value; break;
		case D3DTSS_RESULTARG: st.resultArg = value; break;
		case D3DTSS_TEXTURETRANSFORMFLAGS: st.textureTransformFlags = value; break;
		case D3DTSS_BUMPENVMAT00: case D3DTSS_BUMPENVMAT01: case D3DTSS_BUMPENVMAT10: case D3DTSS_BUMPENVMAT11:
			st.bumpEnv[type - D3DTSS_BUMPENVMAT00] = value; break;
		// D3D8 kalıntısı örnekleyici durumları
		case D3DTSS_ADDRESSU: return SetSamplerState(stage, D3DSAMP_ADDRESSU, value);
		case D3DTSS_ADDRESSV: return SetSamplerState(stage, D3DSAMP_ADDRESSV, value);
		case D3DTSS_ADDRESSW: return SetSamplerState(stage, D3DSAMP_ADDRESSW, value);
		case D3DTSS_BORDERCOLOR: return SetSamplerState(stage, D3DSAMP_BORDERCOLOR, value);
		case D3DTSS_MAGFILTER: return SetSamplerState(stage, D3DSAMP_MAGFILTER, value);
		case D3DTSS_MINFILTER: return SetSamplerState(stage, D3DSAMP_MINFILTER, value);
		case D3DTSS_MIPFILTER: return SetSamplerState(stage, D3DSAMP_MIPFILTER, value);
		case D3DTSS_MIPMAPLODBIAS: return SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, value);
		case D3DTSS_MAXMIPLEVEL: return SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, value);
		case D3DTSS_MAXANISOTROPY: return SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, value);
		default: break;
	}
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD* value)
{
	if (stage >= MAX_STAGES || !value)
		return D3DERR_INVALIDCALL;
	const StageState& st = m_impl->stages[stage];
	switch (type)
	{
		case D3DTSS_COLOROP: *value = st.colorOp; break;
		case D3DTSS_COLORARG1: *value = st.colorArg1; break;
		case D3DTSS_COLORARG2: *value = st.colorArg2; break;
		case D3DTSS_ALPHAOP: *value = st.alphaOp; break;
		case D3DTSS_ALPHAARG1: *value = st.alphaArg1; break;
		case D3DTSS_ALPHAARG2: *value = st.alphaArg2; break;
		case D3DTSS_TEXCOORDINDEX: *value = st.texCoordIndex; break;
		case D3DTSS_RESULTARG: *value = st.resultArg; break;
		case D3DTSS_TEXTURETRANSFORMFLAGS: *value = st.textureTransformFlags; break;
		case D3DTSS_ADDRESSU: return GetSamplerState(stage, D3DSAMP_ADDRESSU, value);
		case D3DTSS_ADDRESSV: return GetSamplerState(stage, D3DSAMP_ADDRESSV, value);
		case D3DTSS_ADDRESSW: return GetSamplerState(stage, D3DSAMP_ADDRESSW, value);
		case D3DTSS_BORDERCOLOR: return GetSamplerState(stage, D3DSAMP_BORDERCOLOR, value);
		case D3DTSS_MAGFILTER: return GetSamplerState(stage, D3DSAMP_MAGFILTER, value);
		case D3DTSS_MINFILTER: return GetSamplerState(stage, D3DSAMP_MINFILTER, value);
		case D3DTSS_MIPFILTER: return GetSamplerState(stage, D3DSAMP_MIPFILTER, value);
		case D3DTSS_MIPMAPLODBIAS: return GetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, value);
		case D3DTSS_MAXMIPLEVEL: return GetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, value);
		case D3DTSS_MAXANISOTROPY: return GetSamplerState(stage, D3DSAMP_MAXANISOTROPY, value);
		default: *value = 0; break;
	}
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
	if (sampler >= MAX_STAGES)
		return D3DERR_INVALIDCALL;
	SamplerState& ss = m_impl->samplers[sampler];
	switch (type)
	{
		case D3DSAMP_ADDRESSU: ss.addressU = value; break;
		case D3DSAMP_ADDRESSV: ss.addressV = value; break;
		case D3DSAMP_ADDRESSW: ss.addressW = value; break;
		case D3DSAMP_BORDERCOLOR: ss.borderColor = value; break;
		case D3DSAMP_MAGFILTER: ss.magFilter = value; break;
		case D3DSAMP_MINFILTER: ss.minFilter = value; break;
		case D3DSAMP_MIPFILTER: ss.mipFilter = value; break;
		case D3DSAMP_MIPMAPLODBIAS: ss.lodBias = value; break;
		case D3DSAMP_MAXMIPLEVEL: ss.maxMipLevel = value; break;
		case D3DSAMP_MAXANISOTROPY: ss.maxAnisotropy = value; break;
		default: break;
	}
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD* value)
{
	if (sampler >= MAX_STAGES || !value)
		return D3DERR_INVALIDCALL;
	const SamplerState& ss = m_impl->samplers[sampler];
	switch (type)
	{
		case D3DSAMP_ADDRESSU: *value = ss.addressU; break;
		case D3DSAMP_ADDRESSV: *value = ss.addressV; break;
		case D3DSAMP_ADDRESSW: *value = ss.addressW; break;
		case D3DSAMP_BORDERCOLOR: *value = ss.borderColor; break;
		case D3DSAMP_MAGFILTER: *value = ss.magFilter; break;
		case D3DSAMP_MINFILTER: *value = ss.minFilter; break;
		case D3DSAMP_MIPFILTER: *value = ss.mipFilter; break;
		case D3DSAMP_MIPMAPLODBIAS: *value = ss.lodBias; break;
		case D3DSAMP_MAXMIPLEVEL: *value = ss.maxMipLevel; break;
		case D3DSAMP_MAXANISOTROPY: *value = ss.maxAnisotropy; break;
		default: *value = 0; break;
	}
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetScissorRect(const RECT* rect)
{
	if (!rect)
		return D3DERR_INVALIDCALL;
	m_impl->scissor = *rect;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetScissorRect(RECT* rect)
{
	*rect = m_impl->scissor;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetFVF(DWORD fvf)
{
	m_impl->fvf = fvf;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetFVF(DWORD* fvf)
{
	*fvf = m_impl->fvf;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetStreamSource(UINT stream, IDirect3DVertexBuffer9* vb, UINT offset, UINT stride)
{
	if (stream != 0)
		return D3DERR_INVALIDCALL;
	if (vb)
		vb->AddRef();
	if (m_impl->streamVB)
		m_impl->streamVB->Release();
	m_impl->streamVB     = vb;
	m_impl->streamOffset = offset;
	m_impl->streamStride = stride;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetStreamSource(UINT stream, IDirect3DVertexBuffer9** vb, UINT* offset, UINT* stride)
{
	if (stream != 0)
		return D3DERR_INVALIDCALL;
	*vb = m_impl->streamVB;
	if (*vb)
		(*vb)->AddRef();
	if (offset) *offset = m_impl->streamOffset;
	if (stride) *stride = m_impl->streamStride;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* ib)
{
	if (ib)
		ib->AddRef();
	if (m_impl->indices)
		m_impl->indices->Release();
	m_impl->indices = ib;
	return D3D_OK;
}

HRESULT IDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ib)
{
	*ib = m_impl->indices;
	if (*ib)
		(*ib)->AddRef();
	return D3D_OK;
}

HRESULT IDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primCount)
{
	DeviceImpl& d = *m_impl;
	if (!d.streamVB)
		return D3DERR_INVALIDCALL;
	FvfLayout layout = ComputeFvfLayout(d.fvf ? d.fvf : d.streamVB->FVF());
	UINT stride      = d.streamStride ? d.streamStride : layout.stride;
	layout.stride    = stride;
	GLuint vbo       = d.streamVB->GLName();
	d.DrawInternal(type, primCount, vbo, (size_t) d.streamOffset + (size_t) startVertex * stride, layout, 0, 0, 0, false);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT, UINT, UINT startIndex, UINT primCount)
{
	DeviceImpl& d = *m_impl;
	if (!d.streamVB || !d.indices)
		return D3DERR_INVALIDCALL;
	FvfLayout layout = ComputeFvfLayout(d.fvf ? d.fvf : d.streamVB->FVF());
	UINT stride      = d.streamStride ? d.streamStride : layout.stride;
	layout.stride    = stride;
	GLuint vbo       = d.streamVB->GLName();
	GLuint ibo       = d.indices->GLName();
	bool idx32       = d.indices->Format() == D3DFMT_INDEX32;
	d.DrawInternal(type, primCount, vbo, (size_t) d.streamOffset + (size_t) baseVertexIndex * stride, layout, ibo,
		idx32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (size_t) startIndex * (idx32 ? 4 : 2), true);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT primCount, const void* vertexData, UINT stride)
{
	DeviceImpl& d = *m_impl;
	if (!vertexData || stride == 0)
		return D3DERR_INVALIDCALL;
	UINT vertexCount = 0;
	PrimitiveToGL(type, primCount, &vertexCount);
	if (vertexCount == 0)
		return D3D_OK;
	FvfLayout layout = ComputeFvfLayout(d.fvf);
	layout.stride    = stride;
	glBindBuffer(GL_ARRAY_BUFFER, d.streamVbo);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) ((size_t) vertexCount * stride), vertexData, GL_STREAM_DRAW);
	d.DrawInternal(type, primCount, d.streamVbo, 0, layout, 0, 0, 0, false);
	return D3D_OK;
}

HRESULT IDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type, UINT minIndex, UINT numVertices, UINT primCount, const void* indexData, D3DFORMAT indexFormat, const void* vertexData, UINT stride)
{
	DeviceImpl& d = *m_impl;
	if (!vertexData || !indexData || stride == 0)
		return D3DERR_INVALIDCALL;
	UINT indexCount = 0;
	PrimitiveToGL(type, primCount, &indexCount);
	if (indexCount == 0)
		return D3D_OK;
	FvfLayout layout = ComputeFvfLayout(d.fvf);
	layout.stride    = stride;
	bool idx32       = indexFormat == D3DFMT_INDEX32;
	glBindBuffer(GL_ARRAY_BUFFER, d.streamVbo);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) ((size_t) (minIndex + numVertices) * stride), vertexData, GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d.streamIbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) ((size_t) indexCount * (idx32 ? 4 : 2)), indexData, GL_STREAM_DRAW);
	d.DrawInternal(type, primCount, d.streamVbo, 0, layout, d.streamIbo, idx32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, true);
	return D3D_OK;
}
