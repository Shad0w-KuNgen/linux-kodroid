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

## v169 (v168 geri bildirimi)
1. **Stat puanı (WIZ_POINT_CHANGE 0x28)**: `re_page_state.istirap` düğmeleri birden çok adla aranır
   (`Btn_Strength…` / `Btn_Str`, `Btn_Sta`, `Btn_Dex`, `Btn_Magic`/`Btn_Int`, `Btn_Cha`); bulunan/bulunamayan adlar Log'a
   (`Stat düğmeleri (2369)`). İstek: `0x28 | u8 tür` (1 STR, 2 STA, 3 DEX, 4 INT, 5 CHA). Yanıt 2369 düzeni:
   `u8 tür, u16 yeniStat, i16 maxHP, i16 maxMP, u16 vuruş, u32 maxAğırlık, u16 HP, u16 MP` → `MsgRecv_MyInfo_PointChange`
   2369 dalı; kalan puan 1 azaltılır, düğmeler puan 0 olunca gizlenir (`UpdateBonusPointAndButtons`). Log:
   `WIZ_POINT_CHANGE (2369) tür…`. Beceri puanı (`WIZ_SKILLPT_CHANGE`) için beceri ağacı UIF düğme adları henüz
   bağlanmadı (Log'daki `UI ağacı` dökümünden `btn_…` adları alınıp `CUISkillTreeDlg` içinde aynı yöntemle bağlanacak).
2. **Kaplama kaybolması**: `GIZLE` dokunuşu düğme döngüsünün içinde `Layout()` çağırıp `m_buttons` vektörünü yeniden
   kuruyordu → döngüdeki referans geçersiz (bellek bozulması; düğmeler/etiket rastgele kayboluyordu). Artık
   `m_relayout` bayrağıyla döngü bittikten sonra yapılır. Ek güvence: kaplama çizilmeyince bir kez Log
   (`Dokunmatik: kaplama çizilmiyor (etkin, oyunda, aktif süreç)`), yeniden çizilince `kaplama yeniden çiziliyor`,
   GİZLE/GÖSTER durumu Log'a yazılır. Yeni günlükte bunlar yoksa ve düğmeler yine kayboluyorsa neden çizim değil,
   giriş (dokunuş) tarafındadır. Bilgi satırı fontları yalnız metin değişince yeniden üretilir (doku sızıntısı/yavaşlama önlemi).
3. **Beceri yuvası sürükleme**: v168'de 4 sn içinde 8 yuva boşalmıştı. Artık yuvayı boşaltmak/değiştirmek için
   ≥300 ms basılı tutup parmağı gerçekten kaydırmak ve 2.4r dışına bırakmak gerekir; kısa kaymalar beceri kullanır.
4. **Klan penceresi** (`UIClanWindow2369`): açılınca `WIZ_KNIGHTS_PROCESS | 13` ister; yanıt
   `u8 sonuç, u16 boyut, u16 2, u16 MAX, str16 duyuru, u16 sayı, üye×(str16 ad, u8 fame, u8 0, u8 seviye, u16 sınıf,
   u8 çevrimiçi, str16 memo, u32 saat)` → 5 satır/sayfa (`grp_member_list_0..4`), satır seçimi `btn_selected*`.
   Düğmeler: refresh (yeniden iste), whisper (sohbete `@ad `), Clan_party (parti daveti), remove/appoint/admit
   (sunucu `MsgSend_Knights…`), notice grubu (`edit_notice` → `WIZ_KNIGHTS_PROCESS | 80 | str16`), memo/purge iptal.
   `btn_bank`/`btn_transfer` Log'a "bağlı değil" yazar (XSafe CLANBANK düzeni gerekir).
   `GameProcMain::MsgRecv_Knights` 2369'da alt kod 13'ü pencereye yönlendirir.
5. **Bölge müziği**: `Zones.tbl` sütunu boşsa `Snd` klasörü taranır: `Snd/<bölgeId>_<ad>.ogg|mp3|wav`
   (örn. `Snd/21_moradon.ogg`, `Snd/1_karus.ogg`); Log `Bölge müziği örnekleri` artık bulunan dosya adını yazar.
   Yerel Claude: istemci `Snd` klasörüne bu adlarla dosya koyması yeterli.
6. **Joystick ayarları**: `Option.ini` [Mobile] anahtarları okunuyor: `CameraSens` (25–600, varsayılan 260),
   `JoyDeadZone` (5–60, 22), `JoyRotateSpeed` (30–400 °/sn, 220), `JoyTurnAndRun` (0/1), `LongPressMs`, `PotHpSlot`,
   `PotMpSlot`, `UiScale`, `RenderScale`. Launcher Settings ve oyun içi menü UI'si bu anahtarları yazmalı (kolay iş, yerel Claude).

## v170 devir (araştırma tamam, kod yazılmadı — yerel Claude uygular)
Kaynaklar okundu, kök nedenler ve paket/dosya düzenleri aşağıda. Her madde doğrudan uygulanabilir.

### 1. NPC'ye tıklanmıyor → kaplamada "NPC AÇ" düğmesi
- Hedef seçimi çalışıyor (Log `SALDIR: hedef 11046 (Duke) … saldırılabilir 0`), konuşma ise yalnız uzun basış /
  ikinci dokunuşla tetikleniyor (`GameProcMain.cpp` ~8484 `m_iTouchTalkNpcID` bloğu; Tick'te ~513 menzile girince `MsgSend_NPCEvent`).
- Yapılacak: `CGameProcMain`'e public `bool TouchInteractTarget()` ekle: hedef `s_pOPMgr->CharacterGetByID(m_iIDTarget,true)`;
  `PlayerType()==PLAYER_NPC && !IsHostileTarget` ise: `m_pShapeExtraRef` varsa mesafe `(Radius+shape->Radius)*2` içinde
  `MsgSend_ObjectEvent(shape->m_iEventID, IDNumber())`; değilse mesafe `(Radius+Radius)*3` içindeyse
  `ActionMove(PSM_STOP); RotateTo; MsgSend_NPCEvent(id); m_pUITransactionDlg->m_iNpcID=id`; uzaksa
  `m_iTouchTalkNpcID=id; CommandMove(MD_FORWARD,true); SetMoveTargetPos(pos)`. (8484–8499'daki kodun aynısı.)
- Kaplama (`KoTouchOverlay.cpp`): oyuncu menüsü (`LayoutPlayerMenu/HitPlayerMenu`, satır ~1162) örneğiyle tek satırlık
  "NPC AC" kutusu; `pMain->TouchTargetIsNpc()` doğruysa hedef sütununun solunda çiz, `OnFingerDown`'da HitPlayerMenu'den
  önce test et → `TouchInteractTarget()`.

### 2. 2. karakter oluştururken kilitlenme
- `UICharacterSelect.cpp` ~198: `btn_start`/`btn_create` → `ProcessOnReturn()` çağırıyor. Bu fonksiyon seçim DEĞİL;
  `m_bReceivedCharacterSelect` sonrası Main'e geçiş. Yanlış bağlama → düğme ya hiçbir şey yapmaz ya da seçimsiz Main'e atlar.
  Doğrusu: `s_iChrSelectIndex` = (başlat: dolu yuva, oluştur: boş yuva; önce m_eCurPos'un yuvası) ve
  `CGameProcedure::s_pProcCharacterSelect->CharacterSelectOrCreate()` (kaplamadaki BASLA/YENI KARAKTER zaten bunu yapıyor, 
  `KoTouchOverlay.cpp` ~183–204). Yalnız `m_eCurProcess == PROCESS_PRESELECT` iken.
- Sunucu 4 karaktere izin verir (`bCharIndex > 3` hata), istemci arayüzü 3 yuva. ALLCHAR yanıtı: `u8 1, u8 sonuç, 4×(str16 ad,
  u8 ırk, u16 sınıf, u8 seviye, u8 yüz, u32 saç, u8 bölge, 8×(u32 eşya,u16 dayanıklılık))` — `ParseAllCharInfo2369` doğru.
  NEW_CHAR yanıtı `u8 sonuç` (0 başarı). Oluşturma başarılıysa sunucu yeni karaktere `giveGenieHour` saat Genie verir.
- Kilitlenmenin logunu iste: `WIZ_NEW_CHAR gönderildi`, `WIZ_NEW_CHAR yanıtı`, `ProcActiveSet:` satırları.

### 3. Pot ikonu yok, yalnız sayı
- Yuvadaki eşya (pot) ikonu `UIHotKeyDlg.cpp` 897/949: `UI\skillicon_{XX}_{N}.dxt` (eşyanın `dwEffectID1` becerisinden).
  2369'da bu dosya yok → doku yüklenmiyor. Çözüm: dosya yoksa (`KoResolvePath`+stat) `spItem->szIconFN` (eşyanın kendi
  `UI\ItemIcon_…dxt` ikonu) kullan. `DrawSkillIcons` (kaplama) o zaman çizer. İsteğe bağlı: halkada adet
  (`m_pUIInventory->GetCountInInvByID(pSkill->dwExhaustItem)`) küçük yazı.

### 4. Karakter seçim sahnesi (orijinal 24xx görünümü)
- Bulgu: `ChrSelect\el_elmo_chairs.n3shape` / `ka_cave.n3shape` konumu (0,1.69,0); 2369 kamera dosyaları ise dünya
  koordinatında: `ChrSelect\Data\el_center_left_camera.n3camera` göz (200.22,0.85,132.09) → bakış (184.3,2.31,132.13);
  sol uç göz (224.05,0.65,120.92) → bakış (220.03,2.31,110.5); sağ (`el_center_right`) göz (224.82,0.66,137.16) →
  bakış (219.02,2.31,147.53). Karus: merkez göz (128.03,0.53,85.93) → bakış (128.07,4.09,105.85); sol (113.55,1.11,61.0)→(103.11,2.44,65.04);
  sağ (136.58,1.22,66.65)→(147.89,2.0,71.04). `*_lording_camera` = 6 sn giriş süzülmesi (180 kare). Bu koordinatlar
  **Zones.tbl 10000 `Zones\elmorad_intro.gtd` / 10001 `Zones\karus_intro.gtd`** bölgesine ait (24xx karakter seçimi bir arazi
  bölgesidir, n3shape değil). Dosya düzeni (CN3Camera::Load'dan farklı!): `u32 adUz, ad, 3f konum, 4f quat, 3f, 
  anahtar{i32 sayı, i32 tür(0=Vector3), f32 hız, sayı×3f} ×3 (konum, BAKIŞ, ölçek), 3f at, __CameraData`. Yani 2. anahtar
  dizisi bakış noktasının animasyonudur; kendi ayrıştırıcını yaz (python ile doğrulandı).
- Uygulama: `GameProcCharacterSelect::Init` 2369 dalında bölge dosyası varsa `s_pWorldMgr->InitWorld(10000/10001)` +
  `SetGameTimeWithSky(…,12,0)`; kamera dosyasından göz/bakış; karakterleri kameraların bakış noktasının XZ'sine
  `GetHeightWithTerrain` yüksekliğiyle koy, kameraya döndür; Render'da `RenderSky/RenderTerrain/RenderShape` sonra karakterler
  (örnek: `main_sdl.cpp` 623–733 KO_TERRAIN_TEST bloğu, ışık için `CLightMgr`+`LoadZoneLight(pZ->szLightObjFN)`).
  Sol/sağ geçişte `*_center_left_camera` anahtarlarını oynat (`RotateLeft` yerine). Masaüstü test verisinde
  `Zones/elmorad_intro.*` (1.298) + 2369 kameralar var → başsız test yapılabilir.
- **Yerel Claude'dan istenecek dosyalar**: telefondaki `Zones\elmorad_intro.*`, `Zones\karus_intro.*` (gtd/tct/tlt/opd/opdext/gev/ens/glo/gmd),
  `DTex\*intro*.gtt`, `Misc\Sky\…` ve 2369 `Zones.tbl` 10000/10001 satırlarının dökümü (Log'da `Zones.tbl 101 satır`).

### 5. Eksik eşyalar ("yusuf"a eklenenler görünmüyor)
- Tablolar tam yükleniyor (Item_Ext_0..23, Log). Eşya ikon/mesh dosyaları `Item\item.src` (613 MB) ve `UI\ui.src` paketlerinden
  `compat/win32/ko_vfs.cpp` ile çıkarılıyor (`item_cache/`). Görünmeyen eşyalar için iki olasılık: (a) Item_Org/Ext satırı yok
  (`MyInfo - Inv - Unknown Item <id>` Log satırı), (b) `MakeResrcFileNameForUPC` (`GameBase.cpp` 629–668) ürettiği
  `UI\ItemIcon_A_BBBB_CC_D.dxt` / `Item\A_BBBB_CC_D.n3cpart|n3cplug` paket dizininde yok.
- Yapılacak: `KoUiSlots2369.cpp`'ye `KoAuditItem(id, pItem, pItemExt, szIconFN, szResrcFN, yer)` ekle; `GameProcMain.cpp`
  MYINFO yuva döngüsü (~2278) ve çanta döngüsü (~2370), `MsgRecv_UserLookChange` (~4004) çağırsın; dosya yoksa
  `EŞYA DENETİMİ: <id> '<ad>' ikon yok: … | mesh yok: …` + özet. `ko_vfs.cpp ExtractFromPack` bulunamayan ada bir kez Log.
  Sonra yusuf'un eşya kimliklerini yerel Claude Log'dan okuyup eksik tablo satırı/paket girdisini ekler.

### 6. Genie (oto av) — sunucu protokolü (`GameServer/GenieHandler.cpp`, `GameDefine.h` 4616–4645)
- İstemci→sunucu: `WIZ_GENIE(0x97) | u8 1 | u8 alt`: 2 seçenekleri yükle, 3 kaydet (+100 bayt), **4 başlat, 5 durdur**;
  `0x97 | u8 2 | u8 (1 hareket, 2 dönüş, 3 saldırı, 4 büyü) | normal paket gövdesi` (sunucu MoveProcess/Rotate/Attack/MagicPacket'e
  aynen iletir; zorunlu değil, normal paketler de çalışır).
- Sunucu→istemci: `0x97 | 1 | 2 | 100 bayt` seçenekler; `0x97 | 1 | 4 | u16 1 | u16 saat` başladı; `0x97 | 1 | 5 | u16 1 | u16 saat` durdu;
  `0x97 | 1 | 6 | u16 saat` kalan süre (dakikada bir, yalnız süre>0 iken); `0x97 | 1 | 7 | u16 oyuncuID | u8 açık` bölgeye yayın.
  Ayrıca `XSafe(0xE9) | 0xDB | u8 açık`.
- **DİKKAT**: süre 0 iken ya da `LootandGeniePremium=1` ve premium yokken `4 başlat` gönderilirse sunucu **bağlantıyı keser**
  ("GENİE HACK … Dc Edildi"). Başlat'ı yalnız `1|6` ile saat>0 geldikten sonra gönder; aksi halde yerel oto av (sunucuya bildirmeden).
- Sunucu av mantığı yapmaz; Genie istemci tarafı bottur. Önerilen istemci tasarımı (`KoGenie.cpp`, Tick 300 ms):
  HP% < eşik → `m_pUIHotKeyDlg->DoOperate(m_pMyHotkey[sayfa][PotHpSlot-1])`; hedef yoksa en yakın düşman NPC
  (`m_NPCs`, `IsHostileTarget`, `IsAlive`, başlangıç noktasına ≤ menzil) → `TargetSelect` + `TryStartAttack()` (uzaksa yürür);
  menzildeyken etkin yuvaların becerileri `MsgSend_MagicProcess(hedefID, pSkill)` (`GetCooldown<=0`); hedef ölünce
  `CorpseGetNearstNPC(true, ulus, konum)` → yaklaş → `MsgSend_RequestItemBundleOpen` → açılınca her eşya için
  `WIZ_ITEM_GET | u32 kutu | u32 eşya | u16 yuva` (`UIDroppedItemDlg.cpp` 480–491). Kaplamada GENIE düğmesi + panel
  (BAŞLAT/DURDUR, HP%, MP%, menzil, yağma, yuva seçimi); seçenekler `Option.ini [Genie]`.
- Arayüz: `ui.hdr` paketinde `re_genie.uif` (66936 B) ve `re_genie_sub.uif` var (telefonda `ISTIRAP\re_genie.istirap` da var);
  `CUIGeneric2369` ile yükleyip `UI ağacı` dökümünü Log'a yazdır, düğme adlarını sonra bağla.
- `WIZ_GENIE` şu an `GameProcMain.cpp` ~1199'da sessizce yutuluyor; oradan çıkarıp işleyiciye yönlendir.

## Açık işler (kolaylar — yerel Claude)
- Metin/etiket düzeltmeleri (kaplama etiketleri ASCII: PARTI, FISILDA…; Türkçe karakterli etiketler için fontta ğ/ş var).
- Klan sayfası metinleri ("Membe", "LevelClas") kesiliyor: yazı tipi genişliği; `co_page_clan.uif` yerine
  `ISTIRAP/re_clan_window.istirap` denenebilir (genel pencere: `CUIGeneric2369`).
- Tablo adı karşılıkları (NPC_us/mob_us), küçük ikon/yol eşlemeleri (`itemicon_*` adları ui.hdr ile aynı), log temizliği.
- `re_minimenu.uif` (orijinal oyuncu menüsü UIF'i) ile kaplama menüsünü değiştirmek (ID'leri Log'a dökerek).
- PUS satın alma yanıtının doğrulanması (CASHCHANGE sonrası `txt_cash`).
- Beceri puanı (`WIZ_SKILLPT_CHANGE | u8 tür`) düğmelerini beceri ağacı UIF'inde bağlamak (stat düğmeleriyle aynı yöntem).
- Joystick ayar ekranı (launcher Settings + oyun içi menü; `Option.ini` [Mobile] anahtarları, bkz. v169/6).
- Klan bankası/transfer (XSafe CLANBANK) ve klan davet (`admit` hedef oyuncu seçimi) cihazda doğrulama.

## Test komutları
- Masaüstü: `cmake --build mobile/build && (cd mobile/build && ctest)`; başsız arazi:
  `KO_CLIENT_DIR=<veri> KO_TOUCH=1 KO_TERRAIN_TEST="210,709,426,25,12,0" KO_MAX_FRAMES=6 KO_SCREENSHOT=/tmp/t.ppm build/KnightOnLine`
- Tam 2369 verisi: GitHub release `veri-2369` (iki zip aynı klasöre).
