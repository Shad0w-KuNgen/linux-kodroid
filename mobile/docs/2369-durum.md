# 2369 (ISTIRAP) uyarlaması — durum ve devir notları

Bu dosya yerel Claude'un (Windows'ta derleyip telefonda test eden) kaldığı yerden devam edebilmesi için
tutulur. Her büyük değişiklik commit mesajında ve burada kısaca açıklanır. Ayrıntılı günlük: `mobile/README.md`
"Sunucu protokolü: 1.298 ve 2369" bölümü (v144 … v165 maddeleri).

## Mimari özet
- İstemci OpenKO 1.298 (`mobile/openko`), SDL2 + d3d9gles (GLES3). 2369 farkları `KoProto::Is2369()` dallarıyla.
- Paket düzenleri sunucu kaynağıyla (`1 - Login & Game Source`) birebir karşılaştırılarak yazılır; opcode'lar aynı.
- 2369 UIF'leri `UIs_us.tbl` (347 sütun) üzerinden; ad tabanlı yuva eşlemesi `KoUiSlots2369.cpp`. Eksik bileşen → yer tutucu
  (`N3UIPlaceholderMake`), Log'da `UI eksik bileşen` ve `UI ağacı` satırları.
- Dokunmatik kaplama `mobile/platform/KoTouchOverlay.*`: joystick, SALDIR, 1-8 beceri halkaları (+ikonlar), F1-F8 sayfa,
  HP/MP, hedef sütunu, kamera, alt çubuk (CANTA … PUS … KAPAT), GİZLE/GÖSTER, oyuncu menüsü.

## v165'te yapılanlar (zor işler)
1. **HP 34/34**: MYINFO değil, `WIZ_LEVEL_CHANGE` (oyuna girişte sunucu bölgeye yayınlar) 1298 düzeniyle okunuyor,
   i64 tecrübe alanları 4 bayt sanılınca HP exp'in içinden geliyordu. 2369 düzeni: `u16 id, u8 lv, i16 puan, u8 beceri,
   i64 maxExp, i64 exp, i16 maxHp, i16 hp, i16 maxMp, i16 mp, u32 maxAğırlık, u32 ağırlık`. Log: `WIZ_LEVEL_CHANGE (2369)`.
   MYINFO için ham döküm (`WIZ_MYINFO ham`) ve `02 03 04 05` işaretçisi (v164) duruyor; döküm gelince hizayı doğrulayın.
2. **Beceri**: beceri penceresinde bir beceriye dokunmak (sağ tık karşılığı = çift dokunuş) onu geçerli sayfanın ilk boş
   yuvasına koyar (orijinal akış). Yuvalardaki ikonlar artık kaplamanın 1-8 halkalarında çizilir (`DrawSkillIcons`).
   Alan becerisi hedef seçimi sürerken (`m_dwRegionMagicState == 1`) joystick/kamera kilitli (`s_bTouchLockMove`);
   onay 0,6 sn içinde bir kez (nova 4 kez çıkmasın).
3. **Oyuncu menüsü**: seçili dost oyuncuya ikinci dokunuş → PARTI DAVET / TICARET / FISILDA / ARKADAS EKLE / KAPAT
   (`CGameProcMain::TouchMenuAction`, kaplama çizer). Arkadaş ekleme paketi: `WIZ_FRIEND_PROCESS | 3 | u16 id | str8 ad`.
   Düello ve "View Info" yok (sunucuda karşılığı bulunmadı / BottomUserList).
4. **Karakter penceresi**: klan sayfası karakter bilgisinin üstünde açılıyordu (`CUIKnights::SetVisible` erken dönüş);
   sayfalar artık taban düzeyde gizlenir, `btn_clan` (2369) klan sayfasına bağlı, sekme değişimi her zaman çalışır.
5. **Ses**: `Snd/` dosyaları .ogg; tablo .mp3/.wav adı verince var olan uzantı seçilir (`KoResolveAudioPath`), ogg
   `stb_vorbis` ile çözülür (`engine-port/AudioVorbis.cpp`); akış için bellek içi PCM (`OwnedPcm`). Intro müziği
   `Snd\Intro_Sound.mp3` → `.ogg` bulunursa çalar.
6. **Karakter seçimi**: 2369 `ChrSelect\el_elmo_chairs.n3shape` / `ka_cave.n3shape` sahnesi denenir, yoksa 1.298 taht;
   panel sağda, bilgiler dokunmatikte her zaman dolu. Oyun içi bilgi (hasar) kutusu geri geldi: sağ yarıda, kamera
   düğmelerinin altında, hedef sütununun solunda.
7. **PUS**: `CUIPowerUpStore2369` sunucudan XSafe PUS/PusCat/CASHCHANGE listesini alır (v163).

## v168 (v167 kök sorunlar)
1. **HP 0/34 kök nedeni**: `WIZ_ITEM_MOVE (0x1f)`; sunucu `SendItemMove` girişte (`SetUserAbility`) yetenek paketini yayınlar:
   `u8 komut, u8 alt, u16 vuruş, u16 savunma, u32 maxAğırlık, u8 0, u8 0, u16 maxHP, u16 maxMP, 5×i16 stat bonus,
   6×u16 direnç, u32 KC, 7×u16 silah direnci, u32 tamir, i16 HP, i16 MP`. İstemci 1298 düzeniyle okuyup HPMax'ı
   `f6 22 00 00` içinden (34) alıyordu. 2369 dalı eklendi; Log: `WIZ_ITEM_MOVE (2369) yetenek: …`. MYINFO ve LEVEL_CHANGE
   zaten doğruydu; `ko_proto_test` içinde ham MYINFO testi (`TestMyInfoRaw2369`, v167 günlüğünden) 2002/100/1982 doğrular.
   Sıkıştırılmış paketlerin iç opcode'u bir kez günlüğe yazılır.
2. **Ses**: dosya çözümleme sessiz (`KoResolvePath` + stat), eksik dosya adı bir kez günlüğe (`Ses dosyası yok (bir kez)`),
   .mp3/.wav/.ogg hangisi varsa; bölge müziği `Zones.tbl` yakalanan sütunlarından (`KoZoneBgmFile`, Log: `Bölge müziği`),
   bölge değişince değişir; ayak sesleri `Data\move_sound.tbl` (ırk satırı, sütun 0); beceri/mob sesleri sound.tbl kimlikleriyle
   zaten çağrılıyor (dosya bulununca çalar). 279 eksik ses ISTIRAP verisinde de yok.
3. **Beceri yuvaları (kaplama)**: halkaya dokunuş = kullan (tuş kalkışta); basılı tutup dışarı bırak = yuvayı boşalt
   (`CUIHotKeyDlg::ClearSlot`); başka halkaya bırak = yer değiştir (`SwapSlots`). Eşya (pot) yuvaya koymak için
   çantadan sürükleme hedefi hâlâ yok (gizli pencere); HP/MP düğmeleri `HpSlot/MpSlot` ayarıyla yuva tuşuna basar.
4. **Joystick / pencere**: açık iletişim penceresi (çanta, karakter, beceri…) alanındaki dokunuş pencereye gider
   (`IsOverDialogUI`); sohbet/durum çubuğu gibi kalıcı HUD joystick'i engellemez.
5. **Klan**: 2369 `re_clan_window.istirap` genel pencere olarak açılır (klan adı yazılır); üye listesi için 2369
   `WIZ_KNIGHTS_PROCESS` yanıt düzenleri henüz çözülmedi (sunucu `KnightsManager`).
6. **Bilgi/hasar satırları**: büyük kutu gizli; `MsgOutput` iletileri kaplamada beceri kümesinin üstünde, sağa hizalı,
   yarı saydam 6 satır (9 sn sonra solar). Sözcük ortasından bölünme ("he wrong") için bilgi kutusu satır kırma boşluktan.

## Açık işler (kolaylar — yerel Claude)
- Metin/etiket düzeltmeleri (kaplama etiketleri ASCII: PARTI, FISILDA…; Türkçe karakterli etiketler için fontta ğ/ş var).
- Klan sayfası metinleri ("Membe", "LevelClas") kesiliyor: yazı tipi genişliği; `co_page_clan.uif` yerine
  `ISTIRAP/re_clan_window.istirap` denenebilir (genel pencere: `CUIGeneric2369`).
- Tablo adı karşılıkları (NPC_us/mob_us), küçük ikon/yol eşlemeleri (`itemicon_*` adları ui.hdr ile aynı), log temizliği.
- `re_minimenu.uif` (orijinal oyuncu menüsü UIF'i) ile kaplama menüsünü değiştirmek (ID'leri Log'a dökerek).
- PUS satın alma yanıtının doğrulanması (CASHCHANGE sonrası `txt_cash`).

## Test komutları
- Masaüstü: `cmake --build mobile/build && (cd mobile/build && ctest)`; başsız arazi:
  `KO_CLIENT_DIR=<veri> KO_TOUCH=1 KO_TERRAIN_TEST="210,709,426,25,12,0" KO_MAX_FRAMES=6 KO_SCREENSHOT=/tmp/t.ppm build/KnightOnLine`
- Tam 2369 verisi: GitHub release `veri-2369` (iki zip aynı klasöre).
