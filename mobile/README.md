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
| Dokunmatik oyun arayüzü (`platform/KoTouchOverlay`) | Yazıldı (KO Mobile düzeni): yüzen joystick, SALDIR + 8 beceri yuvası + HEDEF, kamera kümesi, alt menü çubuğu, dokunuş = sol tık, sürükleme = kamera, iki parmak = yakınlaştırma. Cihazda henüz denenmedi. |
| Ekran ölçekleme | Telefonlarda oyun 1366x768 (16:9) veya 1024x768 (4:3) mantıksal çözünürlükte çizilir, ekrana en-boy oranı korunarak yerleştirilir (yanlarda bant). `[Mobile] LogicalHeight`, `D3D9GLES_STRETCH=1`. |
| GitHub Actions (`.github/workflows/build.yml`) | Linux derleme + testler geçiyor; Android APK işi üzerinde çalışılıyor (artifact olarak APK üretir). |
| Ses (OpenAL) | Derleniyor; cihazda test edilmedi. |
| BMP/JPG/TGA doku yükleme (`D3DXCreateTextureFromFileEx`) | Yapılmadı (oyun dokuları `.DXT`, nadiren gerekir). |
| Korece metin (CP949) | Linux'ta iconv ile; Android'de yalnızca ASCII (1.298 US verileri İngilizce). |

![Simüle 20:9 telefon ekranı, dokunmatik kaplama](docs/phone-20x9-touch-overlay.png)

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
veya sistem yazı tipi), `KO_MAX_FRAMES`, `KO_SCREENSHOT` (mantıksal), `KO_SCREENSHOT_PHYSICAL` (bantlar dahil),
`KO_TOUCH=1` (masaüstünde dokunmatik kaplama), `KO_TOUCH_DEBUG=1` (kaplamayı login'de de çiz),
`KO_LOGICAL_HEIGHT=768` (telefon ölçeklemesini masaüstünde dene), `D3D9GLES_NO_S3TC=1`, `D3D9GLES_STRETCH=1`.

`Option.ini` içinde `[Mobile]` bölümü: `LogicalHeight=768` (0 = ölçekleme yok), `TouchControls=1`.

Not: Oyunun arayüzü 1024x768, 1280x1024, 1366x768, 1600x1200 ve 1920x1080 için tasarlanmış;
başka çözünürlüklerde login arka planı eksik/bozuk çıkar. Bu yüzden mobilde mantıksal çözünürlük
bu listeden seçilir ve ekrana ölçeklenir.

## Android

`android/` Gradle projesi SDL'in `SDLActivity`'sini kullanır; yerel kod `libKnightOnLine.so` olarak
derlenir (`mobile/CMakeLists.txt`, `ANDROID` dalı). Bağımlılıklar `cmake/deps.cmake` içinde
FetchContent ile çekilir (SDL2 2.30, openal-soft, mpg123, libjpeg, FreeType, spdlog).

```
cd mobile/android
./gradlew assembleDebug       # Android Studio / SDK + NDK r27 gerekir
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

Debug APK depodaki sabit `debug.keystore` ile imzalanır; böylece her CI derlemesi bir öncekinin
üzerine kurulabilir (farklı anahtar → `INSTALL_FAILED_UPDATE_INCOMPATIBLE`).

### Oyun verisini telefona kurma

Uygulama açılınca `SetupActivity` oyun verisini arar; yoksa iki seçenek sunar:

1. **Zip dosyası seç**: istemci klasörünü (`UI/`, `Data/`, `Misc/`, `Server.ini`, `Option.ini`, ...)
   tek bir `.zip` yapıp telefona atın (Download klasörü yeterli) ve seçin. Zip tek bir üst klasörle
   paketlenmişse içeriği otomatik kök dizine taşınır.
2. **Adresten indir**: aynı zip'in `http(s)://` adresini yazın; akış doğrudan veri dizinine açılır.
   Alan varsayılan olarak `http://86.105.4.195/knightonline-mobile.zip` ile gelir
   (`GameData.DEFAULT_DATA_URL`). Bağlantı koparsa `ResumingHttpStream` kaldığı bayttan
   `Range` isteğiyle yeniden bağlanır (30 denemeye kadar, üstel bekleme); sunucu `Range`
   desteklemiyorsa eksik baytlar okunup atılır. Ekranda yüzde, alınan/toplam MB, hız ve kalan süre
   gösterilir; indirme boyunca ekran açık tutulur.

Kurulumdan sonra ve her oyun başlatılışında `Server.ini` denetlenir: dosya yoksa yazılır, `[Server]`
altında `IP0` yok ya da `127.0.0.1`/boş ise `IP0=86.105.4.195` (`GameData.DEFAULT_SERVER_IP`) yapılır.
Elle başka bir adres yazılmışsa dokunulmaz.

Veri dizini `Android/data/online.knight.mobile/files/` (yoksa `/data/data/online.knight.mobile/files/`),
oyuna `--client-dir` argümanıyla iletilir. Android 11+ sürümlerinde `adb push` ile buraya atılan
dosyaların sahibi `shell` olduğundan oyun bunları okuyamaz; o yüzden veri uygulamanın kendisi
tarafından açılır. Root'suz cihazda adb ile hızlı yükleme (debug APK, `run-as` ile):

```
adb push ko-assets.zip /data/local/tmp/ko.zip
adb shell run-as online.knight.mobile sh -c 'mkdir -p files && cd files && unzip -o /data/local/tmp/ko.zip'
# unzip yoksa: tar ile paketleyip  run-as online.knight.mobile tar -xf /data/local/tmp/ko.tar -C files
```

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
