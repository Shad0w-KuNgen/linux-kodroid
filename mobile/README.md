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

Ortam değişkenleri: `KO_INPUT_DEBUG=1` (ya da `Option.ini` `[Mobile] InputDebug=1`): dokunma ve metin
giriş olaylarını stderr/logcat'e yazar (`[ko-input] ...`). `KO_CLIENT_DIR` (veri dizini), `KO_FONT_PATH` (TTF; yoksa `<veri>/fonts/default.ttf`
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

### Launcher (LauncherActivity)

Uygulama PC'deki KO Launcher gibi tam ekran yatay bir başlatıcıyla açılır: arka plan (veri dizinine
`launcher_bg.png/jpg` konursa o kullanılır), haberler (VersionManager `LS_NEWS`, sunucudaki
`Version.ini [NEWS]`), sunucu durumu (`LS_SERVERLIST`: çevrimiçi/dolu + oyuncu sayısı), sürüm etiketi
(veri sürümü = `Server.ini [Version] Files`, APK sürümü), tek ilerleme çubuğu (yüzde, MB, hız, kalan
süre) ve **OYUNA BAŞLA**. Açılış sırası:

1. **APK güncellemesi**: `apk.json` bildirimi (varsayılan `http://86.105.4.195/ko/mobile/apk.json`,
   Ayarlar'dan değişir) okunur; `versionCode` kurulu olandan büyükse sorulur, APK kaldığı yerden
   devam eden indiriciyle önbelleğe alınır, `sha256` doğrulanır ve paket yükleyicisi açılır
   (Android 8+ "bilinmeyen uygulama" izni istenir). CI her derlemede artifact içine
   `KnightOnline.apk` + `apk.json` koyar; ikisi sunucuda aynı dizine kopyalanır.
   CI'da `versionCode = 100 + GITHUB_RUN_NUMBER`, `versionName = 0.1.<no>+<commit>`
   (`KO_VERSION_CODE`/`KO_VERSION_NAME`); yerel derlemede git commit sayısı.
2. **Veri kurulumu** (ilk sefer / "Veriyi onar"): adresten ya da seçilen zip'ten.
3. **Veri yaması**: `PatchClient` (aşağıda).
4. Hazır → OYUNA BAŞLA. "Ayarlar": sunucu IP, protokol (1.298/2369) ve portlar (Server.ini), veri/APK adresleri, dokunmatik kontroller,
   mantıksal yükseklik, girdi hata ayıklama (Option.ini `[Mobile]`).

### Sunucu protokolü: 1.298 (OpenKO) ve 2369 (ISTIRAP)

İstemci iki sunucu protokolüyle konuşabilir; seçim çalışma zamanında `Server.ini`'den okunur
(`KoProtocol.h`, hem yerel kod hem Launcher aynı kuralı uygular):

```ini
[Server]
Count=1
IP0=86.105.4.195
Protocol=2369     ; yoksa [Version] Files >= 2000 ise 2369, değilse 1298
LoginPort=15200   ; yoksa protokole göre 15200 / 15100
GamePort=15301    ; yoksa protokole göre 15301 / 15001
```

Launcher Ayarlar'ında "Sunucu protokolü" (1.298 / 2369), giriş/oyun portu ve veri adresi vardır;
2369 seçilince varsayılan veri adresi `http://86.105.4.195/ko/client2369.zip` olur (~3 GB,
14.115 dosya, `Files=2434`). Sunucu durumu/yama sorguları (`LS_SERVERLIST`, `LS_VERSION_REQ`,
`LS_DOWNLOADINFO_REQ`, `LS_NEWS`) protokole göre ayrıştırılır.

2369 düzenleri depodaki `1 - Login & Game Source` sunucu kaynağından türetildi (`__VERSION 2369`,
şifreleme yok, anti-cheat sunucuda zorunlu değil). Aşama 1'de uygulanan paketler:

| Paket | 2369 farkı |
|---|---|
| `LS_SERVERLIST` | istek `uint16 echo`; yanıt lanIP/IP/ad, kullanıcı, sunucu/grup ID, kapasite, ekran tipi, 4 kral/duyuru dizgesi |
| `LS_LOGIN_REQ` | `uint16 0` + sonuç (1 başarı, 2 hesap yok, 3 şifre, 4 yasaklı, 5 oyunda, 0xF sözleşme, 0x10 OTP) |
| `WIZ_VERSION_CHECK` | `uint8, uint16 sürüm, uint8, uint64, uint64, uint8`; sürüm 2369 ile karşılaştırılır |
| `WIZ_COMPRESS_PACKET` | başlık `uint32 × 3` (1298: `uint16, uint16, uint32`) |
| `WIZ_ALLCHAR_INFO_REQ` | istek alt opcode 1; yanıt 4 karakter, `uint32` saç (arayüzde 3 yuva gösterilir) |
| `WIZ_SEL_CHAR` | zoneCur baytı gönderilmez |
| `WIZ_GAMESTART` | adımlar 1/2 + karakter adı (`uint8` uzunluk) |
| `WIZ_MYINFO` | genişletilmiş düzen: `int64` exp, klan/pelerin bloğu, 75 envanter yuvası, premium/manner/rebirth kuyruğu |
| `WIZ_USER_INOUT` / `WIZ_REQ_USERIN` | `uint16` tür; `uint32` saç, abnormal, yön, 15 eşya yuvası; REQ_USERIN kayıtları önünde `uint8 0` |
| `WIZ_REGIONCHANGE` | alt opcode 0/1/2; liste yalnız 1'de |
| `WIZ_NPC_INOUT` / `WIZ_REQ_NPCIN` | ad paket içinde yok (görünüm tablosundaki model adı kullanılır); `int16` yön; tür 3/4 = giriş |
| `WIZ_NPC_MOVE` | önde `uint8 1` |
| `WIZ_MOVE` | gönderimde mevcut konum eklenir; echo 0 dur / 1 başla / 3 devam |
| `WIZ_CHAT` | ChatType → N3 sohbet kipi eşlemesi (`MapChatType2369`) |
| `WIZ_NOTICE` | biçim 1 (eski), 2 (başlık+mesaj çiftleri), 4 (sağ üst başlık iletisi) |
| `WIZ_ZONE_CHANGE` | ışınlanma: `uint16` bölge, x, z, y, ulus, eski zafer |

**2xxx veri tabloları (DES katmanı):** `client2369.zip` içindeki 243 `.tbl` dosyasının 233'ü yeni
biçimde: ham dosya `[16 bayt sabit başlık][uint32 BE özgün uzunluk][8 baytlık DES blokları]`
(IP/FP'siz DES, 16 tur, sabit tur anahtarları; bu dosyalarda klasik XOR katmanı yoktur), çözülen veri
akış XOR'u (0x0418/0x8041/0x1804) sonrası `[5 bayt önek][standart N3 tablosu]`. Algoritma
[co3moz/ko-tbl-reader](https://github.com/co3moz/ko-tbl-reader) (MIT) `double.js`'den türetildi;
`openko/src/N3Base/KoTableCrypt.{h,cpp}` uygular, `N3TableBaseImpl::LoadFromFile` başlığa göre otomatik
seçer (klasik XOR / DES). `ko_tbl_test` 2195 ve 1886 örnek tablolarıyla (platform/tests/data) çözümü
baştan sona doğrular. PC'de: `scripts/tbl_tool.py info <Data>` (sınıflandırma) ve `dump <tbl>` (CSV).
Temel tablolar (Texts/UIs/Zones) yine de okunamazsa oyun siyah ekran yerine hata kutusu gösterip kapanır.

**2xxx tablo şemaları:** 2369 tablolarında sütunlar eklenmiş (Zones 27, Item_Ext 56, skill_magic_main 37…).
`CN3TableBase::Load` dosya düzeni struct'a uymazsa dosya sütunlarını 1.298 şemasına
(`KoTableSchemas.cpp`, 1.298 verisinden üretildi) hizalar: bilinen tablolar için elle eşleme
(Zones: HDR gökyüzü, .mob, .NaviMesh sütunları atlanır), diğerleri için sırayı koruyan en uzun ortak
alt dizi; fazlalıklar okunup atılır, eşlenmeyen struct alanları varsayılan kalır. Her hizalama Log.txt'ye
"2xxx şeması hizalandı: … atlanan dosya sütunları […]" biçiminde yazılır; yanlış eşlenen bir tablo için
`MANUAL_MAPS`'e satır eklenir. Olmayan tablolar (Quest_Content, Help) yumuşak atlanır; `UI_US` ↔ `UI`
klasörü `KoResolvePath` ile karşılıklı yedeklenir. PC'de `scripts/tbl_tool.py schema <Data>` tüm tabloların
tür dizgisini ve ilk satırını listeler (eşleme doğrulama için).

**2369 arayüz dosyaları:** `UI\` klasöründe tek tek `.uif/.dxt` yok; `ui.hdr` (şifresiz dizin) +
`ui.src` (634 MB paket). `compat/win32/ko_vfs.cpp` "ui\<ad>" / "ui_us\<ad>" isteklerinde kaydı
`ui_cache/<ad>` altına çıkarıp o yolu döndürür (`KoResolvePath` içinden; ilk kullanımda bir kez).
ISTIRAP'a özel `ISTIRAP\*.istirap` UIF'leri Pearl Guard `dcpUIF` şemasıyla şifreli (ilk 4 bayt düz,
32/31 baytlık bloklar, her blok RC4 başından, anahtar SHA1(parola[:29])[:16]); `.istirap` açılınca
`ui_cache/istirap/<ad>.uif` olarak çözülür. `ko_vfs_test` sentetik paket ve istirap gidiş-dönüşünü doğrular;
PC'de `scripts/ui_tool.py list|extract|istirap|info|uif` (`uif`: ağacı 1264 biçimine göre yürür, sapma noktasında hex bağlamı; 1.298'in 174 UIF'inde tam tüketim doğrulandı).

**2369 UIF biçimi:** 1264 başlığındaki `int16` "idk" alanı aslında düğüm sürümüdür ve her düğüm
kendi değerini taşır: 0 = eski (string'de satır aralığı yok; ISTIRAP `re_login_intro`), 1 = 1264 (1.298
verisinin tamamı), 2 = 2xxx (kapanış sesinden sonra 2 ek bayt; `co_tooltip.uif`), 3 = 2369 (3 ek bayt
`01 00 00`; `el_login_intro_us.uif`, `re_messagebox.uif`). `CN3UIBase::Load` bu alanı `m_sNodeVersion`
olarak saklar (`N3UIExtraBytesForNodeVersion`); `CN3UIString` satır aralığını sürüm 0'da okumaz; ID ve doku
adlarının sonundaki `\0`/boşluk atılır (`N3TrimTrailingJunk`). Doğrulama: `ui_tool.py uif` (1.298'in 174
UIF'i + 2369 `co_tooltip` fikstürü tam tüketim), `uifall <UI>` paketteki 892 UIF'i tarar. ISTIRAP giriş
arayüzü (`Group_Login`, `Group_ServerList_01`, `btn_remember`…) için `CUILogIn_1298::Load` ID takma adları
ve tür/alt dizge sezgisi kullanır, bulamazsa grup çocuklarını Log.txt'ye döker; giriş denetimleri yine
yoksa `CGameProcLogIn_1298` paketteki `el_login_intro_us.uif` / `ka_login_intro_us.uif` dosyasına döner.
Mesaj kutusu arayüzü (`re_messagebox.uif`) yüklenemezse `CUIMessageBoxManager` çökmek yerine sistem
mesaj kutusunu gösterir.

**Eksik arayüz bileşenleri (yer tutucu):** 2369 UIF'lerinde 1.298'in beklediği çocuk ID'lerinin bir kısmı
yok ya da gruplar içinde (`CGameProcMain::InitUI` → `CUIKnights::Load` SIGSEGV v144). `GetChildByID` önce
doğrudan, sonra özyinelemeli (harf duyarsız) arar; yine bulunamazsa `N3_VERIFY_UI_COMPONENT` görünmez,
ebeveynsiz bir yer tutucu denetim üretir ve Log.txt'ye `UI eksik bileşen (yer tutucu): <uif> -> <ifade>`
yazar. Böylece arayüz yüklemeleri çökmez; kalıcı çözüm için bu satırlardan 2369 ID takma adları eklenir.
Oyun öncesi süreçlerde (karakter seçimi) sunucunun gönderdiği oyun içi paketler (`WIZ_QUEST` 0x64,
`WIZ_WEIGHT_CHANGE` 0x54) "yutuldu" diye günlüklenir; bilgiler `WIZ_MYINFO` ile yeniden gelir.

**3D karakter önizlemesi / Item klasörü:** 2369 `UPC_DefaultLooks.tbl` parça adları `\item\upc_el_rm_face00.n3cpart`
biçiminde (başı `\`, `face00`/`hair00`); temel yola eklenince oluşan `//` `KoResolvePath`/`KoVfsResolve`'da
sadeleştirilir. Item klasörü de UI gibi paketliyse (`item/item.hdr` + `item.src`, ya da `item.hdr` kökte,
ya da klasördeki herhangi bir `*.hdr/*.src` çifti) VFS aynı dizin biçimiyle açar ve `item_cache/` altına
çıkarır (`ko_vfs_test` sentetik item paketi). `re_charactercreate.uif`'te `area_character` olmadığından
önizleme dikdörtgeni varsayılan olarak orta panel ile sınıf sütunu arasına (%62–80 × %12–86) konur.

**2369 harita dosyaları (GTD/OPD):** ISTIRAP istemcisinde `Zones\*.gtd` ve `*.opd/.opdext` başlığı
`[int32 adUzunluk][ad: tablo akış XOR'u 0x0816/0x6081/0x1608][int32 sürüm]` (1264: `[sürüm][adUzunluk][ad]`);
gövde 1264 ile aynı, GTD'de çim özniteliğinden sonra 4096 baytlık ek blok var. `CN3Terrain` ve `CN3ShapeMgr`
önce 1264, sonra 2369 başlığı (`CN3BaseFileAccess::ReadHeader2369`), sonra 1098 dener. Su verisi (2369,
`data/2369-samples` dalındaki `moradon.gtd` ile çözüldü): ışık haritası sayısından sonra iki bölüm, her biri
`[u32 7][7 bayt imza][int sürüm=1]` ile başlar; nehir gövdesi 1264 ile aynı (Moradon'da 0), havuz gövdesi 1264 +
`fWave` her zaman + `iIC`'den sonra `[int][ikinci doku adı][80 bayt]` (`CN3Pond::Load(..., b2369)`); dosya
tam tüketilir. Uymazsa su atlanır ve ilk 96 bayt dökülür. OPD: gövde 1264 ile aynı, ancak nesne kayıtları
arasına ve dosya sonuna `[u32 7][7 bayt imza]` blokları eklenmiş (moradon.opd 4257. kayıt öncesi); tür değeri
0x100'den küçükse blok atlanır. Yarım yüklemede `CN3ShapeMgr::Tick` hücre dizinlerini sınırlar, zemin
yüklenemezse `CN3Terrain::Tick/Render` erken döner (v146/v147 SIGSEGV'leri). 2369 `Zones.tbl` 28 sütun:
9. sütun HDR gökyüzü ve sondaki 3 sütun atlanır (`KoTableSchemas` elle eşleme; satır 210 dökümünden).
`d3d9gles` `D3DXCreateTextureFromFileEx` artık BMP (24/32) ve TGA (2/3/10/11) okur (`terrain_base.bmp`,
`sky\*.bmp`, `phases.tga`); `CN3Texture` yolu `KoResolvePath` ile çözer. `InitZone` bölge dosya adlarını,
`CN3Chr::PartSet` ve `CPlayerBase::InitChr` bozuk (yazdırılamayan) dosya adlarını tablo ID'siyle günlükler;
`CGameProcCharacterSelect` yüklediği UIF'i günlükler.

**2369 oyuna giriş (WIZ_GAMESTART):** sunucu (`CUser::HandlePacket`) karakter seçildikten sonra oyun başlayana dek yalnız
belirli opcode'ları kabul eder (GAMESTART, REGENE, REQ_USERIN/NPCIN, ZONE_CHANGE, QUEST, SPEEDHACK_CHECK, DATASAVE,
KNIGHTS/FRIEND_PROCESS, …); `InitZone`'daki `CommandToggleMoveContinous` → `WIZ_MOVE` bu aşamada bağlantıyı
kestiriyordu (v148 "Disconnected" 11 sn). `CGameProcedure::SendAllowed` 2369'da Main'e geçişten `WIZ_GAMESTART 2`
gönderilene kadar izinsiz paketleri atlar (günlük: "oyun başlamadan gönderilemez"); Main'e geçişte ilk 80 gönderim ve
ilk 60 alım `Send:`/`Recv:` satırlarıyla yazılır. Akış: `WIZ_SEL_CHAR` → bölge → `0D 01 <ad>` → sunucunun tek baytlık
`0x0d` yanıtı → `0D 02 <ad>` → oyun içi. `CGameProcMain::Init`'teki `_findfirst` ön yüklemeleri compat'in `-1`
dönüşünü başarı sayıp çöp adlarla (`\chr\???o`, `\item\???o`) yükleme yapıyordu; düzeltildi. Yardım penceresi
(F10) açılışta kapalı. Karakter seçimi 2369'da UIs tablosunun 121. sütunu `szCharSelect`
(`ui\re_characterselect.uif`) ile yüklenir, olmazsa eski `ka_/el_CharacterSelect`.

**2369 .n3anim (DES katmanı):** ISTIRAP animasyon dosyaları `u32 kayıtSayısı` + her kayıt için `u32 uzunluk` + tablo DES
katmanı parçası (16 bayt sabit başlık + u32 BE uzunluk + 8'lik bloklar, **iç XOR yok**). Çözülen parça
`[u16 9][int yer tutucu (15.0f)][1.298 alanları][u32 ad uzunluğu][ad]`. `CN3AnimControl::Load` sabit başlıktan
tanıyıp `KoTableLayer2DecryptDesOnly` ile çözer (`ko_anim_test`: `platform/tests/data/2369/upc_el_ba.n3anim`
155 kayıt, 1.298 kopyasıyla ad/kare karşılaştırması). `.n3joint`/`.n3chr` düz. T duruşunun sebebi buydu.
Paketler: `fx/fx.hdr` kayıtları alt yollu (`billboard\x\y.dxt`); `KoVfsResolve` istenen yolun üst klasörlerini
(4 seviye) paket adayı olarak dener ve göreli adla arar (`ko_vfs_test` fx fikstürü). Yüklenemeyen kaynaklar
(`skillicon_55_2062.dxt` gibi pakette olmayanlar) `CN3Mng` içinde "başarısız" olarak önbelleğe alınır, her karede
yeniden denenmez. 2369 sunucusu `WIZ_ATTACK` sonunda bir `u8` ve `WIZ_MAGIC_PROCESS`'te 7. `sData` okur; gönderimler
2369'da buna göre uzatılır. Oyun içinde tanınmayan opcode'lar (0x83, 0xE9 XSafe/panel, 0x6C, 0xDB, 0xC4, 0xB9)
opcode başına bir kez günlüklenir.

**2369 NPC görünümü:** `GetNpcInfo` varsayılan düzeninde ad yoktur; `NPC_Looks.tbl` resim ID'si (`pid`) ile
anahtarlıdır (1.298'de de `100 = mob_kecoon` gibi), `protoID` sunucunun K_NPC kimliğidir. `MsgRecv_NPCIn` 2369'da
modeli `pid` ile arar (bulunamazsa `protoID`), eksik satırları bir kez günlükler.

**Türkçe metin (kod sayfası):** ISTIRAP sunucusu ve verisi Windows-1254 (tek bayt) kullanır; 1.298 kodu her yüksek
bitli baytı CP949 çift baytlı karakterin ilk baytı sayıyordu ("Geçici" → "Ge??ci"). `N3Base/KoText.h`:
`KoSetTextCodePage(1254|949)` (2369'da 1254, `Option.ini [Text] CodePage` ile değişir), `KoTextCharLen`,
1254↔Unicode tabloları. Yazı tipi çözücüsü (`DFont_ft`), satır kırma (`CN3UIString`, `CUIChat`), düzenleme
kutusu imleci ve klavyeden UTF-8 → 1254 dönüşümü buna uyar. Glifler yazı tipinden gelir; `fonts/default.ttf`
Türkçe karakter içermiyorsa `/system/fonts/Roboto` kullanılır.

Sunucudan gelen sohbet/duyuru dizeleri UTF-8 gelir (`47 65 ef bf bd 69 63 69` = "Ge\uFFFDici"); `ByteBuffer`
dize süzgeci (`s_pfnStringFilter`) 1254 kipinde geçerli UTF-8'i 1254'e çevirir (`KoTextNormalizeIncoming`,
U+FFFD → `?`). Yer tutucu üretilen her UIF için gerçek çocuk ağacı bir kez `UI ağacı (<uif>): id(tür)…` olarak
yazılır (2369 ID eşlemesi için). `Old format DXT` uyarısı kısılır. `d3d9gles` BMP 8 bit paletli dosyaları da okur
(`sundisk.bmp`). Varsayılan kamera hassasiyeti `[Mobile] CameraSens` 170.

**Ses:** istemciyle gelen `Option.ini` `[Sound] Bgm=0 Effect=0` olduğundan `s_SndMgr.Init()` hiç çağrılmıyor,
OpenAL bağlamı olmadan `alListener*` her kare A004 (`AL_INVALID_OPERATION`) üretiyordu. Dinleyici çağrıları
ses kapalıyken atlanır; telefonda ilk açılışta (`[Mobile] SoundInit` yoksa) ses açılıp Option.ini'ye yazılır,
sonrasında oyun içi seçenek geçerlidir. Başlangıçta `Seçenekler: ses bgm=… efekt=…` günlüklenir.

`ko_proto_test` (CTest) sunucu kaynağındaki `Packet <<` sırasını taklit eden paketlerle bu ayrıştırıcıları
ve gönderilen paket yapıcılarını doğrular. Sonraki aşamalar: savaş/eşya/yükseltme, klan/kral/PUS,
2369 arayüzü, 2369 veri uyumluluğu (bölge numaraları, `UI` klasörü, NPC ad tablosu).

- 2369 arayüz uyarlamaları: `re_warp.istirap` (liste yok) → `str_warp0..8` satırları + `img_select`
  seçim resmi, satıra dokunma seçer, çift dokunma/`Btn_Ok` ışınlar; `re_characterselect.uif`
  → `btn_left/btn_right/btn_exit/btn_back` takma adları, `btn_start`/`btn_create` Enter ile aynı yolu
  izler, `str_id/str_lev/str_job` ayrı yazılır. NPC adları `Data\NPC_us.tbl` (DTDDDDD, K_NPC kimliği →
  ad) ile gösterilir; görünüm (`NPC_Looks`) Korece adı yalnız yedektir.

### Oyun verisini telefona kurma

Launcher oyun verisini arar; yoksa "Veriyi indir ve kur" (varsayılan adres) ya da "Veriyi onar /
yeniden kur" menüsünden zip seçimi sunar:

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

### Başsız zemin testi (2369 arazi hataları için)

`KO_TERRAIN_TEST="bölge,x,z[,uzaklık,yükseklik,açı°]"` ile istemci sunucusuz yalnız bölge dosyalarını
(gtd/tct/tlt/gtt/opd) yükler, bölge ışığını (.glo) ve öğlen gök ışığını kurar, kamerayı (x,z)
noktasına bakacak şekilde yerleştirir ve `KO_MAX_FRAMES` kare çizip `KO_SCREENSHOT` dosyasına
kaydeder. `KO_TERRAIN_TEST_NOLIGHT=1` aydınlatmayı kapatır, `KO_TERRAIN_TEST_MANUALCAM=1` serbest
kamera kullanır. Örnek (2369 Moradon dosyaları 1.298 veri dizininde `Zones/moradon_xmas.*` adıyla):

```
SDL_VIDEODRIVER=offscreen EGL_PLATFORM=surfaceless LIBGL_ALWAYS_SOFTWARE=1 \
KO_CLIENT_DIR=/veri KO_TERRAIN_TEST="210,709,426,25,12,0" KO_MAX_FRAMES=6 KO_SCREENSHOT=/tmp/t.ppm build/KnightOnLine
```

Log.txt'ye `CN3Terrain: döşeme dokuları: N döşeme, M gtt dosyası, K dosyası eksik döşeme, F okunamayan`
ve `CN3Pond: havuz …` satırları yazılır; eksik `.gtt` dosyaları adıyla listelenir (o döşemeler beyaz
çizilir).

### Tanılama ve performans seçenekleri

- Launcher'da **Hata raporu gönder**: `Log.txt`, `gpu.txt` (GL_VENDOR/RENDERER/VERSION, S3TC, sınırlar,
  GL_EXTENSIONS), `Server.ini`, `Option.ini`, son logcat (yalnız uygulamanın kayıtları) ve cihaz
  bilgisi tek zip'te, Android paylaşım menüsüyle.
- `Log.txt`'ye düşenler: `[gpu] ...` aygıt bilgisi, `[d3d9gles] ...` (GL hataları `glGetError` ile doku
  yüklemede ve her kare sonunda; desteklenmeyen doku formatı; FBO/ölçek), `[dosya yok] <yol>`
  (okunamayan her dosya, yol başına bir kez).
- `Option.ini [Mobile]`: `ShowFps=1` (sol üstte FPS/çözünürlük/ölçek), `RenderScale=50|75|100`
  (3D sahne mantıksal çözünürlüğün yüzdesi kadar FBO'ya çizilir, UI keskinliği değişmez),
  `[Shadow] Use`, `[Texture] LOD_*` (düşük doku kalitesi). Hepsi launcher Ayarlar'da.
  Ortam: `KO_SHOW_FPS`, `KO_RENDER_SCALE`.

### Dokunmatik kontroller (oyun içi)

- Sol yarı: parmağın bastığı yerde beliren joystick. Varsayılan **dön ve koş** (`JoyTurnAndRun=1`):
  joystick yönü kameraya göre dünya yönüne çevrilir, karakter anında o yöne döner ve ileri koşar
  (oyunun 60°/sn A/D dönüşü yerine). `JoyTurnAndRun=0`: eski W/S ileri-geri, A/D dönüş. Ölü bölge
  `JoyDeadZone`. Serbest alanda sürükleme: kamera (sağ tuş sürüklemesi, hız `CameraSens`, varsayılan
  260 %); iki parmak: yakınlaştırma.
- Tek dokunuş = sol tık (yürü / hedef seç / arayüz). **Seçili dost NPC'ye veya seçili kapı/bind
  nesnesine ikinci dokunuş = sağ tık** (konuş, dükkân, kapı olayı; uzaksa oyunun "çok uzak" iletisi
  çıkar). 3D dünyada çift dokunuş = oyunun çift tıkı = hedefe saldır; uzun basış (`LongPressMs`,
  varsayılan 450 ms) = sağ tık: NPC ile konuş, ceset/kutu aç, kapı/nesne olayı.
- Arayüz pencereleri üstünde (çanta, beceri, skill bar, ticaret, depo, pazar, ekipman):
  sürükleme = sol tuş sürüklemesi (ikon taşıma, pencere taşıma, kaydırma çubuğu; LBCLICK ve ilk
  LBDOWN ikonun üstünde iki kare tutulur); uzun basış = ikonu tut, parmak kıpırdamadan kalkarsa
  **yapışkan taşıma** (ipucu çıkar, sonraki dokunuş ikonu oraya bırakır); çift dokunuş = sağ tık
  (eşya kullan / giy, beceri kullan). Sayılabilir eşyalar ticaret/depo/pazarda bırakılınca oyunun
  kendi adet penceresi açılır (klavye otomatik gelir).
- Sağ alt: SALDIR (R), HEDEF (Z), PARTI (X), DOST (V), NPC (B), 2×4 beceri ızgarası (1-8), üstünde F1..F8
  sayfa düğmeleri (seçili sayfa vurgulu), HP/MP pot düğmeleri (`PotHpSlot`/`PotMpSlot`). Joystick üstünde
  OTO (E, sürekli yürüme). Sağ üst: kamera kümesi (F9, 180°, +/-, T koş). Alt çubuk: Çanta (I),
  Karakter (U), Beceri (K), Otur (C), Harita (M), Al (F), Sohbet (Enter → klavye açılır, Enter
  gönderir), Menü (H komut listesi), Yardım (F10), Kapat (ESC).
- `ko_touch_test` (ctest): parmak olayı → kaplama → CLocalInput zincirini başsız doğrular.
- Düğme boyutu fiziksel DPI'dan hesaplanır (en az ~48dp). `UiScale=125|150` oyun içi mantıksal
  yüksekliği 768/ölçek yapar (arayüz pencereleri ve yazılar büyür); giriş/karakter ekranları 768'de
  kalır. Hepsi launcher Ayarlar'da.

### Ekran klavyesi

SDL 2.30 metin girişini açılışta "aktif" bırakır (klavye göstermeden); bu yüzden `SDL_IsTextInputActive()`
ile koşullanan `SDL_StartTextInput` hiç çalışmıyor ve telefonda klavye açılmıyordu. Pencere
oluşturulunca bir kez `SDL_StopTextInput()` çağrılır; bir edit odak kazandığında koşulsuz
`SDL_SetTextInputRect` (kutunun ekrandaki yeri) + `SDL_StartTextInput` yapılır, odak gidince 15 kare
sonra kapatılır. `SDL_HINT_ENABLE_SCREEN_KEYBOARD=1`, oyun etkinliğinde `windowSoftInputMode=adjustPan`.

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
   2369: ISTIRAP sunucusu (86.105.4.195:15200/15301) ile aynı akış; veri `client2369.zip`.
3. Dokunmatik oyun arayüzü: sanal joystick, hedef seçme, beceri çubuğu, sohbet için sanal klavye.
4. Performans: shader/uniform önbelleği, doku belleği (CPU gölge kopyalarını serbest bırakma), ETC2/ASTC doku ön dönüşümü.
5. Ses testleri, BMP/JPG doku yükleme, iOS (aynı kod + SDL iOS projesi).
