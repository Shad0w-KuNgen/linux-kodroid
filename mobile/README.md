# Knight Online Mobile (OpenKO 1.298 tabanlı port)

Bu dizin, açık kaynak **OpenKO** (Knight Online 1.298, MIT lisansı) istemcisini
Android/Linux'ta **birebir aynı oyun koduyla** çalıştırmak için yapılan portu içerir.
Oyun mantığı, arayüz (UIF), dosya formatları ve ağ protokolü OpenKO ile aynıdır;
yalnızca Windows'a özgü katmanlar (DirectX 9, Win32, DirectInput, Winsock, GDI) değiştirildi.

![Login ekranı — Linux, OpenGL ES 3 (Mesa llvmpipe)](docs/login-screen-linux-gles.png)

## Durum

| Parça | Durum |
|---|---|
| `d3d9gles/` DirectX 9 → OpenGL ES 3.0 katmanı | Çalışıyor. Sabit işlev boru hattı (köşe aydınlatması, 8 ışık, sis, doku aşamaları, alfa testi), DXT1/3/5 (donanım yoksa CPU çözücü), A8R8G8B8/A4R4G4B4/A1R5G5B5/R5G6B5, köşe/indeks tamponları, mip üretimi. Başsız piksel testleri geçiyor. |
| `compat/win32/` Win32 uyumluluk | Çalışıyor: tipler, RECT/POINT, zamanlayıcılar, kayıt defteri (dosya), INI, `_findfirst`, Winsock (`WSAAsyncSelect` → `poll`), yol çözümleme (`\` ve harf duyarsızlık). |
| `engine-port/` N3Base taşınan parçalar | FreeType yazı tipi, SHA-1/RC4 doku şifresi, SDL metin girişi, POSIX mmap dosya okuyucu, OpenGL ekran görüntüsü. |
| `platform/` SDL2 kabuğu | WinMain/WndProc yerine SDL olay döngüsü; klavye (DIK eşlemesi), fare, dokunmatik (tek parmak = sol tık, iki parmak = sağ tık), tekerlek, odak, soket olayları. |
| Linux (x86_64) derleme ve çalıştırma | **Login ekranı çiziliyor** (yukarıdaki görüntü, 1.298 istemci verileriyle, başsız Mesa üzerinde). Sunucuya bağlanma kodu değişmedi; OpenKO sunucusu ile test edilmedi. |
| `android/` Gradle projesi | Yazıldı, **henüz derlenmedi** (bu ortamda Android SDK/NDK yok). İlk Android derlemesinde CMake/bağımlılık ayarı gerekebilir. |
| Dokunmatik oyun arayüzü (sanal joystick, beceri çubuğu, yakınlaştırma) | Yapılmadı. Şimdilik dokunma fare gibi davranır. |
| Ses (OpenAL) | Derleniyor; cihazda test edilmedi. |
| BMP/JPG/TGA doku yükleme (`D3DXCreateTextureFromFileEx`) | Yapılmadı (oyun dokuları `.DXT`, nadiren gerekir). |
| Korece metin (CP949) | Linux'ta iconv ile; Android'de yalnızca ASCII (1.298 US verileri İngilizce). |

## Dizin yapısı

```
mobile/
  openko/        Open-KO/KnightOnline (git subtree, MIT) — küçük #ifdef yamalarıyla
  d3d9gles/      Direct3D 9 API'sinin OpenGL ES 3 uygulaması (+ tests/smoke_test.cpp)
  compat/win32/  windows.h, dinput.h, winsock2.h, wincrypt.h ... uyumluluk başlıkları
  engine-port/   DFont_ft.cpp, WinCrypt_portable.cpp, N3UIEdit_portable.cpp, FileReader_posix.cpp, JpegFile_portable.cpp
  platform/      main_sdl.cpp, LocalInput_sdl.cpp, KoPlatformInput.cpp
  android/       Gradle + CMake Android projesi (SDLActivity türevi)
  cmake/deps.cmake  masaüstünde pkg-config, Android'de FetchContent
```

## Linux'ta derleme ve çalıştırma

Gerekli paketler (Ubuntu 24.04):

```
sudo apt install build-essential cmake ninja-build pkg-config libgles-dev libegl-dev libsdl2-dev \
     libfreetype-dev libopenal-dev libmpg123-dev libjpeg-dev libspdlog-dev libfmt-dev
```

Derleme:

```
cd mobile
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build
ctest --test-dir build        # d3d9gles ve şifre testleri
```

İstemci verileri (1.298 US, ~1.1 GB):

```
git clone --depth 1 -b openko-1298 https://github.com/Open-KO/ko-client-assets.git assets
cp assets/Server.ini.default assets/Server.ini   # IP0=sunucu adresi
```

Çalıştırma:

```
KO_CLIENT_DIR=/yol/assets ./build/KnightOnLine
```

Başsız doğrulama (ekran yok, Mesa yazılım çizici):

```
SDL_VIDEODRIVER=offscreen EGL_PLATFORM=surfaceless LIBGL_ALWAYS_SOFTWARE=1 \
KO_CLIENT_DIR=/yol/assets KO_MAX_FRAMES=20 KO_SCREENSHOT=/tmp/shot.ppm ./build/KnightOnLine
```

Ortam değişkenleri: `KO_CLIENT_DIR` (veri dizini), `KO_FONT_PATH` (TTF; yoksa `<veri>/fonts/default.ttf`
veya sistem yazı tipi), `KO_MAX_FRAMES`, `KO_SCREENSHOT`, `D3D9GLES_NO_S3TC=1` (DXT CPU çözücüyü zorla).

## Android

`android/` Gradle projesi SDL'in `SDLActivity`'sini kullanır; yerel kod `libKnightOnLine.so` olarak
derlenir (`mobile/CMakeLists.txt`, `ANDROID` dalı). Bağımlılıklar `cmake/deps.cmake` içinde
FetchContent ile çekilir (SDL2 2.30, openal-soft, mpg123, libjpeg, FreeType, spdlog).

```
cd mobile/android
./gradlew assembleDebug       # Android Studio / SDK + NDK r27 gerekir
adb install app/build/outputs/apk/debug/app-debug.apk
adb push assets/. /sdcard/Android/data/online.knight.mobile/files/
```

Veri dizini `SDL_AndroidGetExternalStoragePath()` ile bulunur; `Option.ini` ve `Server.ini` oraya konur.
Yazı tipi için `<veri>/fonts/default.ttf` koyun (yoksa `/system/fonts/` denenir).

## Nasıl çalışıyor (kısa)

- Oyun motoru `s_lpD3DDev->SetRenderState(...)`, `DrawPrimitiveUP(...)` gibi D3D9 çağrıları yapar.
  `d3d9gles` bu çağrıları olduğu gibi alır, durumu izler ve her çizimde durum anahtarına göre
  üretilen GLSL ES 3.00 programıyla OpenGL ES'te çizer. D3D'nin saat yönü ön yüz, sol el koordinat,
  `[0,1]` derinlik ve piksel merkezi kuralları shader/GL durumunda birebir karşılanır.
- `.DXT` dokular (v7) RC4 ile şifreli; `WinCrypt_portable.cpp` CryptoAPI'nin `CryptDeriveKey`
  türetimini (SHA-1 ilk 16 bayt) ve her parçada sıfırlanan RC4 akışını aynen uygular.
- Dosya adları oyunda küçük harf ve `\` ile gelir; `KoResolvePath` gerçek dosyayı harf duyarsız bulur.

## Yol haritası

1. Android'de ilk derleme ve cihazda login ekranı.
2. OpenKO sunucusu (Ebenezer/Aujard) ile giriş, karakter seçimi, Moradon'a giriş testi.
3. Dokunmatik oyun arayüzü: sanal joystick, hedef seçme, beceri çubuğu, sohbet için sanal klavye.
4. Performans: shader/uniform önbelleği, doku belleği (CPU gölge kopyalarını serbest bırakma), ETC2/ASTC doku ön dönüşümü.
5. Ses testleri, BMP/JPG doku yükleme, iOS (aynı kod + SDL iOS projesi).
