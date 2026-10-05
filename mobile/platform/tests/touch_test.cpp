// touch_test.cpp — dokunmatik kaplama + CLocalInput başsız testi.
// Parmak olaylarını simüle eder, her karede Update()+Tick() çalıştırır ve oyunun gördüğü
// fare bayraklarını (MOUSE_LBCLICK/LBDOWN/LBCLICKED/RB*) ve imleç konumunu doğrular.
#include "StdAfx.h"
#include "GameProcedure.h"
#include "LocalInput.h"
#include "UIManager.h"
#include <N3Base/N3UIBase.h>

#include "KoPlatformInput.h"
#include "KoTouchOverlay.h"

#include <windows.h>
#include <d3d9.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <N3Base/N3Base.h>
#include <cstring>
#include <string>

#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

static int g_fail = 0;
#define CHECK(cond, ...)                                                   \
	do                                                                     \
	{                                                                      \
		if (!(cond))                                                       \
		{                                                                  \
			std::printf("  BASARISIZ %s:%d: ", __FILE__, __LINE__);         \
			std::printf(__VA_ARGS__);                                      \
			std::printf("\n");                                             \
			++g_fail;                                                      \
		}                                                                  \
	} while (0)

static void ClientSize(int* w, int* h)
{
	*w = 1366;
	*h = 768;
}

static CLocalInput* g_li;
static int Frame()
{
	KoTouch().Update();
	g_li->Tick();
	return g_li->MouseGetFlag();
}
static void Reset()
{
	KoInputState& in = KoInput();
	in = KoInputState {};
	in.windowFocused = true;
	for (int i = 0; i < 3; ++i)
		Frame();
}

static void TestTap()
{
	std::printf("[test] tek dokunus = bir kare LBCLICK, sonraki kare LBCLICKED\n");
	Reset();
	KoTouch().OnFingerDown(1, 600, 400);
	KoTouch().OnFingerUp(1, 600, 400); // ayni karede bas+birak
	int f1 = Frame();
	CHECK((f1 & MOUSE_LBCLICK) && (f1 & MOUSE_LBDOWN), "kare1 bayrak 0x%X", f1);
	CHECK(g_li->MouseGetPos().x == 600 && g_li->MouseGetPos().y == 400, "kare1 konum");
	int f2 = Frame();
	CHECK((f2 & MOUSE_LBCLICKED) && !(f2 & MOUSE_LBDOWN), "kare2 bayrak 0x%X", f2);
	int f3 = Frame();
	CHECK(f3 == 0, "kare3 bayrak 0x%X (bos olmali)", f3);
}

static void TestDoubleTap()
{
	std::printf("[test] cift dokunus = ikinci birakista LBDBLCLK (saldiri)\n");
	Reset();
	KoTouch().OnFingerDown(1, 600, 400);
	KoTouch().OnFingerUp(1, 600, 400);
	KoTouch().OnFingerDown(2, 602, 401);
	KoTouch().OnFingerUp(2, 602, 401);
	int f1 = Frame(), f2 = Frame(), f3 = Frame(), f4 = Frame();
	CHECK(f1 & MOUSE_LBCLICK, "kare1 0x%X", f1);
	CHECK(f2 & MOUSE_LBCLICKED, "kare2 0x%X", f2);
	CHECK(f3 & MOUSE_LBCLICK, "kare3 0x%X (ikinci tik birlesmemeli)", f3);
	CHECK((f4 & MOUSE_LBCLICKED) && (f4 & MOUSE_LBDBLCLK), "kare4 0x%X (LBDBLCLK bekleniyor)", f4);
}

static void TestDragHold()
{
	std::printf("[test] surukleme: LBCLICK ve ilk LBDOWN baslangic noktasinda, sonra takip, birakista LBCLICKED hedefte\n");
	Reset();
	KoTouch().OnFingerDown(1, 300, 300);
	KoTouch().OnFingerMotion(1, 340, 330); // ayni karede esik asildi
	KoTouch().OnFingerMotion(1, 380, 360);
	int f1 = Frame();
	CHECK((f1 & MOUSE_LBCLICK) && g_li->MouseGetPos().x == 300 && g_li->MouseGetPos().y == 300,
		"kare1 0x%X konum %ld,%ld (baslangicta LBCLICK bekleniyor)", f1, g_li->MouseGetPos().x, g_li->MouseGetPos().y);
	int f2 = Frame();
	CHECK((f2 & MOUSE_LBDOWN) && !(f2 & MOUSE_LBCLICK) && g_li->MouseGetPos().x == 300, "kare2 0x%X konum %ld", f2, g_li->MouseGetPos().x);
	KoTouch().OnFingerMotion(1, 500, 420);
	int f3 = Frame();
	CHECK((f3 & MOUSE_LBDOWN) && g_li->MouseGetPos().x == 500 && g_li->MouseGetPos().y == 420, "kare3 0x%X konum %ld,%ld", f3, g_li->MouseGetPos().x, g_li->MouseGetPos().y);
	KoTouch().OnFingerUp(1, 520, 430);
	int f4 = Frame();
	CHECK((f4 & MOUSE_LBCLICKED) && g_li->MouseGetPos().x == 520, "kare4 0x%X konum %ld (hedefte birakma)", f4, g_li->MouseGetPos().x);
}

static void TestLongPressWorld()
{
	std::printf("[test] 3D dunyada uzun basis = sag tik (RBCLICK, sonra RBCLICKED)\n");
	Reset();
	KoTouchOverlay::Tuning t = KoTouch().GetTuning();
	t.longPressMs           = 1;
	KoTouch().SetTuning(t);
	KoTouch().OnFingerDown(1, 700, 300);
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	int f1 = Frame();
	CHECK((f1 & MOUSE_RBCLICK) && (f1 & MOUSE_RBDOWN), "kare1 0x%X", f1);
	int f2 = Frame();
	CHECK((f2 & MOUSE_RBCLICKED), "kare2 0x%X", f2);
	KoTouch().OnFingerUp(1, 700, 300);
	int f3 = Frame();
	CHECK(!(f3 & (MOUSE_LBCLICK | MOUSE_RBCLICK)), "kare3 0x%X (kalkis ek tik uretmemeli)", f3);
}

static void TestLongPressOnUIStickyDrag()
{
	std::printf("[test] arayuz ustunde uzun basis = ikonu tut; parmak kalkinca yapiskan; ikinci dokunusta hedefe birak\n");
	Reset();
	CUIManager* mgr = new CUIManager();
	CN3UIBase* wnd  = new CN3UIBase();
	wnd->Init(mgr);
	RECT rc = {100, 100, 400, 400};
	wnd->SetRegion(rc);
	wnd->SetVisible(true);
	mgr->SetVisible(true);
	CGameProcedure::s_pUIMgr = mgr;

	KoTouchOverlay::Tuning t = KoTouch().GetTuning();
	t.longPressMs           = 1;
	KoTouch().SetTuning(t);

	KoTouch().OnFingerDown(1, 200, 200);
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	int f1 = Frame();
	CHECK((f1 & MOUSE_LBCLICK) && !(f1 & MOUSE_RBCLICK) && g_li->MouseGetPos().x == 200, "kare1 0x%X (LBCLICK ikon ustunde, sag tik yok)", f1);
	int f2 = Frame();
	CHECK((f2 & MOUSE_LBDOWN) && g_li->MouseGetPos().x == 200, "kare2 0x%X", f2);
	KoTouch().OnFingerUp(1, 201, 200); // kipirdamadan kalkti → yapiskan
	int f3 = Frame();
	CHECK((f3 & MOUSE_LBDOWN) && !(f3 & MOUSE_LBCLICKED) && KoInput().stickyDrag, "kare3 0x%X yapiskan=%d (tus basili kalmali)", f3, (int) KoInput().stickyDrag);
	// Hedef yuvaya dokun
	KoTouch().OnFingerDown(2, 350, 350);
	KoTouch().OnFingerUp(2, 350, 350);
	int f4 = Frame();
	CHECK((f4 & MOUSE_LBDOWN) && g_li->MouseGetPos().x == 350 && g_li->MouseGetPos().y == 350, "kare4 0x%X konum %ld (ikon hedefe tasindi)", f4, g_li->MouseGetPos().x);
	int f5 = Frame();
	CHECK((f5 & MOUSE_LBCLICKED) && g_li->MouseGetPos().x == 350, "kare5 0x%X (hedefte birakma)", f5);
	CHECK(!KoInput().stickyDrag, "yapiskan mod kapanmali");

	// Arayuz ustunde surukleme (ikon tasima): esik asilinca sol surukleme, kamera degil
	std::printf("[test] arayuz ustunde surukleme = sol tus surukleme (kamera degil)\n");
	Reset();
	KoTouch().OnFingerDown(3, 150, 150);
	KoTouch().OnFingerMotion(3, 190, 190);
	int g1 = Frame();
	CHECK((g1 & MOUSE_LBCLICK) && !(g1 & MOUSE_RBDOWN), "kare1 0x%X", g1);
	KoTouch().OnFingerUp(3, 300, 300);
	Frame();
	Frame();

	// Arayuz ikonunda cift dokunus = sag tik (esya kullan / giy)
	std::printf("[test] arayuz ustunde cift dokunus = sag tik (RBCLICK + RBCLICKED ayni yerde)\n");
	Reset();
	KoTouch().OnFingerDown(4, 250, 250);
	KoTouch().OnFingerUp(4, 250, 250);
	KoTouch().OnFingerDown(5, 252, 251);
	KoTouch().OnFingerUp(5, 252, 251);
	int h1 = Frame(), h2 = Frame(), h3 = Frame(), h4 = Frame();
	CHECK((h1 & MOUSE_LBCLICK) && (h2 & MOUSE_LBCLICKED), "ilk dokunus sol tik: 0x%X 0x%X", h1, h2);
	CHECK((h3 & MOUSE_RBCLICK) && !(h3 & MOUSE_LBCLICK), "ikinci dokunus sag tik: 0x%X", h3);
	CHECK((h4 & MOUSE_RBCLICKED) && g_li->MouseGetPos().x == 252, "sag tik birakma: 0x%X", h4);

	CGameProcedure::s_pUIMgr = nullptr;
	delete mgr; // cocuklari da siler
}

// Düğmeler birbirine binmemeli (ekranda yanlış tuş algılanıyordu). minTouchPx farklı DPI'ları temsil eder.
static void TestNoOverlap(float minTouchPx, int w, int h)
{
	std::printf("[test] dugmeler ust uste binmiyor (minTouch=%.0f, %dx%d)\n", minTouchPx, w, h);
	KoTouchOverlay::Tuning t = KoTouch().GetTuning();
	t.minTouchPx            = minTouchPx;
	KoTouch().SetTuning(t);
	KoTouch().Layout(w, h);
	size_t n = KoTouch().ButtonCount();
	for (size_t i = 0; i < n; ++i)
	{
		float ax0, ay0, ax1, ay1;
		std::string al;
		KoTouch().ButtonBox(i, &ax0, &ay0, &ax1, &ay1, &al);
		CHECK(ax0 >= -1 && ay0 >= -1 && ax1 <= w + 1 && ay1 <= h + 1, "%s ekran disina tasiyor (%.0f,%.0f)-(%.0f,%.0f)", al.c_str(), ax0, ay0, ax1, ay1);
		for (size_t j = i + 1; j < n; ++j)
		{
			float bx0, by0, bx1, by1;
			std::string bl;
			KoTouch().ButtonBox(j, &bx0, &by0, &bx1, &by1, &bl);
			bool overlap = ax0 < bx1 - 1 && bx0 < ax1 - 1 && ay0 < by1 - 1 && by0 < ay1 - 1;
			CHECK(!overlap, "%s ile %s cakisiyor", al.c_str(), bl.c_str());
		}
	}
	t.minTouchPx = 0;
	KoTouch().SetTuning(t);
	KoTouch().Layout(1366, 768);
}

// --render cikti.ppm [minTouchPx]: kaplamayi bassiz cizip PPM olarak kaydet (duzen gozle denetimi)
static int RenderOverlay(const char* out, float minTouchPx)
{
	const int W = 1366, H = 768;
	PFNEGLGETPLATFORMDISPLAYEXTPROC getPlat = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
	EGLDisplay dpy = getPlat ? getPlat(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr) : eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (dpy == EGL_NO_DISPLAY || !eglInitialize(dpy, nullptr, nullptr))
	{
		std::printf("EGL yok\n");
		return 1;
	}
	EGLint attr[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8, EGL_NONE};
	EGLConfig cfg;
	EGLint n = 0;
	eglChooseConfig(dpy, attr, &cfg, 1, &n);
	eglBindAPI(EGL_OPENGL_ES_API);
	EGLint ca[]     = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
	EGLContext ctx  = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ca);
	EGLint pa[]     = {EGL_WIDTH, W, EGL_HEIGHT, H, EGL_NONE};
	EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pa);
	eglMakeCurrent(dpy, surf, surf, ctx);
	d3d9gles::PlatformHooks hooks;
	hooks.getDrawableSize = [](void*, int* w, int* h) { *w = 1366; *h = 768; };
	hooks.present         = [](void*) { glFinish(); };
	d3d9gles::SetPlatformHooks(hooks);
	IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS pp {};
	pp.Windowed = TRUE; pp.BackBufferWidth = W; pp.BackBufferHeight = H; pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	pp.EnableAutoDepthStencil = TRUE; pp.AutoDepthStencilFormat = D3DFMT_D24S8; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	IDirect3DDevice9* dev = nullptr;
	if (FAILED(d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &dev)) || !dev)
		return 1;
	CN3Base::s_lpD3DDev = dev;
	dev->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(60, 80, 60), 1.0f, 0);
	KoTouchOverlay::Tuning t = KoTouch().GetTuning();
	t.minTouchPx            = minTouchPx;
	KoTouch().SetTuning(t);
	KoTouch().Layout(W, H);
	KoTouch().SetEnabled(true);
	KoTouch().SetForceVisible(true);
	KoTouch().Render(dev);
	glFinish();
	std::vector<unsigned char> rgba((size_t) W * H * 4);
	glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
	FILE* f = std::fopen(out, "wb");
	if (!f)
		return 1;
	std::fprintf(f, "P6\n%d %d\n255\n", W, H);
	for (int y = H - 1; y >= 0; --y)
		for (int x = 0; x < W; ++x)
			std::fwrite(&rgba[((size_t) y * W + x) * 4], 1, 3, f);
	std::fclose(f);
	std::printf("kaplama cizildi: %s\n", out);
	return 0;
}

static void TestMenuKeys()
{
	std::printf("[test] alt cubuk: MENU=H, KAPAT=ESC, YARDIM=F10; beceri sayfasi dugmesi F1..F8\n");
	Reset();
	// Dugmeler oyun icinde etkin; burada yalniz dizilim tablosu denetlenir
	CHECK(KM_TOGGLE_CMDLIST == DIK_H, "KM_TOGGLE_CMDLIST H olmali");
	CHECK(KM_SKILL_PAGE_1 + 7 == KM_SKILL_PAGE_8, "F1..F8 ardisik olmali");
}

int main(int argc, char** argv)
{
	KoWin32GetHooks().getClientSize = ClientSize;
	if (argc >= 3 && std::strcmp(argv[1], "--render") == 0)
		return RenderOverlay(argv[2], argc >= 4 ? (float) std::atof(argv[3]) : 0.0f);
	CLocalInput li;
	li.Init(nullptr, nullptr);
	g_li = &li;
	KoTouch().Layout(1366, 768);
	KoTouch().SetEnabled(true);

	TestTap();
	TestDoubleTap();
	TestDragHold();
	TestLongPressWorld();
	TestLongPressOnUIStickyDrag();
	TestMenuKeys();
	TestNoOverlap(0.0f, 1366, 768);
	TestNoOverlap(86.0f, 1366, 768);   // ~400 dpi telefon
	TestNoOverlap(64.0f, 1024, 768);   // tablet 4:3
	TestNoOverlap(100.0f, 1366, 768);  // çok yüksek dpi

	std::printf(g_fail ? "SONUC: %d hata\n" : "SONUC: tum testler gecti\n", g_fail);
	return g_fail ? 1 : 0;
}
