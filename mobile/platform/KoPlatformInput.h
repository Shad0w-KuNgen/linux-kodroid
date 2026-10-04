// KoPlatformInput.h — platform (SDL) katmanı ile oyun girdisi arasındaki ortak durum.
//
// Oyun kodu (CLocalInput, _IsKeyDown) fare/dokunmatik ve klavye durumunu buradan okur;
// SDL olay döngüsü burayı doldurur. Dokunmatik ekranda tek parmak = sol tık, ikinci parmak
// = sağ tık olarak eşlenir (ilk sürüm; sanal joystick/beceri çubuğu sonraki aşama).
#ifndef KO_PLATFORM_INPUT_H
#define KO_PLATFORM_INPUT_H

#include <cstdint>

struct KoInputState
{
	int mouseX = 0, mouseY = 0;   // istemci piksel koordinatı
	bool lbDown = false, mbDown = false, rbDown = false;
	int wheelDelta = 0;           // birikmiş tekerlek (120 = bir çentik)
	bool windowFocused = true;
	uint8_t keysDIK[256] = {};    // DirectInput tarama kodu → basılı mı (0x80)
};

KoInputState& KoInput();

/// SDL klavye durumundan DIK dizisini günceller (her karede çağrılır).
void KoInputUpdateKeyboard();

/// Win32 sanal tuş kodu (VK_*) basılı mı? (compat GetAsyncKeyState kancası)
int KoInputIsVkDown(int vk);

#endif // KO_PLATFORM_INPUT_H
