// anim_test.cpp — 2369 (ISTIRAP) .n3anim: DES katmanlı kayıtlar 1.298 ile aynı adlara/karelere çözülmeli.
#include "N3AnimControl.h"

#include <cstdio>
#include <cstdlib>
#include <string>

static int g_fail = 0;
#define CHECK(cond, ...)                                      \
	do                                                        \
	{                                                         \
		if (!(cond))                                          \
		{                                                     \
			std::printf("HATA %s:%d: ", __FILE__, __LINE__); \
			std::printf(__VA_ARGS__);                         \
			std::printf("\n");                                \
			g_fail++;                                         \
		}                                                     \
	} while (0)

int main()
{
	std::string dir = KO_TEST_DATA_DIR "/2369/";
	CN3AnimControl a2369, a1298, npc;
	CHECK(a2369.LoadFromFile(dir + "upc_el_ba.n3anim"), "2369 n3anim yuklenemedi");
	CHECK(a1298.LoadFromFile(dir + "upc_el_ba.1298.n3anim"), "1298 n3anim yuklenemedi");
	CHECK(npc.LoadFromFile(dir + "npc_el_ba.n3Anim"), "2369 npc n3anim yuklenemedi");
	std::printf("2369: %d kayit, 1298: %d kayit, npc: %d kayit\n", a2369.Count(), a1298.Count(), npc.Count());
	CHECK(a2369.Count() == 155, "2369 kayit sayisi %d", a2369.Count());
	CHECK(a1298.Count() == 136, "1298 kayit sayisi %d", a1298.Count());
	CHECK(npc.Count() == 16, "npc kayit sayisi %d", npc.Count());
	// İlk 6 kayıt ad ve kare aralıklarıyla 1.298 ile aynı olmalı (breath, walk, run, walk_reverse, struck0, struck1)
	for (int i = 0; i < 6 && i < a2369.Count() && i < a1298.Count(); i++)
	{
		const __AnimData& a = *a2369.DataGet(i);
		const __AnimData& b = *a1298.DataGet(i);
		CHECK(a.szName == b.szName, "kayit %d ad: '%s' != '%s'", i, a.szName.c_str(), b.szName.c_str());
		CHECK(a.fFrmStart == b.fFrmStart && a.fFrmEnd == b.fFrmEnd && a.fFrmPerSec == b.fFrmPerSec, "kayit %d kareler: %g-%g@%g != %g-%g@%g", i,
			a.fFrmStart, a.fFrmEnd, a.fFrmPerSec, b.fFrmStart, b.fFrmEnd, b.fFrmPerSec);
		CHECK(a.fTimeBlend == b.fTimeBlend && a.iBlendFlags == b.iBlendFlags, "kayit %d blend", i);
	}
	CHECK(a2369.Count() > 100 && a2369.DataGet(100)->szName == "attack_Bash0_A", "kayit 100 adi '%s'",
		a2369.Count() > 100 ? a2369.DataGet(100)->szName.c_str() : "");
	CHECK(a2369.Count() > 154 && a2369.DataGet(154)->szName == "shoot", "son kayit adi");
	if (g_fail == 0)
		std::printf("ko_anim_test: tum testler gecti\n");
	return g_fail == 0 ? 0 : 1;
}
