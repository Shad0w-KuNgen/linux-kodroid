package online.knight.mobile;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.text.InputType;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.SynchronousQueue;

/**
 * Knight Online Mobile launcher (PC'deki KO Launcher'ın karşılığı):
 *  - tam ekran yatay, arka plan + logo, sürüm etiketi
 *  - haberler (VersionManager LS_NEWS) ve sunucu durumu (LS_SERVERLIST)
 *  - sıra: APK güncellemesi → veri kurulumu (ilk sefer) → veri yaması (LS_VERSION_REQ/
 *    LS_DOWNLOADINFO_REQ) → OYUNA BAŞLA; hepsi aynı ilerleme çubuğunda (yüzde, MB, hız)
 *  - Ayarlar (Server.ini / Option.ini), Veriyi onar / yeniden kur (adres, zip, sil)
 */
public class LauncherActivity extends Activity {
    private static final int REQ_PICK_ZIP = 1001;
    private static final int REQ_UNKNOWN_SOURCES = 1002;
    private static final String PREFS = "setup";
    private static final String PREF_URL = "dataUrl";
    private static final String PREF_APK_MANIFEST = "apkManifestUrl";
    private static final int PATCH_TIMEOUT_MS = 10000;

    private static final int GOLD = 0xFFE0B35A;
    private static final int GOLD_DIM = 0xFF9C7A33;
    private static final int TEXT = 0xFFE8E2D6;
    private static final int TEXT_DIM = 0xFFB3AA9A;
    private static final int CARD = 0xB3120E0B;
    private static final int CARD_BORDER = 0x66C9A24C;

    private File dataDir;
    private SharedPreferences prefs;

    // Görünümler
    private TextView versionLabel, newsText, newsTitle, serverTitle, statusText, stageText;
    private LinearLayout serverList;
    private ProgressBar bar;
    private Button btnStart, btnSettings, btnRepair, btnCancel, btnReport;

    // İş durumu
    private volatile boolean cancelRequested;
    private Thread worker;
    private volatile boolean apkPending;      // yeni APK indirildi, kurulum bekliyor
    private File pendingApk;

    // İlerleme (iş parçacığından yazılır, UI zamanlayıcısı okur)
    private volatile long progDone, progTotal = -1;
    private volatile String progEntry = "", progNote = "";
    private volatile int progReconnects;
    private volatile String currentVerb = "";
    private long speedLastTime, speedLastBytes;
    private double speedBps;
    private final Handler uiHandler = new Handler(Looper.getMainLooper());
    private final Runnable uiTick = new Runnable() {
        @Override
        public void run() {
            updateProgressUi();
            if (worker != null && worker.isAlive())
                uiHandler.postDelayed(this, 400);
        }
    };

    // ------------------------------------------------------------------------------------
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        dataDir = GameData.dataDir(this);
        prefs = getSharedPreferences(PREFS, MODE_PRIVATE);
        hideSystemBars();
        buildUi();
        refreshState();
        fetchServerInfo();
        if (GameData.hasGameData(dataDir))
            startChain(null, null);         // APK → yama → hazır
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemBars();
    }

    private void hideSystemBars() {
        View v = getWindow().getDecorView();
        v.setSystemUiVisibility(View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
    }

    // ---- Arayüz --------------------------------------------------------------------------
    private void buildUi() {
        FrameLayout root = new FrameLayout(this);
        root.addView(new LauncherBackgroundView(this, dataDir),
                new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        LinearLayout col = new LinearLayout(this);
        col.setOrientation(LinearLayout.VERTICAL);
        int pad = dp(18);
        col.setPadding(pad, dp(12), pad, dp(10));

        // Başlık satırı: logo + sürüm
        LinearLayout header = new LinearLayout(this);
        header.setOrientation(LinearLayout.HORIZONTAL);
        header.setGravity(Gravity.CENTER_VERTICAL);
        LinearLayout logo = new LinearLayout(this);
        logo.setOrientation(LinearLayout.VERTICAL);
        TextView t1 = new TextView(this);
        t1.setText("KNIGHT ONLINE");
        t1.setTextSize(30);
        t1.setTypeface(Typeface.create(Typeface.SERIF, Typeface.BOLD));
        t1.setTextColor(GOLD);
        t1.setShadowLayer(8f, 0, 3, 0xCC000000);
        t1.setLetterSpacing(0.12f);
        TextView t2 = new TextView(this);
        t2.setText("M O B I L E");
        t2.setTextSize(11);
        t2.setTextColor(TEXT_DIM);
        t2.setLetterSpacing(0.3f);
        t2.setPadding(dp(2), 0, 0, 0);
        logo.addView(t1);
        logo.addView(t2);
        header.addView(logo, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        versionLabel = new TextView(this);
        versionLabel.setTextColor(TEXT_DIM);
        versionLabel.setTextSize(12);
        versionLabel.setGravity(Gravity.END);
        header.addView(versionLabel, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        col.addView(header, matchWrap());

        // Orta: haberler (sol) + sunucu durumu & düğmeler (sağ)
        LinearLayout middle = new LinearLayout(this);
        middle.setOrientation(LinearLayout.HORIZONTAL);
        LinearLayout.LayoutParams mlp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f);
        mlp.topMargin = dp(10);
        col.addView(middle, mlp);

        // Haber kartı
        LinearLayout newsCard = card();
        newsTitle = cardTitle("HABERLER");
        newsCard.addView(newsTitle);
        newsText = new TextView(this);
        newsText.setTextColor(TEXT);
        newsText.setTextSize(14);
        newsText.setLineSpacing(0, 1.15f);
        newsText.setText("Haberler alınıyor...");
        ScrollView newsScroll = new ScrollView(this);
        newsScroll.addView(newsText, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        newsCard.addView(newsScroll, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f));
        LinearLayout.LayoutParams nlp = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.MATCH_PARENT, 3f);
        nlp.rightMargin = dp(10);
        middle.addView(newsCard, nlp);

        // Sağ sütun
        LinearLayout right = new LinearLayout(this);
        right.setOrientation(LinearLayout.VERTICAL);
        middle.addView(right, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.MATCH_PARENT, 2f));

        LinearLayout serverCard = card();
        serverTitle = cardTitle("SUNUCU DURUMU");
        serverCard.addView(serverTitle);
        serverList = new LinearLayout(this);
        serverList.setOrientation(LinearLayout.VERTICAL);
        TextView wait = new TextView(this);
        wait.setText("Sorgulanıyor...");
        wait.setTextColor(TEXT_DIM);
        wait.setTextSize(13);
        serverList.addView(wait);
        ScrollView ss = new ScrollView(this);
        ss.addView(serverList, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        serverCard.addView(ss, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f));
        right.addView(serverCard, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f));

        // Düğmeler
        btnStart = bigButton("OYUNA BAŞLA");
        btnStart.setOnClickListener(v -> onStartPressed());
        LinearLayout.LayoutParams blp = matchWrap();
        blp.topMargin = dp(10);
        right.addView(btnStart, blp);

        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        btnSettings = smallButton("Ayarlar");
        btnSettings.setOnClickListener(v -> showSettings());
        btnRepair = smallButton("Veriyi onar / yeniden kur");
        btnRepair.setOnClickListener(v -> showRepair());
        LinearLayout.LayoutParams s1 = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f);
        s1.rightMargin = dp(6);
        LinearLayout.LayoutParams s2 = new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1.6f);
        row.addView(btnSettings, s1);
        row.addView(btnRepair, s2);
        LinearLayout.LayoutParams rlp = matchWrap();
        rlp.topMargin = dp(6);
        right.addView(row, rlp);

        btnReport = smallButton("Hata raporu gönder (Log + logcat + GPU)");
        btnReport.setOnClickListener(v -> sendBugReport());
        LinearLayout.LayoutParams brlp = matchWrap();
        brlp.topMargin = dp(6);
        right.addView(btnReport, brlp);

        // Alt: aşama + ilerleme çubuğu + durum + iptal
        stageText = new TextView(this);
        stageText.setTextColor(GOLD);
        stageText.setTextSize(13);
        stageText.setTypeface(Typeface.DEFAULT_BOLD);
        LinearLayout.LayoutParams stlp = matchWrap();
        stlp.topMargin = dp(10);
        col.addView(stageText, stlp);

        LinearLayout bottom = new LinearLayout(this);
        bottom.setOrientation(LinearLayout.HORIZONTAL);
        bottom.setGravity(Gravity.CENTER_VERTICAL);
        bar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        bar.setMax(1000);
        bar.setProgress(0);
        bar.getProgressDrawable().setColorFilter(GOLD, android.graphics.PorterDuff.Mode.SRC_IN);
        LinearLayout.LayoutParams plp = new LinearLayout.LayoutParams(0, dp(14), 1f);
        bottom.addView(bar, plp);
        btnCancel = smallButton("İptal");
        btnCancel.setVisibility(View.GONE);
        btnCancel.setOnClickListener(v -> cancelRequested = true);
        LinearLayout.LayoutParams clp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        clp.leftMargin = dp(10);
        bottom.addView(btnCancel, clp);
        LinearLayout.LayoutParams botlp = matchWrap();
        botlp.topMargin = dp(4);
        col.addView(bottom, botlp);

        statusText = new TextView(this);
        statusText.setTextColor(TEXT_DIM);
        statusText.setTextSize(12);
        statusText.setMaxLines(2);
        col.addView(statusText, matchWrap());

        root.addView(col, new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        setContentView(root);
    }

    private LinearLayout card() {
        LinearLayout c = new LinearLayout(this);
        c.setOrientation(LinearLayout.VERTICAL);
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(CARD);
        bg.setCornerRadius(dp(6));
        bg.setStroke(dp(1), CARD_BORDER);
        c.setBackground(bg);
        int p = dp(12);
        c.setPadding(p, dp(8), p, p);
        return c;
    }

    private TextView cardTitle(String s) {
        TextView t = new TextView(this);
        t.setText(s);
        t.setTextColor(GOLD);
        t.setTextSize(12);
        t.setTypeface(Typeface.DEFAULT_BOLD);
        t.setLetterSpacing(0.15f);
        t.setPadding(0, 0, 0, dp(6));
        return t;
    }

    private Button bigButton(String s) {
        Button b = new Button(this);
        b.setText(s);
        b.setAllCaps(false);
        b.setTextSize(20);
        b.setTypeface(Typeface.create(Typeface.SERIF, Typeface.BOLD));
        b.setTextColor(0xFF1A1206);
        GradientDrawable bg = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM,
                new int[] { 0xFFF1CF7A, 0xFFC9962F, 0xFF8F6418 });
        bg.setCornerRadius(dp(6));
        bg.setStroke(dp(1), 0xFF5C3F0C);
        b.setBackground(bg);
        b.setPadding(dp(12), dp(14), dp(12), dp(14));
        return b;
    }

    private Button smallButton(String s) {
        Button b = new Button(this);
        b.setText(s);
        b.setAllCaps(false);
        b.setTextSize(13);
        b.setTextColor(TEXT);
        GradientDrawable bg = new GradientDrawable();
        bg.setColor(0xCC2A2118);
        bg.setCornerRadius(dp(5));
        bg.setStroke(dp(1), CARD_BORDER);
        b.setBackground(bg);
        b.setPadding(dp(8), dp(8), dp(8), dp(8));
        return b;
    }

    private void refreshState() {
        boolean has = GameData.hasGameData(dataDir);
        File ini = GameData.findServerIni(dataDir);
        String files = GameData.getIniValue(ini, "Version", "Files", "-");
        versionLabel.setText("Veri sürümü " + (has ? files : "-") + " · protokol " + GameData.protocolOf(ini) + "\nAPK "
                + ApkUpdater.installedVersionName(this) + " (" + ApkUpdater.installedVersionCode(this) + ")");
        if (apkPending)
            btnStart.setText("APK GÜNCELLEMESİNİ KUR");
        else
            btnStart.setText(has ? "OYUNA BAŞLA" : "VERİYİ İNDİR VE KUR");
        btnRepair.setEnabled(true);
        if (!has && stageText.getText().length() == 0)
            stageText.setText("Oyun verisi kurulu değil. \"Veriyi indir ve kur\" ile başlayın.");
    }

    private void setBusy(boolean busy) {
        btnStart.setEnabled(!busy);
        btnSettings.setEnabled(!busy);
        btnRepair.setEnabled(!busy);
        btnCancel.setVisibility(busy ? View.VISIBLE : View.GONE);
        if (busy) {
            bar.setIndeterminate(true);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            bar.setIndeterminate(false);
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    // ---- Haberler / sunucu durumu --------------------------------------------------------
    private void fetchServerInfo() {
        final File ini = GameData.findServerIni(dataDir);
        final String host = GameData.getIniValue(ini, "Server", "IP0", GameData.DEFAULT_SERVER_IP);
        final int protocol = GameData.protocolOf(ini);
        final int loginPort = GameData.loginPortOf(ini);
        new Thread(() -> {
            List<PatchClient.ServerInfo> servers = null;
            List<PatchClient.NewsItem> news = null;
            String err = null;
            try (PatchClient pc = new PatchClient(host, loginPort, 6000)) {
                try {
                    news = pc.queryNews();
                } catch (IOException e) {
                    err = e.getMessage();
                }
                servers = pc.queryServerList(protocol);
            } catch (IOException e) {
                err = e.getMessage();
            }
            final List<PatchClient.ServerInfo> fs = servers;
            final List<PatchClient.NewsItem> fn = news;
            final String ferr = err;
            runOnUiThread(() -> {
                // Haberler
                if (fn != null && !fn.isEmpty()) {
                    android.text.SpannableStringBuilder sb = new android.text.SpannableStringBuilder();
                    for (PatchClient.NewsItem n : fn) {
                        if (!n.title.isEmpty()) {
                            int a = sb.length();
                            sb.append(n.title).append('\n');
                            sb.setSpan(new android.text.style.ForegroundColorSpan(GOLD), a, sb.length(), 0);
                            sb.setSpan(new android.text.style.StyleSpan(Typeface.BOLD), a, sb.length(), 0);
                        }
                        sb.append(n.message).append("\n\n");
                    }
                    newsText.setText(sb);
                } else if (fn != null) {
                    newsText.setText("Yeni haber yok.");
                } else {
                    newsText.setText("Haberler alınamadı" + (ferr != null ? ": " + ferr : "."));
                }
                // Sunucu listesi
                serverList.removeAllViews();
                if (fs == null) {
                    serverList.addView(serverRow("Giriş sunucusu " + host + ":" + loginPort + " (v" + protocol + ")", 0xFFD9534F,
                            "çevrimdışı" + (ferr != null ? " (" + ferr + ")" : "")));
                } else if (fs.isEmpty()) {
                    serverList.addView(serverRow("Giriş sunucusu", 0xFF5CB85C, "çevrimiçi, oyun sunucusu listesi boş"));
                } else {
                    for (PatchClient.ServerInfo si : fs) {
                        boolean full = si.users < 0;
                        String state = full ? "dolu" : "çevrimiçi · " + si.users + " oyuncu";
                        if (protocol == GameData.PROTOCOL_2369 && si.playerCap > 0 && !full)
                            state += " / " + si.playerCap;
                        serverList.addView(serverRow((si.name.isEmpty() ? si.ip : si.name) + " (v" + protocol + ")",
                                full ? 0xFFF0AD4E : 0xFF5CB85C, state));
                    }
                }
            });
        }, "ko-info").start();
    }

    private View serverRow(String name, int dotColor, String state) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(0, dp(3), 0, dp(3));
        View dot = new View(this);
        GradientDrawable d = new GradientDrawable();
        d.setShape(GradientDrawable.OVAL);
        d.setColor(dotColor);
        dot.setBackground(d);
        LinearLayout.LayoutParams dlp = new LinearLayout.LayoutParams(dp(10), dp(10));
        dlp.rightMargin = dp(8);
        row.addView(dot, dlp);
        TextView n = new TextView(this);
        n.setText(name);
        n.setTextColor(TEXT);
        n.setTextSize(14);
        n.setTypeface(Typeface.DEFAULT_BOLD);
        row.addView(n, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        TextView s = new TextView(this);
        s.setText(state);
        s.setTextColor(TEXT_DIM);
        s.setTextSize(12);
        row.addView(s, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        return row;
    }

    // ---- Düğmeler --------------------------------------------------------------------------
    private void onStartPressed() {
        if (apkPending && pendingApk != null) {
            installPendingApk();
            return;
        }
        if (!GameData.hasGameData(dataDir)) {
            startChain(prefs.getString(PREF_URL, GameData.DEFAULT_DATA_URL), null);
            return;
        }
        launchGameNow();
    }

    private void launchGameNow() {
        GameData.ensureServerIni(dataDir);
        Intent i = new Intent(this, KnightOnlineActivity.class);
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(i);
        finish();
    }

    private void showRepair() {
        String[] items = { "Adresten yeniden indir ve kur", "Zip dosyası seç (telefondan)", "Kurulu veriyi sil",
                "Yamayı yeniden denetle" };
        new AlertDialog.Builder(this)
                .setTitle("Veriyi onar / yeniden kur")
                .setItems(items, (d, which) -> {
                    switch (which) {
                        case 0:
                            askUrlThen(url -> startChain(url, null));
                            break;
                        case 1:
                            pickZip();
                            break;
                        case 2:
                            confirmDelete();
                            break;
                        default:
                            startChain(null, null);
                    }
                })
                .setNegativeButton("Vazgeç", null)
                .show();
    }

    private interface UrlCallback {
        void run(String url);
    }

    private void askUrlThen(final UrlCallback cb) {
        final EditText e = new EditText(this);
        e.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI);
        e.setSingleLine(true);
        e.setText(prefs.getString(PREF_URL, GameData.DEFAULT_DATA_URL));
        new AlertDialog.Builder(this)
                .setTitle("Veri paketi adresi (.zip)")
                .setView(e)
                .setPositiveButton("İndir", (d, w) -> {
                    String url = e.getText().toString().trim();
                    if (!url.isEmpty()) {
                        prefs.edit().putString(PREF_URL, url).apply();
                        cb.run(url);
                    }
                })
                .setNegativeButton("Vazgeç", null)
                .show();
    }

    private void pickZip() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("*/*");
        i.putExtra(Intent.EXTRA_MIME_TYPES, new String[] { "application/zip", "application/x-zip-compressed",
                "application/octet-stream" });
        try {
            startActivityForResult(i, REQ_PICK_ZIP);
        } catch (Exception e) {
            statusText.setText("Dosya seçici açılamadı: " + e.getMessage());
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQ_UNKNOWN_SOURCES) {
            if (apkPending && pendingApk != null && ApkUpdater.canInstallUnknown(this))
                installPendingApk();
            return;
        }
        if (requestCode != REQ_PICK_ZIP || resultCode != RESULT_OK || data == null || data.getData() == null)
            return;
        startChain(null, data.getData());
    }

    private void confirmDelete() {
        new AlertDialog.Builder(this)
                .setTitle("Veriyi sil")
                .setMessage("Kurulu oyun verisi silinsin mi?\n" + dataDir.getAbsolutePath())
                .setPositiveButton("Sil", (d, w) -> {
                    setBusy(true);
                    stageText.setText("Veri siliniyor...");
                    new Thread(() -> {
                        File[] kids = dataDir.listFiles();
                        if (kids != null)
                            for (File k : kids)
                                GameData.deleteRecursive(k);
                        runOnUiThread(() -> {
                            setBusy(false);
                            stageText.setText("Veri silindi.");
                            refreshState();
                        });
                    }).start();
                })
                .setNegativeButton("Vazgeç", null)
                .show();
    }

    // ---- Hata raporu -----------------------------------------------------------------------
    private void sendBugReport() {
        btnReport.setEnabled(false);
        stageText.setText("Hata raporu hazırlanıyor...");
        new Thread(() -> {
            File zip = null;
            String err = null;
            try {
                zip = BugReport.build(this, dataDir);
            } catch (Exception e) {
                err = e.getMessage();
            }
            final File fz = zip;
            final String ferr = err;
            runOnUiThread(() -> {
                btnReport.setEnabled(true);
                if (fz == null) {
                    stageText.setText("Hata raporu oluşturulamadı: " + ferr);
                    return;
                }
                stageText.setText("Hata raporu hazır: " + fz.getName() + " (" + GameData.humanBytes(fz.length()) + ")");
                try {
                    startActivity(BugReport.shareIntent(this, fz));
                } catch (Exception e) {
                    stageText.setText("Paylaşım açılamadı: " + e.getMessage());
                }
            });
        }, "ko-report").start();
    }

    // ---- Ayarlar ---------------------------------------------------------------------------
    private void showSettings() {
        final File serverIni = GameData.findServerIni(dataDir);
        final File optionIni = findIni("Option.ini");
        LinearLayout box = new LinearLayout(this);
        box.setOrientation(LinearLayout.VERTICAL);
        int p = dp(16);
        box.setPadding(p, dp(8), p, 0);

        final EditText ip = settingField(box, "Giriş sunucusu (Server.ini IP0)",
                GameData.getIniValue(serverIni, "Server", "IP0", GameData.DEFAULT_SERVER_IP));
        TextView protoLabel = new TextView(this);
        protoLabel.setText("Sunucu protokolü (Server.ini [Server] Protocol)");
        protoLabel.setTextSize(12);
        protoLabel.setPadding(0, dp(8), 0, 0);
        box.addView(protoLabel);
        final android.widget.RadioGroup proto = new android.widget.RadioGroup(this);
        proto.setOrientation(LinearLayout.HORIZONTAL);
        final int curProto = GameData.protocolOf(serverIni);
        int[] protos = { GameData.PROTOCOL_1298, GameData.PROTOCOL_2369 };
        String[] protoNames = { "1.298 (OpenKO)", "2369 (ISTIRAP)" };
        for (int k = 0; k < protos.length; k++) {
            android.widget.RadioButton rb = new android.widget.RadioButton(this);
            rb.setText(protoNames[k]);
            rb.setId(3000 + protos[k]);
            proto.addView(rb);
            if (protos[k] == curProto)
                rb.setChecked(true);
        }
        box.addView(proto);
        final EditText loginPort = settingField(box, "Giriş sunucusu portu (1298: 15100, 2369: 15200)",
                String.valueOf(GameData.loginPortOf(serverIni)));
        final EditText gamePort = settingField(box, "Oyun sunucusu portu (1298: 15001, 2369: 15301)",
                String.valueOf(GameData.gamePortOf(serverIni)));
        final EditText dataUrl = settingField(box, "Veri paketi adresi (.zip)",
                prefs.getString(PREF_URL, GameData.DEFAULT_DATA_URL));
        proto.setOnCheckedChangeListener((g, id) -> {
            // Protokol değişince portları ve varsayılan veri adresini eşle (elle girilmiş değerlere dokunma)
            boolean to2369 = id == 3000 + GameData.PROTOCOL_2369;
            int lp = parseIntOr(loginPort.getText().toString(), 0), gp = parseIntOr(gamePort.getText().toString(), 0);
            if (lp == GameData.LOGIN_PORT_1298 || lp == GameData.LOGIN_PORT_2369 || lp == 0)
                loginPort.setText(String.valueOf(to2369 ? GameData.LOGIN_PORT_2369 : GameData.LOGIN_PORT_1298));
            if (gp == GameData.GAME_PORT_1298 || gp == GameData.GAME_PORT_2369 || gp == 0)
                gamePort.setText(String.valueOf(to2369 ? GameData.GAME_PORT_2369 : GameData.GAME_PORT_1298));
            String u = dataUrl.getText().toString().trim();
            if (u.isEmpty() || u.equals(GameData.DEFAULT_DATA_URL) || u.equals(GameData.DEFAULT_DATA_URL_2369))
                dataUrl.setText(to2369 ? GameData.DEFAULT_DATA_URL_2369 : GameData.DEFAULT_DATA_URL);
        });
        final EditText apkUrl = settingField(box, "APK güncelleme bildirimi (apk.json)",
                prefs.getString(PREF_APK_MANIFEST, ApkUpdater.DEFAULT_MANIFEST_URL));
        final EditText logical = settingField(box, "Mantıksal yükseklik (0 = ekran, 768 = ölçekle)",
                GameData.getIniValue(optionIni, "Mobile", "LogicalHeight", "768"));
        final CheckBox touch = new CheckBox(this);
        touch.setText("Dokunmatik kontroller (joystick, beceri düğmeleri)");
        touch.setChecked(!"0".equals(GameData.getIniValue(optionIni, "Mobile", "TouchControls", "1")));
        box.addView(touch);
        final CheckBox dbg = new CheckBox(this);
        dbg.setText("Girdi hata ayıklama kaydı (logcat [ko-input])");
        dbg.setChecked("1".equals(GameData.getIniValue(optionIni, "Mobile", "InputDebug", "0")));
        box.addView(dbg);
        final CheckBox fps = new CheckBox(this);
        fps.setText("FPS sayacı göster");
        fps.setChecked("1".equals(GameData.getIniValue(optionIni, "Mobile", "ShowFps", "0")));
        box.addView(fps);
        final CheckBox shadow = new CheckBox(this);
        shadow.setText("Gölgeler");
        shadow.setChecked(!"0".equals(GameData.getIniValue(optionIni, "Shadow", "Use", "1")));
        box.addView(shadow);
        final CheckBox lowTex = new CheckBox(this);
        lowTex.setText("Düşük doku kalitesi (daha az bellek, hızlı)");
        lowTex.setChecked("1".equals(GameData.getIniValue(optionIni, "Texture", "LOD_Chr", "0")));
        box.addView(lowTex);
        TextView rsLabel = new TextView(this);
        rsLabel.setText("Çözünürlük ölçeği (3D sahne)");
        rsLabel.setTextSize(12);
        rsLabel.setPadding(0, dp(8), 0, 0);
        box.addView(rsLabel);
        final android.widget.RadioGroup rs = new android.widget.RadioGroup(this);
        rs.setOrientation(LinearLayout.HORIZONTAL);
        int cur = parseIntOr(GameData.getIniValue(optionIni, "Mobile", "RenderScale", "100"), 100);
        int[] scales = { 50, 75, 100 };
        for (int sc : scales) {
            android.widget.RadioButton rb = new android.widget.RadioButton(this);
            rb.setText("%" + sc);
            rb.setId(1000 + sc);
            rs.addView(rb);
            if (sc == cur)
                rb.setChecked(true);
        }
        if (rs.getCheckedRadioButtonId() == -1)
            rs.check(1100);
        box.addView(rs);

        TextView uiLabel = new TextView(this);
        uiLabel.setText("Oyun içi arayüz ölçeği (düğme/pencere boyutu)");
        uiLabel.setTextSize(12);
        uiLabel.setPadding(0, dp(8), 0, 0);
        box.addView(uiLabel);
        final android.widget.RadioGroup ui = new android.widget.RadioGroup(this);
        ui.setOrientation(LinearLayout.HORIZONTAL);
        int curUi = parseIntOr(GameData.getIniValue(optionIni, "Mobile", "UiScale", "100"), 100);
        int[] uiScales = { 100, 125, 150 };
        for (int sc : uiScales) {
            android.widget.RadioButton rb = new android.widget.RadioButton(this);
            rb.setText("%" + sc);
            rb.setId(2000 + sc);
            ui.addView(rb);
            if (sc == curUi)
                rb.setChecked(true);
        }
        if (ui.getCheckedRadioButtonId() == -1)
            ui.check(2100);
        box.addView(ui);

        final EditText camSens = settingField(box, "Kamera sürükleme hızı (%, 25-400)",
                GameData.getIniValue(optionIni, "Mobile", "CameraSens", "100"));
        final EditText joyDead = settingField(box, "Joystick ölü bölgesi (%, 5-60)",
                GameData.getIniValue(optionIni, "Mobile", "JoyDeadZone", "22"));
        final EditText joyRot = settingField(box, "Joystick dönüş hızı (°/sn, 30-400; klasik kip)",
                GameData.getIniValue(optionIni, "Mobile", "JoyRotateSpeed", "150"));
        final CheckBox joyTurnRun = new CheckBox(this);
        joyTurnRun.setText("Joystick \"dön ve koş\" kipi (yön = karakter yönü; kapalı = klasik ileri/geri + dönüş)");
        joyTurnRun.setChecked("1".equals(GameData.getIniValue(optionIni, "Mobile", "JoyTurnAndRun", "0")));
        box.addView(joyTurnRun);
        final EditText longPress = settingField(box, "Uzun basış süresi, ms (sağ tık: konuş / al)",
                GameData.getIniValue(optionIni, "Mobile", "LongPressMs", "450"));
        final EditText hpSlot = settingField(box, "HP pot düğmesi → kısayol yuvası (1-8)",
                GameData.getIniValue(optionIni, "Mobile", "PotHpSlot", "7"));
        final EditText mpSlot = settingField(box, "MP pot düğmesi → kısayol yuvası (1-8)",
                GameData.getIniValue(optionIni, "Mobile", "PotMpSlot", "8"));

        ScrollView sv = new ScrollView(this);
        sv.addView(box);
        new AlertDialog.Builder(this)
                .setTitle("Ayarlar")
                .setView(sv)
                .setPositiveButton("Kaydet", (d, w) -> {
                    File sIni = serverIni != null ? serverIni : new File(dataDir, "Server.ini");
                    if (!sIni.isFile())
                        GameData.ensureServerIni(dataDir);
                    String newIp = ip.getText().toString().trim();
                    if (!newIp.isEmpty()) {
                        GameData.setIniValue(sIni, "Server", "IP0", newIp);
                        GameData.setIniValue(sIni, "Server", "Count", "1");
                    }
                    int newProto = proto.getCheckedRadioButtonId() == 3000 + GameData.PROTOCOL_2369
                            ? GameData.PROTOCOL_2369 : GameData.PROTOCOL_1298;
                    GameData.setIniValue(sIni, "Server", "Protocol", String.valueOf(newProto));
                    GameData.setIniValue(sIni, "Server", "LoginPort", String.valueOf(parseIntOr(loginPort.getText().toString(),
                            newProto == GameData.PROTOCOL_2369 ? GameData.LOGIN_PORT_2369 : GameData.LOGIN_PORT_1298)));
                    GameData.setIniValue(sIni, "Server", "GamePort", String.valueOf(parseIntOr(gamePort.getText().toString(),
                            newProto == GameData.PROTOCOL_2369 ? GameData.GAME_PORT_2369 : GameData.GAME_PORT_1298)));
                    prefs.edit().putString(PREF_URL, dataUrl.getText().toString().trim())
                            .putString(PREF_APK_MANIFEST, apkUrl.getText().toString().trim()).apply();
                    File oIni = optionIni != null ? optionIni : new File(dataDir, "Option.ini");
                    GameData.setIniValue(oIni, "Mobile", "LogicalHeight", logical.getText().toString().trim());
                    GameData.setIniValue(oIni, "Mobile", "TouchControls", touch.isChecked() ? "1" : "0");
                    GameData.setIniValue(oIni, "Mobile", "InputDebug", dbg.isChecked() ? "1" : "0");
                    GameData.setIniValue(oIni, "Mobile", "ShowFps", fps.isChecked() ? "1" : "0");
                    GameData.setIniValue(oIni, "Mobile", "RenderScale", String.valueOf(rs.getCheckedRadioButtonId() - 1000));
                    GameData.setIniValue(oIni, "Mobile", "UiScale", String.valueOf(ui.getCheckedRadioButtonId() - 2000));
                    GameData.setIniValue(oIni, "Mobile", "CameraSens", String.valueOf(parseIntOr(camSens.getText().toString(), 100)));
                    GameData.setIniValue(oIni, "Mobile", "JoyDeadZone", String.valueOf(parseIntOr(joyDead.getText().toString(), 22)));
                    GameData.setIniValue(oIni, "Mobile", "JoyRotateSpeed", String.valueOf(parseIntOr(joyRot.getText().toString(), 150)));
                    GameData.setIniValue(oIni, "Mobile", "JoyTurnAndRun", joyTurnRun.isChecked() ? "1" : "0");
                    GameData.setIniValue(oIni, "Mobile", "LongPressMs", String.valueOf(parseIntOr(longPress.getText().toString(), 450)));
                    GameData.setIniValue(oIni, "Mobile", "PotHpSlot", String.valueOf(parseIntOr(hpSlot.getText().toString(), 7)));
                    GameData.setIniValue(oIni, "Mobile", "PotMpSlot", String.valueOf(parseIntOr(mpSlot.getText().toString(), 8)));
                    GameData.setIniValue(oIni, "Shadow", "Use", shadow.isChecked() ? "1" : "0");
                    String lod = lowTex.isChecked() ? "1" : "0";
                    GameData.setIniValue(oIni, "Texture", "LOD_Chr", lod);
                    GameData.setIniValue(oIni, "Texture", "LOD_Shape", lod);
                    GameData.setIniValue(oIni, "Texture", "LOD_Terrain", lod);
                    refreshState();
                    fetchServerInfo();
                })
                .setNegativeButton("Vazgeç", null)
                .show();
    }

    private EditText settingField(LinearLayout box, String label, String value) {
        TextView l = new TextView(this);
        l.setText(label);
        l.setTextSize(12);
        l.setPadding(0, dp(8), 0, 0);
        box.addView(l);
        EditText e = new EditText(this);
        e.setSingleLine(true);
        e.setText(value);
        box.addView(e);
        return e;
    }

    private File findIni(String name) {
        File[] list = dataDir.listFiles();
        if (list != null)
            for (File f : list)
                if (f.isFile() && f.getName().equalsIgnoreCase(name))
                    return f;
        return null;
    }

    // ---- Zincir: APK → veri kurulumu → yama → hazır ----------------------------------------
    private static final class Source {
        final InputStream in;
        final long total;

        Source(InputStream in, long total) {
            this.in = in;
            this.total = total;
        }
    }

    private ResumingHttpStream.Listener resumeListener() {
        return new ResumingHttpStream.Listener() {
            @Override
            public void onReconnect(int attempt, int maxAttempts, String reason) {
                progReconnects = attempt;
                progNote = "Bağlantı koptu (" + reason + "), kaldığı yerden yeniden bağlanıyor... " + attempt + "/" + maxAttempts;
            }

            @Override
            public boolean isCancelled() {
                return cancelRequested;
            }
        };
    }

    private GameData.Progress zipProgress() {
        return (done, total, entry) -> {
            if (done > progDone)
                progNote = "";
            progDone = done;
            if (total > 0)
                progTotal = total;
            progEntry = entry;
            return !cancelRequested;
        };
    }

    private void resetProgress(String verb) {
        progDone = 0;
        progTotal = -1;
        progEntry = "";
        progNote = "";
        progReconnects = 0;
        speedLastTime = System.currentTimeMillis();
        speedLastBytes = 0;
        speedBps = 0;
        currentVerb = verb;
    }

    private void stage(final String s) {
        runOnUiThread(() -> {
            stageText.setText(s);
            statusText.setText("");
        });
    }

    /** Kullanıcı kararını iş parçacığından bekler (APK güncellemesi sorusu). */
    private boolean askOnUi(final String title, final String message, final String yes, final String no) {
        final SynchronousQueue<Boolean> q = new SynchronousQueue<>();
        runOnUiThread(() -> new AlertDialog.Builder(LauncherActivity.this)
                .setTitle(title).setMessage(message)
                .setPositiveButton(yes, (d, w) -> new Thread(() -> { try { q.put(true); } catch (InterruptedException ignored) {} }).start())
                .setNegativeButton(no, (d, w) -> new Thread(() -> { try { q.put(false); } catch (InterruptedException ignored) {} }).start())
                .setCancelable(false)
                .show());
        try {
            return q.take();
        } catch (InterruptedException e) {
            return false;
        }
    }

    /**
     * @param dataUrl  null değilse önce bu adresten veri paketi indirilip kurulur
     * @param dataUri  null değilse önce bu zip dosyası kurulur
     */
    private void startChain(final String dataUrl, final Uri dataUri) {
        if (worker != null && worker.isAlive())
            return;
        cancelRequested = false;
        resetProgress("");
        setBusy(true);
        btnCancel.setText("İptal");
        worker = new Thread(() -> {
            String error = null;
            try {
                // 1) APK güncellemesi
                stepApkUpdate();
                if (apkPending)
                    return;                       // kurulum kullanıcıya kaldı; zincir orada biter
                // 2) Veri kurulumu (ilk sefer / onarım)
                if (dataUrl != null || dataUri != null)
                    stepInstallData(dataUrl, dataUri);
                // 3) Yama
                if (GameData.hasGameData(dataDir) && !cancelRequested)
                    stepPatch();
            } catch (Exception e) {
                error = cancelRequested ? "İptal edildi." : e.getMessage();
            } finally {
                final String err = error;
                runOnUiThread(() -> {
                    uiHandler.removeCallbacks(uiTick);
                    setBusy(false);
                    bar.setProgress(err == null && !cancelRequested ? 1000 : 0);
                    refreshState();
                    if (err != null) {
                        stageText.setText("Hata: " + err);
                        statusText.setText(GameData.humanBytes(progDone) + " alınmıştı. Tekrar denemek için düğmeye basın.");
                    } else if (apkPending) {
                        stageText.setText("Yeni APK indirildi. Kurmak için düğmeye basın.");
                    } else if (!GameData.hasGameData(dataDir)) {
                        stageText.setText("Oyun verisi kurulu değil. \"Veriyi indir ve kur\" ile başlayın.");
                    } else {
                        stageText.setText(cancelRequested ? "İşlem iptal edildi." : "Hazır. İyi oyunlar!");
                    }
                });
            }
        }, "ko-launcher");
        worker.start();
        uiHandler.postDelayed(uiTick, 400);
    }

    private void stepApkUpdate() throws IOException {
        stage("APK sürümü denetleniyor...");
        String manifestUrl = prefs.getString(PREF_APK_MANIFEST, ApkUpdater.DEFAULT_MANIFEST_URL);
        if (manifestUrl.isEmpty())
            return;
        ApkUpdater.Manifest m;
        try {
            m = ApkUpdater.fetchManifest(manifestUrl, 5000);
        } catch (IOException e) {
            runOnUiThread(() -> statusText.setText("APK güncelleme bilgisi alınamadı: " + e.getMessage()));
            return;                               // engellemez
        }
        int installed = ApkUpdater.installedVersionCode(this);
        if (m.versionCode <= installed) {
            runOnUiThread(() -> statusText.setText("APK güncel (" + installed + ")."));
            return;
        }
        File apk = ApkUpdater.apkFile(this, m);
        if (!(apk.isFile() && (m.sha256.isEmpty() || m.sha256.equals(ApkUpdater.sha256Hex(apk))))) {
            boolean ok = askOnUi("Yeni sürüm " + m.versionName,
                    "Yeni bir uygulama sürümü var (" + installed + " → " + m.versionCode + ")."
                            + (m.notes.isEmpty() ? "" : "\n\n" + m.notes) + "\n\nŞimdi indirilsin mi?",
                    "Güncelle", "Daha sonra");
            if (!ok)
                return;
            resetProgress("APK " + m.versionName);
            stage("APK indiriliyor...");
            ResumingHttpStream rin = new ResumingHttpStream(m.url, resumeListener()).open();
            progTotal = rin.total();
            try (InputStream in = rin) {
                GameData.copyToFile(in, apk, (done, total, entry) -> {
                    if (done > progDone)
                        progNote = "";
                    progDone = done;
                    return !cancelRequested;
                });
            }
            if (!m.sha256.isEmpty() && !m.sha256.equals(ApkUpdater.sha256Hex(apk))) {
                apk.delete();
                throw new IOException("APK özeti (sha256) uyuşmuyor; indirme bozuk");
            }
        }
        pendingApk = apk;
        apkPending = true;
        runOnUiThread(this::installPendingApk);
    }

    private void installPendingApk() {
        if (pendingApk == null)
            return;
        if (!ApkUpdater.canInstallUnknown(this)) {
            new AlertDialog.Builder(this)
                    .setTitle("İzin gerekli")
                    .setMessage("Güncellemeyi kurmak için bu uygulamaya \"bilinmeyen uygulamaları yükleme\" izni verin.")
                    .setPositiveButton("Ayarları aç", (d, w) -> startActivityForResult(ApkUpdater.unknownSourcesSettingsIntent(this), REQ_UNKNOWN_SOURCES))
                    .setNegativeButton("Vazgeç", null)
                    .show();
            return;
        }
        try {
            startActivity(ApkUpdater.installIntent(this, pendingApk));
        } catch (Exception e) {
            stageText.setText("Yükleyici açılamadı: " + e.getMessage());
        }
    }

    private void stepInstallData(String url, Uri uri) throws IOException {
        resetProgress(uri != null ? "Zip açılıyor" : "Veri indiriliyor");
        stage(uri != null ? "Oyun verisi zip'ten kuruluyor..." : "Oyun verisi indiriliyor...\n" + url);
        Source src;
        if (uri != null) {
            long size = -1;
            try (android.database.Cursor c = getContentResolver().query(uri, null, null, null, null)) {
                if (c != null && c.moveToFirst()) {
                    int idx = c.getColumnIndex(android.provider.OpenableColumns.SIZE);
                    if (idx >= 0 && !c.isNull(idx))
                        size = c.getLong(idx);
                }
            } catch (Exception ignored) {
            }
            InputStream in = getContentResolver().openInputStream(uri);
            if (in == null)
                throw new IOException("Dosya açılamadı");
            src = new Source(in, size);
        } else {
            ResumingHttpStream rin = new ResumingHttpStream(url, resumeListener()).open();
            src = new Source(rin, rin.total());
        }
        progTotal = src.total;
        try (InputStream in = src.in) {
            GameData.extractZip(in, src.total, dataDir, zipProgress());
        }
        if (!GameData.hasGameData(dataDir))
            throw new IOException("Paket açıldı ama içinde oyun verisi (UI klasörü) yok");
        runOnUiThread(this::refreshState);
    }

    private void stepPatch() throws IOException {
        final File ini = GameData.findServerIni(dataDir);
        final String host = GameData.getIniValue(ini, "Server", "IP0", GameData.DEFAULT_SERVER_IP);
        final int clientVersion = parseIntOr(GameData.getIniValue(ini, "Version", "Files", "0"), 0);
        final int loginPort = GameData.loginPortOf(ini);
        stage("Sürüm denetleniyor (" + host + ":" + loginPort + ", veri " + clientVersion + ", protokol "
                + GameData.protocolOf(ini) + ")...");
        int latest;
        PatchClient.DownloadInfo info;
        try (PatchClient pc = new PatchClient(host, loginPort, PATCH_TIMEOUT_MS)) {
            latest = pc.queryLatestVersion();
            if (latest <= clientVersion) {
                stage("Veri güncel (sürüm " + clientVersion + ").");
                return;
            }
            info = pc.queryDownloadInfo(clientVersion);
        } catch (IOException e) {
            throw new IOException("Sürüm sunucusuna ulaşılamadı (" + host + ":" + loginPort + "): " + e.getMessage());
        }
        int n = info.files.size();
        for (int k = 0; k < n && !cancelRequested; k++) {
            String name = info.files.get(k);
            String url = info.fileUrl(name);
            resetProgress("Yama " + (k + 1) + "/" + n + " (" + name + ")");
            stage("Yama indiriliyor " + (k + 1) + "/" + n + ": " + name + "  (" + clientVersion + " → " + latest + ")");
            ResumingHttpStream rin = new ResumingHttpStream(url, resumeListener()).open();
            progTotal = rin.total();
            try (InputStream in = rin) {
                if (name.toLowerCase(Locale.ROOT).endsWith(".zip"))
                    GameData.extractZip(in, rin.total(), dataDir, zipProgress());
                else
                    GameData.copyToFile(in, new File(dataDir, name.replace('\\', '/')), (done, total, entry) -> {
                        if (done > progDone)
                            progNote = "";
                        progDone = done;
                        return !cancelRequested;
                    });
            }
        }
        if (!cancelRequested) {
            File target = ini != null ? ini : new File(dataDir, "Server.ini");
            GameData.setIniValue(target, "Version", "Files", String.valueOf(latest));
            stage("Yama tamamlandı: sürüm " + latest + ".");
        }
    }

    // ---- İlerleme metni ------------------------------------------------------------------
    private void updateProgressUi() {
        long done = progDone, total = progTotal;
        long now = System.currentTimeMillis();
        if (now - speedLastTime >= 1000) {
            speedBps = (done - speedLastBytes) * 1000.0 / (now - speedLastTime);
            speedLastTime = now;
            speedLastBytes = done;
        }
        if (currentVerb.isEmpty() && done == 0) {
            return;                                   // ağ sorgusu aşaması: yalnız aşama metni
        }
        StringBuilder sb = new StringBuilder(currentVerb);
        if (total > 0) {
            int permille = (int) Math.min(1000, done * 1000 / total);
            bar.setIndeterminate(false);
            bar.setProgress(permille);
            sb.append(String.format(Locale.ROOT, ": %%%d  (%s / %s)", permille / 10, GameData.humanBytes(done),
                    GameData.humanBytes(total)));
            if (speedBps > 1024) {
                long etaSec = (long) ((total - done) / speedBps);
                sb.append(String.format(Locale.ROOT, "  %.1f MB/sn, kalan ~%d:%02d", speedBps / (1024 * 1024), etaSec / 60,
                        etaSec % 60));
            }
        } else {
            sb.append(": ").append(GameData.humanBytes(done));
            if (speedBps > 1024)
                sb.append(String.format(Locale.ROOT, "  %.1f MB/sn", speedBps / (1024 * 1024)));
        }
        if (progNote.length() > 0)
            sb.append("\n").append(progNote);
        else if (progEntry.length() > 0)
            sb.append("\n").append(progEntry);
        statusText.setText(sb.toString());
    }

    // ---- Yardımcılar ---------------------------------------------------------------------
    private static int parseIntOr(String s, int def) {
        try {
            return Integer.parseInt(s.trim());
        } catch (Exception e) {
            return def;
        }
    }

    private int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
    }
}
