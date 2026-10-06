// KoTouchOverlay.h — dokunmatik oyun kontrolleri (KO Mobile tarzı düzen).
//
// Oyun kodu klavye/fare bekler; bu katman dokunuşları şu şekilde çevirir:
//   • Sol yarıda parmağın bastığı yerde beliren joystick → W/S (ileri/geri), A/D (dönüş)
//   • Sağ altta büyük SALDIR tuşu (R) + etrafında yay şeklinde 8 beceri yuvası (1-8 kısayolları)
//     + HEDEF tuşu (Z, en yakın düşman)
//   • Sağ üstte kamera kümesi: görüş açısı (F9), 180° dönüş, yakınlaştır/uzaklaştır, koş/yürü (T)
//   • Altta çubuk: Çanta (I), Karakter (U), Beceri (K), Otur (C), Harita (M), Al (F), Sohbet (Enter), Menü (Esc)
//   • Serbest alanda kısa dokunuş → sol tık (yürü / hedef seç / arayüz)
//   • Serbest alanda sürükleme → sağ tuşla sürükleme (kamera döndürme)
//   • İki parmakla açma/kapama → yakınlaştırma
#ifndef KO_TOUCH_OVERLAY_H
#define KO_TOUCH_OVERLAY_H

#include <cstdint>
#include <string>
#include <vector>
#include <deque>

class IDirect3DDevice9;
class CDFont;

class KoTouchOverlay
{
public:
	KoTouchOverlay();
	~KoTouchOverlay();

	void SetEnabled(bool on) { m_enabled = on; }
	bool Enabled() const { return m_enabled; }
	/// Oyun içi olmasa da çiz (test/ekran görüntüsü için)
	void SetForceVisible(bool on) { m_forceVisible = on; }
	/// Sol üstte FPS / çizim çağrısı sayacı (Ayarlar: ShowFps)
	void SetShowFps(bool on) { m_showFps = on; }

	/// Ayarlanabilir davranış (Option.ini [Mobile])
	struct Tuning
	{
		float camSens        = 1.0f;  // kamera sürükleme hızı çarpanı (CameraSens %)
		bool joyTurnAndRun   = false; // joystick: yön = karakterin baktığı yön (kameraya göre) + ileri koşma; false = klasik: ileri/geri + dönüş
		float joyRotateDegPerSec = 220.0f; // klasik kipte tam yatık joystick ile dönüş hızı (oyunun A/D'si 60°/sn)
		float joyDeadZone    = 0.22f; // joystick ölü bölge (yarıçap oranı, JoyDeadZone %)
		uint32_t longPressMs = 450;   // uzun basış = sağ tık (NPC ile konuş / eşya al)
		int hpSlot           = 7;     // HP pot düğmesinin bastığı kısayol yuvası (1..8)
		int mpSlot           = 8;     // MP pot düğmesinin bastığı kısayol yuvası (1..8)
		float minTouchPx     = 0.0f;  // düğmeler için en az dokunma boyutu (mantıksal piksel, ~48dp)
	};
	void SetTuning(const Tuning& t) { m_tuning = t; }
	const Tuning& GetTuning() const { return m_tuning; }

	/// Mantıksal (oyun) çözünürlüğüne göre düzeni kur
	void Layout(int logicalW, int logicalH);

	// Parmak olayları (mantıksal koordinat)
	void OnFingerDown(int64_t id, int x, int y);
	void OnFingerMotion(int64_t id, int x, int y);
	void OnFingerUp(int64_t id, int x, int y);

	/// Her kare: sanal tuş/fare durumunu KoInput'a yazar
	void Update();
	/// Kaplamayı çizer (Present'ten hemen önce çağrılır)
	void Render(IDirect3DDevice9* dev);
	void AddInfoLine(const std::string& text, uint32_t color); // bilgi/hasar iletisi (MsgOutput)

	bool IsInGame() const;
	/// Giriş: sunucu seçme ekranı açık mı? (büyük BAĞLAN düğmesi çizilir; Enter yerine geçer)
	bool IsServerSelect() const;
	/// Karakter oluşturma / karakter seçme ekranları (ırk-sınıf-oluştur ve BAŞLA-YENİ düğmeleri)
	bool IsCharacterCreate() const;
	bool IsCharacterSelect() const;

	/// Test: düğme sayısı ve i. düğmenin sınır kutusu (üst üste binme denetimi)
	size_t ButtonCount() const { return m_buttons.size(); }
	void ButtonBox(size_t i, float* x0, float* y0, float* x1, float* y1, std::string* label) const;

private:
	enum class Action { Key, Yaw180, ZoomIn, ZoomOut, SkillPage, ToggleHide };
	enum class Shape { Circle, Ring, Rect };
	struct Button
	{
		Action action = Action::Key;
		Shape shape   = Shape::Circle;
		int dik       = 0;
		std::string label;
		float cx = 0, cy = 0, r = 0, w = 0, h = 0;
		uint32_t color = 0;
		bool down      = false;
		int64_t finger = -1;
		bool fired     = false;
		bool tapQueued = false; // parmak aynı karede basıp kalktıysa tuş yine de bir kare basılı sayılır
		int hotkeySlot = -1;    // 0..7: beceri yuvası halkası (tuş kalkışta basılır; sürükleme = taşı/boşalt)
	};
	enum class Role { None, Joystick, Button, Pending, Camera, LeftDrag, Done };
	struct Finger
	{
		int64_t id;
		Role role;
		int startX, startY, x, y;
		int buttonIndex;
		uint32_t downTicks;
		bool longPressDrag = false; // uzun basışla başlayan sürükleme (kalkınca yapışkan olur)
		bool slotDrag      = false; // beceri yuvasından sürükleme başladı
	};

	Finger* Find(int64_t id);
	int HitButton(int x, int y) const;
	bool InJoystickZone(int x, int y) const;
	void ReleaseFinger(Finger& f);
	void BeginLeftDrag(Finger& f);
	/// Nokta görünür bir arayüz penceresinin (çanta, beceri, ticaret...) üstünde mi?
	bool IsOverUI(int x, int y) const;
	/// Nokta bir iletişim penceresinin (çanta, karakter, beceri, ticaret…) üstünde mi? Sohbet/durum çubuğu gibi
	/// kalıcı HUD pencereleri sayılmaz (joystick onların üstünde çalışmaya devam eder)
	bool IsOverDialogUI(int x, int y) const;
	int HitHotkeyRing(int x, int y) const; // 0..7 ya da -1
	// Küçük bilgi satırları (hasar/olay iletileri): sağ altta, beceri kümesinin üstünde, yarı saydam
	struct InfoLine { std::string text; uint32_t color; uint32_t ticks; };
	std::deque<InfoLine> m_infoLines;
	void DrawInfoLines(IDirect3DDevice9* dev);
	float m_clusterTop = 0; // beceri kümesinin üst kenarı (F1-F8 satırı)
	bool m_relayout    = false; // ToggleHide: düğme dizisi döngü dışında yeniden kurulur
	std::vector<std::string> m_infoFontText; // DrawInfoLines: satır metni değişmedikçe SetText çağrılmaz
	bool m_lastRenderSkipped = false;
	void DrawCircle(IDirect3DDevice9* dev, float cx, float cy, float r, uint32_t color, int segs = 32);
	void DrawRing(IDirect3DDevice9* dev, float cx, float cy, float r, float thickness, uint32_t color, int segs = 40);
	void DrawRect(IDirect3DDevice9* dev, float x, float y, float w, float h, uint32_t color);
	void DrawLabel(IDirect3DDevice9* dev, int index, const std::string& text, float x, float y, uint32_t color, int height);
	void RenderFps(IDirect3DDevice9* dev);
	void DrawTexturedQuad(IDirect3DDevice9* dev, float x, float y, float w, float h, void* pTex, uint32_t color);
	void DrawSkillIcons(IDirect3DDevice9* dev); // kısayol yuvalarındaki beceri ikonları halkaların içine
	// Oyuncu menüsü (CGameProcMain::m_iTouchMenuPlayerID): satır dikdörtgenleri
	struct MenuRow { int action; float x, y, w, h; std::string label; };
	void LayoutPlayerMenu(std::vector<MenuRow>& rows) const;
	bool HitPlayerMenu(int x, int y);

	bool m_enabled      = false;
	bool m_showFps      = false;
	Tuning m_tuning;
	uint32_t m_fpsLastTick = 0;
	int m_fpsFrames        = 0;
	float m_fpsValue       = 0.0f;
	std::string m_fpsText;
	CDFont* m_fpsFont      = nullptr;
	bool m_forceVisible = false;
	bool m_hidden       = false; // GİZLE: yalnız gizle/göster düğmesi kalır (joystick ve tuşlar kapalı)
	int m_w = 1024, m_h = 768;
	// Sunucu seçme ekranındaki BAĞLAN düğmesi (alt orta)
	float m_connCx = 0, m_connCy = 0, m_connW = 0, m_connH = 0;
	bool m_connDown = false;
	// Oyun öncesi ekran düğmeleri (karakter oluşturma / seçme); her kare yeniden yerleştirilir
	enum class PreAction { Race, Class, AutoBonus, Create, Cancel, SelStart, SelNew };
	struct PreButton
	{
		PreAction action;
		int arg;
		std::string label;
		int fontIdx;
		float x, y, w, h;
		bool selected;
	};
	std::vector<PreButton> m_preButtons;
	void LayoutPreButtons();
	void DoPreAction(const PreButton& b);
	float m_u = 1.0f; // 768p'ye göre ölçek
	float m_barH = 0.0f; // alt çubuk yüksekliği (Layout belirler)
	// Joystick (yüzen)
	float m_joyR = 0, m_joyHomeX = 0, m_joyHomeY = 0;
	float m_joyCx = 0, m_joyCy = 0, m_knobX = 0, m_knobY = 0;
	int64_t m_joyFinger = -1;
	uint32_t m_lastTapTicks = 0; // çift dokunuş tespiti (arayüz ikonu: sağ tık = kullan/giy)
	int m_lastTapX = -1000, m_lastTapY = -1000;
	int m_skillPage     = 1;  // beceri çubuğu sayfası (F1..F8)
	int m_pageKey       = 0;  // basılı tutulacak sayfa tuşu
	int m_pageKeyFrames = 0;
	std::vector<Button> m_buttons;
	std::vector<Finger> m_fingers;
	std::vector<CDFont*> m_fonts;
	float m_pinchDist = 0.0f;
};

KoTouchOverlay& KoTouch();

#endif // KO_TOUCH_OVERLAY_H
