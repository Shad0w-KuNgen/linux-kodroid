// Shaders.cpp — Direct3D 9 sabit işlev boru hattının GLSL ES 3.00 üretimi.
//
// D3D9 sabit işlev: köşe başına aydınlatma (Gouraud), 8 ışık, sis (vertex/table),
// doku aşamaları (D3DTOP_*), alfa testi. Her farklı durum kombinasyonu için bir
// program üretilir ve önbelleğe alınır.
#include "Internal.h"

#include <sstream>

namespace d3d9gles
{
namespace
{
const char* ArgExpr(uint8_t arg, bool alpha)
{
	// Temel kaynak
	const char* base = "cur";
	switch (arg & D3DTA_SELECTMASK)
	{
		case D3DTA_DIFFUSE:  base = "vDiffuse"; break;
		case D3DTA_CURRENT:  base = "cur"; break;
		case D3DTA_TEXTURE:  base = "tex"; break;
		case D3DTA_TFACTOR:  base = "uTFactor"; break;
		case D3DTA_SPECULAR: base = "vSpecular"; break;
		case D3DTA_TEMP:     base = "tmp"; break;
		case D3DTA_CONSTANT: base = "uTFactor"; break;
		default: break;
	}
	static thread_local std::string buf;
	buf = base;
	if (arg & D3DTA_ALPHAREPLICATE)
		buf = "vec4(" + buf + ".a)";
	if (arg & D3DTA_COMPLEMENT)
		buf = "(vec4(1.0) - " + buf + ")";
	if (alpha)
		buf += ".a";
	else
		buf += ".rgb";
	return buf.c_str();
}

std::string OpExpr(uint8_t op, const std::string& a1, const std::string& a2, bool alpha, const std::string& texAlpha, const std::string& diffAlpha, const std::string& curAlpha)
{
	auto v = [&](const char* s) { return alpha ? std::string(s) : std::string("vec3(") + s + ")"; };
	switch (op)
	{
		case D3DTOP_SELECTARG1: return a1;
		case D3DTOP_SELECTARG2: return a2;
		case D3DTOP_MODULATE:   return "(" + a1 + " * " + a2 + ")";
		case D3DTOP_MODULATE2X: return "(" + a1 + " * " + a2 + " * 2.0)";
		case D3DTOP_MODULATE4X: return "(" + a1 + " * " + a2 + " * 4.0)";
		case D3DTOP_ADD:        return "(" + a1 + " + " + a2 + ")";
		case D3DTOP_ADDSIGNED:  return "(" + a1 + " + " + a2 + " - " + v("0.5") + ")";
		case D3DTOP_ADDSIGNED2X: return "((" + a1 + " + " + a2 + " - " + v("0.5") + ") * 2.0)";
		case D3DTOP_SUBTRACT:   return "(" + a1 + " - " + a2 + ")";
		case D3DTOP_ADDSMOOTH:  return "(" + a1 + " + " + a2 + " - " + a1 + " * " + a2 + ")";
		case D3DTOP_BLENDDIFFUSEALPHA: return "mix(" + a2 + ", " + a1 + ", " + diffAlpha + ")";
		case D3DTOP_BLENDTEXTUREALPHA: return "mix(" + a2 + ", " + a1 + ", " + texAlpha + ")";
		case D3DTOP_BLENDFACTORALPHA:  return "mix(" + a2 + ", " + a1 + ", uTFactor.a)";
		case D3DTOP_BLENDCURRENTALPHA: return "mix(" + a2 + ", " + a1 + ", " + curAlpha + ")";
		case D3DTOP_BLENDTEXTUREALPHAPM: return "(" + a1 + " + " + a2 + " * (1.0 - " + texAlpha + "))";
		case D3DTOP_DOTPRODUCT3:
			if (alpha) return "dot(vec3(" + a1 + ") - 0.5, vec3(" + a2 + ") - 0.5) * 4.0";
			return "vec3(dot(" + a1 + " - 0.5, " + a2 + " - 0.5) * 4.0)";
		case D3DTOP_LERP:       return "mix(" + a2 + ", " + a1 + ", " + diffAlpha + ")";
		case D3DTOP_MULTIPLYADD: return "(" + a1 + " * " + a2 + " + " + (alpha ? std::string("cur.a") : std::string("cur.rgb")) + ")";
		default: return a1;
	}
}
} // namespace

std::string BuildVertexShader(const ProgramKey& key)
{
	std::ostringstream s;
	s << "#version 300 es\n"
		 "precision highp float;\n"
		 "layout(location=0) in vec4 aPosition;\n";
	if (key.hasNormal)   s << "layout(location=1) in vec3 aNormal;\n";
	if (key.hasDiffuse)  s << "layout(location=2) in vec4 aDiffuse;\n";
	if (key.hasSpecular) s << "layout(location=3) in vec4 aSpecular;\n";
	if (key.hasPSize)    s << "layout(location=4) in float aPSize;\n";
	for (int i = 0; i < key.texCount; ++i)
		s << "layout(location=" << (5 + i) << ") in vec2 aTex" << i << ";\n";

	s << "uniform mat4 uWorld, uView, uWVP;\n"
		 "uniform vec2 uViewportSize;\n"
		 "uniform vec3 uEyePos;\n"
		 "uniform vec4 uGlobalAmbient, uMatDiffuse, uMatAmbient, uMatSpecular, uMatEmissive;\n"
		 "uniform float uMatPower;\n"
		 "uniform int uLightCount;\n"
		 "uniform int uLightType[8];\n"
		 "uniform vec4 uLightDiffuse[8], uLightSpecular[8], uLightAmbient[8];\n"
		 "uniform vec3 uLightPos[8], uLightDir[8];\n"
		 "uniform vec4 uLightAtt[8];\n"   // range, a0, a1, a2
		 "uniform vec3 uLightSpot[8];\n"  // cosHalfTheta, cosHalfPhi, falloff
		 "uniform vec3 uFogParams;\n"     // start, end, density
		 "uniform float uPointSize;\n"
		 "out vec4 vDiffuse;\n"
		 "out vec4 vSpecular;\n"
		 "out float vFog;\n";
	for (int i = 0; i < key.texCount; ++i)
		s << "out vec2 vTex" << i << ";\n";

	s << "void main() {\n";
	if (key.rhw)
	{
		// Önceden dönüştürülmüş köşe: ekran koordinatı (piksel), z [0,1], rhw.
		// D3D piksel merkezi tam sayı; GL'de +0.5. x,y'yi NDC'ye çevir, y'yi ters çevir.
		s << "  float px = aPosition.x + 0.5;\n"
			 "  float py = aPosition.y + 0.5;\n"
			 "  float nx = px / uViewportSize.x * 2.0 - 1.0;\n"
			 "  float ny = 1.0 - py / uViewportSize.y * 2.0;\n"
			 "  gl_Position = vec4(nx, ny, aPosition.z * 2.0 - 1.0, 1.0);\n"
			 "  vec3 worldPos = aPosition.xyz;\n";
	}
	else
	{
		s << "  vec4 pos = vec4(aPosition.xyz, 1.0);\n"
			 "  vec4 clip = uWVP * pos;\n"
			 "  clip.z = clip.z * 2.0 - clip.w;\n" // D3D z∈[0,w] → GL z∈[-w,w]
			 "  gl_Position = clip;\n"
			 "  vec3 worldPos = (uWorld * pos).xyz;\n";
	}

	// Renk kaynakları
	s << "  vec4 vtxDiffuse = " << (key.hasDiffuse ? "aDiffuse.bgra" : "vec4(1.0)") << ";\n";
	s << "  vec4 vtxSpecular = " << (key.hasSpecular ? "aSpecular.bgra" : "vec4(0.0)") << ";\n";

	if (key.lighting && !key.rhw)
	{
		const char* matDiffuse  = (key.diffuseSrc == D3DMCS_COLOR1 && key.hasDiffuse && key.colorVertex) ? "vtxDiffuse" : (key.diffuseSrc == D3DMCS_COLOR2 && key.hasSpecular && key.colorVertex) ? "vtxSpecular" : "uMatDiffuse";
		const char* matAmbient  = (key.ambientSrc == D3DMCS_COLOR1 && key.hasDiffuse && key.colorVertex) ? "vtxDiffuse" : (key.ambientSrc == D3DMCS_COLOR2 && key.hasSpecular && key.colorVertex) ? "vtxSpecular" : "uMatAmbient";
		const char* matSpecular = (key.specularSrc == D3DMCS_COLOR1 && key.hasDiffuse && key.colorVertex) ? "vtxDiffuse" : (key.specularSrc == D3DMCS_COLOR2 && key.hasSpecular && key.colorVertex) ? "vtxSpecular" : "uMatSpecular";
		const char* matEmissive = (key.emissiveSrc == D3DMCS_COLOR1 && key.hasDiffuse && key.colorVertex) ? "vtxDiffuse" : (key.emissiveSrc == D3DMCS_COLOR2 && key.hasSpecular && key.colorVertex) ? "vtxSpecular" : "uMatEmissive";
		s << "  vec4 mDiff = " << matDiffuse << ";\n"
			 "  vec4 mAmb = " << matAmbient << ";\n"
			 "  vec4 mSpec = " << matSpecular << ";\n"
			 "  vec4 mEmis = " << matEmissive << ";\n";
		if (key.hasNormal)
			s << "  vec3 N = normalize(mat3(uWorld) * aNormal);\n";
		else
			s << "  vec3 N = vec3(0.0, 0.0, 0.0);\n";
		s << "  vec3 V = normalize(uEyePos - worldPos);\n"
			 "  vec3 ambAcc = uGlobalAmbient.rgb * mAmb.rgb;\n"
			 "  vec3 diffAcc = vec3(0.0);\n"
			 "  vec3 specAcc = vec3(0.0);\n"
			 "  for (int i = 0; i < 8; ++i) {\n"
			 "    if (i >= uLightCount) break;\n"
			 "    int t = uLightType[i];\n"
			 "    if (t == 0) continue;\n"
			 "    vec3 L; float att = 1.0;\n"
			 "    if (t == 3) { L = normalize(-uLightDir[i]); }\n"
			 "    else {\n"
			 "      vec3 d = uLightPos[i] - worldPos; float dist = length(d);\n"
			 "      if (dist > uLightAtt[i].x) continue;\n"
			 "      L = d / max(dist, 1e-6);\n"
			 "      att = 1.0 / max(uLightAtt[i].y + uLightAtt[i].z * dist + uLightAtt[i].w * dist * dist, 1e-6);\n"
			 "      if (t == 2) {\n"
			 "        float rho = dot(-L, normalize(uLightDir[i]));\n"
			 "        float ct = uLightSpot[i].x, cp = uLightSpot[i].y;\n"
			 "        if (rho <= cp) att = 0.0;\n"
			 "        else if (rho < ct) att *= pow(clamp((rho - cp) / max(ct - cp, 1e-6), 0.0, 1.0), uLightSpot[i].z);\n"
			 "      }\n"
			 "    }\n"
			 "    ambAcc += att * uLightAmbient[i].rgb * mAmb.rgb;\n"
			 "    float ndl = dot(N, L);\n"
			 "    if (ndl > 0.0) {\n"
			 "      diffAcc += att * ndl * uLightDiffuse[i].rgb * mDiff.rgb;\n";
		if (key.specularEnable)
			s << "      vec3 H = normalize(L + V);\n"
				 "      float ndh = max(dot(N, H), 0.0);\n"
				 "      if (uMatPower > 0.0) specAcc += att * pow(ndh, uMatPower) * uLightSpecular[i].rgb * mSpec.rgb;\n";
		s << "    }\n"
			 "  }\n"
			 "  vDiffuse = vec4(clamp(mEmis.rgb + ambAcc + diffAcc, 0.0, 1.0), clamp(mDiff.a, 0.0, 1.0));\n"
			 "  vSpecular = vec4(clamp(specAcc, 0.0, 1.0), 0.0);\n";
	}
	else
	{
		s << "  vDiffuse = vtxDiffuse;\n"
			 "  vSpecular = vtxSpecular;\n";
	}

	// Sis
	if (key.fogMode != 0 && !key.rhw)
	{
		s << "  vec4 viewPos = uView * vec4(worldPos, 1.0);\n";
		s << (key.rangeFog ? "  float fd = length(viewPos.xyz);\n" : "  float fd = abs(viewPos.z);\n");
		if (key.fogMode == 1)
		{
			switch (key.fogKind)
			{
				case D3DFOG_EXP:  s << "  vFog = clamp(exp(-uFogParams.z * fd), 0.0, 1.0);\n"; break;
				case D3DFOG_EXP2: s << "  vFog = clamp(exp(-pow(uFogParams.z * fd, 2.0)), 0.0, 1.0);\n"; break;
				default:          s << "  vFog = clamp((uFogParams.y - fd) / max(uFogParams.y - uFogParams.x, 1e-6), 0.0, 1.0);\n"; break;
			}
		}
		else
			s << "  vFog = fd;\n"; // table fog: mesafe piksel başına değerlendirilir
	}
	else
		s << "  vFog = 1.0;\n";

	for (int i = 0; i < key.texCount; ++i)
		s << "  vTex" << i << " = aTex" << i << ";\n";

	if (key.hasPSize)
		s << "  gl_PointSize = aPSize;\n";
	else
		s << "  gl_PointSize = uPointSize;\n";
	s << "}\n";
	return s.str();
}

std::string BuildFragmentShader(const ProgramKey& key)
{
	std::ostringstream s;
	// GLSL ES 3.00 fragment shader'da highp zorunlu olarak desteklenir; vertex tarafıyla
	// uniform hassasiyetleri eşleşmeli (uFogParams vb.).
	s << "#version 300 es\n"
		 "precision highp float;\n"
		 "in vec4 vDiffuse;\n"
		 "in vec4 vSpecular;\n"
		 "in float vFog;\n";
	for (int i = 0; i < key.texCount; ++i)
		s << "in vec2 vTex" << i << ";\n";
	for (int i = 0; i < key.stageCount; ++i)
		s << "uniform sampler2D uTex" << i << ";\n";
	s << "uniform vec4 uTFactor;\n"
		 "uniform vec4 uFogColor;\n"
		 "uniform vec3 uFogParams;\n"
		 "uniform float uAlphaRef;\n"
		 "uniform int uAlphaFunc;\n"
		 "out vec4 fragColor;\n"
		 "void main() {\n"
		 "  vec4 cur = vDiffuse;\n"
		 "  vec4 tmp = vec4(0.0);\n"
		 "  vec4 tex = vec4(1.0);\n";

	for (int i = 0; i < key.stageCount; ++i)
	{
		const auto& st = key.stage[i];
		int tci        = st.texCoordIndex;
		if (tci >= key.texCount)
			tci = key.texCount > 0 ? 0 : -1;
		if (tci >= 0)
			s << "  tex = texture(uTex" << i << ", vTex" << tci << ");\n";
		else
			s << "  tex = texture(uTex" << i << ", vec2(0.0));\n";

		std::string c1 = ArgExpr(st.colorArg1, false), c2 = ArgExpr(st.colorArg2, false);
		std::string a1 = ArgExpr(st.alphaArg1, true), a2 = ArgExpr(st.alphaArg2, true);
		std::string colorExpr = OpExpr(st.colorOp, c1, c2, false, "tex.a", "vDiffuse.a", "cur.a");
		std::string alphaExpr = (st.alphaOp == D3DTOP_DISABLE) ? std::string("cur.a") : OpExpr(st.alphaOp, a1, a2, true, "tex.a", "vDiffuse.a", "cur.a");
		s << "  { vec3 c = clamp(" << colorExpr << ", 0.0, 1.0); float a = clamp(" << alphaExpr << ", 0.0, 1.0); cur = vec4(c, a); }\n";
	}

	if (key.specularEnable)
		s << "  cur.rgb = clamp(cur.rgb + vSpecular.rgb, 0.0, 1.0);\n";

	if (key.fogMode == 1)
		s << "  cur.rgb = mix(uFogColor.rgb, cur.rgb, clamp(vFog, 0.0, 1.0));\n";
	else if (key.fogMode == 2)
	{
		switch (key.fogKind)
		{
			case D3DFOG_EXP:  s << "  float ff = clamp(exp(-uFogParams.z * vFog), 0.0, 1.0);\n"; break;
			case D3DFOG_EXP2: s << "  float ff = clamp(exp(-pow(uFogParams.z * vFog, 2.0)), 0.0, 1.0);\n"; break;
			default:          s << "  float ff = clamp((uFogParams.y - vFog) / max(uFogParams.y - uFogParams.x, 1e-6), 0.0, 1.0);\n"; break;
		}
		s << "  cur.rgb = mix(uFogColor.rgb, cur.rgb, ff);\n";
	}

	if (key.alphaTest)
	{
		s << "  bool pass = true;\n"
			 "  float a = cur.a;\n"
			 "  if (uAlphaFunc == 1) pass = false;\n"
			 "  else if (uAlphaFunc == 2) pass = a < uAlphaRef;\n"
			 "  else if (uAlphaFunc == 3) pass = abs(a - uAlphaRef) < 0.002;\n"
			 "  else if (uAlphaFunc == 4) pass = a <= uAlphaRef;\n"
			 "  else if (uAlphaFunc == 5) pass = a > uAlphaRef;\n"
			 "  else if (uAlphaFunc == 6) pass = abs(a - uAlphaRef) >= 0.002;\n"
			 "  else if (uAlphaFunc == 7) pass = a >= uAlphaRef;\n"
			 "  if (!pass) discard;\n";
	}

	s << "  fragColor = cur;\n"
		 "}\n";
	return s.str();
}

} // namespace d3d9gles
