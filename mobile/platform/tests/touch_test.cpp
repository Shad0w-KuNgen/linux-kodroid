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

	CGameProcedure::s_pUIMgr = nullptr;
	delete mgr; // cocuklari da siler
}

static void TestMenuKeys()
{
	std::printf("[test] alt cubuk: MENU=H, KAPAT=ESC, YARDIM=F10; beceri sayfasi dugmesi F1..F8\n");
	Reset();
	// Dugmeler oyun icinde etkin; burada yalniz dizilim tablosu denetlenir
	CHECK(KM_TOGGLE_CMDLIST == DIK_H, "KM_TOGGLE_CMDLIST H olmali");
	CHECK(KM_SKILL_PAGE_1 + 7 == KM_SKILL_PAGE_8, "F1..F8 ardisik olmali");
}

int main()
{
	KoWin32GetHooks().getClientSize = ClientSize;
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

	std::printf(g_fail ? "SONUC: %d hata\n" : "SONUC: tum testler gecti\n", g_fail);
	return g_fail ? 1 : 0;
}
