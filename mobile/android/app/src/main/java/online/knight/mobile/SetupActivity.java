package online.knight.mobile;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.WindowManager;
import android.text.InputType;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.ScrollView;
import android.widget.TextView;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;

/**
 * Başlatıcı etkinlik: oyun verisi yoksa kullanıcıdan bir zip seçmesini ya da bir adresten
 * indirmesini ister, veriyi uygulamanın kendi dizinine açar ve oyunu başlatır. Veri zaten
 * varsa doğrudan oyuna geçer (uzun basışla / "Veri" düğmesiyle bu ekrana dönülebilir).
 */
public class SetupActivity extends Activity {
    private static final int REQ_PICK_ZIP = 1001;
    private static final String PREFS = "setup";
    private static final String PREF_URL = "dataUrl";

    private File dataDir;
    private TextView status;
    private ProgressBar bar;
    private Button btnPick, btnDownload, btnStart, btnCancel, btnDelete;
    private EditText urlEdit;
    private volatile boolean cancelRequested;
    private Thread worker;

    // İlerleme durumu (iş parçacığından yazılır, UI zamanlayıcısı okur)
    private volatile long progDone, progTotal = -1;
    private volatile String progEntry = "", progNote = "";
    private volatile int progReconnects;
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        dataDir = GameData.dataDir(this);

        boolean forceSetup = getIntent().getBooleanExtra("setup", false);
        if (!forceSetup && GameData.hasGameData(dataDir)) {
            startGame();
            return;
        }
        buildUi();
        refreshState();
    }

    // ---- Arayüz --------------------------------------------------------------------------
    private void buildUi() {
        int pad = dp(20);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(pad, pad, pad, pad);
        root.setBackgroundColor(Color.rgb(18, 16, 14));

        TextView title = new TextView(this);
        title.setText("Knight Online Mobile");
        title.setTextSize(26);
        title.setTypeface(Typeface.DEFAULT_BOLD);
        title.setTextColor(Color.rgb(228, 196, 120));
        root.addView(title);

        TextView info = new TextView(this);
        info.setText("Oyun verisi (UI, Data, Misc, Server.ini ...) telefona bir kez kurulmalı. "
                + "İstemci klasörünü tek bir .zip olarak paketleyip aşağıdan seçin ya da bir adresten indirin.\n\n"
                + "Veri dizini: " + dataDir.getAbsolutePath());
        info.setTextColor(Color.rgb(210, 205, 195));
        info.setPadding(0, dp(8), 0, dp(16));
        root.addView(info);

        btnPick = new Button(this);
        btnPick.setText("Zip dosyası seç (telefondan)");
        btnPick.setOnClickListener(v -> pickZip());
        root.addView(btnPick, matchWrap());

        TextView urlLabel = new TextView(this);
        urlLabel.setText("veya indirme adresi (.zip):");
        urlLabel.setTextColor(Color.rgb(210, 205, 195));
        urlLabel.setPadding(0, dp(16), 0, dp(4));
        root.addView(urlLabel);

        urlEdit = new EditText(this);
        urlEdit.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_URI);
        urlEdit.setHint("https://ornek.com/knightonline-mobile.zip");
        urlEdit.setSingleLine(true);
        urlEdit.setTextColor(Color.WHITE);
        urlEdit.setHintTextColor(Color.GRAY);
        urlEdit.setText(getSharedPreferences(PREFS, MODE_PRIVATE).getString(PREF_URL, GameData.DEFAULT_DATA_URL));
        root.addView(urlEdit, matchWrap());

        btnDownload = new Button(this);
        btnDownload.setText("Adresten indir ve kur");
        btnDownload.setOnClickListener(v -> downloadZip());
        root.addView(btnDownload, matchWrap());

        bar = new ProgressBar(this, null, android.R.attr.progressBarStyleHorizontal);
        bar.setMax(1000);
        bar.setVisibility(View.GONE);
        LinearLayout.LayoutParams blp = matchWrap();
        blp.topMargin = dp(20);
        root.addView(bar, blp);

        status = new TextView(this);
        status.setTextColor(Color.rgb(180, 220, 180));
        status.setPadding(0, dp(8), 0, dp(8));
        root.addView(status);

        btnCancel = new Button(this);
        btnCancel.setText("İptal");
        btnCancel.setVisibility(View.GONE);
        btnCancel.setOnClickListener(v -> cancelRequested = true);
        root.addView(btnCancel, matchWrap());

        btnStart = new Button(this);
        btnStart.setText("Oyunu başlat");
        btnStart.setOnClickListener(v -> startGame());
        LinearLayout.LayoutParams slp = matchWrap();
        slp.topMargin = dp(24);
        root.addView(btnStart, slp);

        btnDelete = new Button(this);
        btnDelete.setText("Kurulu veriyi sil");
        btnDelete.setOnClickListener(v -> confirmDelete());
        root.addView(btnDelete, matchWrap());

        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        scroll.addView(root, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT));
        setContentView(scroll);
    }

    private void refreshState() {
        boolean has = GameData.hasGameData(dataDir);
        btnStart.setEnabled(has);
        btnDelete.setEnabled(has);
        if (has)
            status.setText("Oyun verisi kurulu.");
        else
            status.setText("Oyun verisi bulunamadı.");
    }

    private void setBusy(boolean busy) {
        btnPick.setEnabled(!busy);
        btnDownload.setEnabled(!busy);
        urlEdit.setEnabled(!busy);
        btnStart.setEnabled(!busy && GameData.hasGameData(dataDir));
        btnDelete.setEnabled(!busy && GameData.hasGameData(dataDir));
        btnCancel.setVisibility(busy ? View.VISIBLE : View.GONE);
        bar.setVisibility(busy ? View.VISIBLE : View.GONE);
        if (busy) {
            bar.setIndeterminate(true);
            bar.setProgress(0);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    // ---- Eylemler ------------------------------------------------------------------------
    private void pickZip() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("*/*");
        i.putExtra(Intent.EXTRA_MIME_TYPES, new String[] { "application/zip",
                "application/x-zip-compressed", "application/octet-stream" });
        try {
            startActivityForResult(i, REQ_PICK_ZIP);
        } catch (Exception e) {
            status.setText("Dosya seçici açılamadı: " + e.getMessage());
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQ_PICK_ZIP || resultCode != RESULT_OK || data == null || data.getData() == null)
            return;
        final Uri uri = data.getData();
        runImport("Zip açılıyor", () -> {
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
            return new Source(in, size);
        });
    }

    private void downloadZip() {
        final String url = urlEdit.getText().toString().trim();
        if (url.isEmpty()) {
            status.setText("Önce bir adres yazın.");
            return;
        }
        getSharedPreferences(PREFS, MODE_PRIVATE).edit().putString(PREF_URL, url).apply();
        runImport("İndiriliyor", () -> {
            ResumingHttpStream in = new ResumingHttpStream(url, new ResumingHttpStream.Listener() {
                @Override
                public void onReconnect(int attempt, int maxAttempts, String reason) {
                    progReconnects = attempt;
                    progNote = "Bağlantı koptu (" + reason + "), kaldığı yerden yeniden bağlanıyor... "
                            + attempt + "/" + maxAttempts;
                }

                @Override
                public boolean isCancelled() {
                    return cancelRequested;
                }
            }).open();
            return new Source(in, in.total());
        });
    }

    private void confirmDelete() {
        new AlertDialog.Builder(this)
                .setTitle("Veriyi sil")
                .setMessage("Kurulu oyun verisi silinsin mi? (" + dataDir.getAbsolutePath() + ")")
                .setPositiveButton("Sil", (d, w) -> {
                    setBusy(true);
                    new Thread(() -> {
                        File[] kids = dataDir.listFiles();
                        if (kids != null)
                            for (File k : kids)
                                GameData.deleteRecursive(k);
                        runOnUiThread(() -> {
                            setBusy(false);
                            refreshState();
                        });
                    }).start();
                })
                .setNegativeButton("Vazgeç", null)
                .show();
    }

    private static final class Source {
        final InputStream in;
        final long total;

        Source(InputStream in, long total) {
            this.in = in;
            this.total = total;
        }
    }

    private interface SourceOpener {
        Source open() throws IOException;
    }

    private void runImport(final String verb, final SourceOpener opener) {
        cancelRequested = false;
        progDone = 0;
        progTotal = -1;
        progEntry = "";
        progNote = "";
        progReconnects = 0;
        speedLastTime = System.currentTimeMillis();
        speedLastBytes = 0;
        speedBps = 0;
        currentVerb = verb;
        setBusy(true);
        status.setText(verb + "...");
        worker = new Thread(() -> {
            String error = null;
            try {
                Source src = opener.open();
                progTotal = src.total;
                try (InputStream in = src.in) {
                    GameData.extractZip(in, src.total, dataDir, (done, total, entry) -> {
                        if (done > progDone)
                            progNote = "";          // veri akıyor: kopma notunu kaldır
                        progDone = done;
                        if (total > 0)
                            progTotal = total;
                        progEntry = entry;
                        return !cancelRequested;
                    });
                }
            } catch (Exception e) {
                error = cancelRequested ? "İptal edildi." : "Hata: " + e.getMessage()
                        + (progReconnects > 0 ? " (" + progReconnects + " yeniden bağlanma denemesinden sonra)" : "");
            }
            final String err = error;
            runOnUiThread(() -> {
                uiHandler.removeCallbacks(uiTick);
                setBusy(false);
                refreshState();
                if (err != null)
                    status.setText(err + "\n" + GameData.humanBytes(progDone) + " alınmıştı.");
                else if (GameData.hasGameData(dataDir))
                    status.setText("Kurulum tamamlandı (" + GameData.humanBytes(progDone)
                            + "). Oyunu başlatabilirsiniz.");
                else
                    status.setText("Zip açıldı ama içinde oyun verisi (UI klasörü) bulunamadı.");
            });
        }, "ko-import");
        worker.start();
        uiHandler.postDelayed(uiTick, 400);
    }

    private String currentVerb = "";

    private void updateProgressUi() {
        long done = progDone, total = progTotal;
        long now = System.currentTimeMillis();
        if (now - speedLastTime >= 1000) {
            speedBps = (done - speedLastBytes) * 1000.0 / (now - speedLastTime);
            speedLastTime = now;
            speedLastBytes = done;
        }
        StringBuilder sb = new StringBuilder(currentVerb);
        if (total > 0) {
            int permille = (int) Math.min(1000, done * 1000 / total);
            bar.setIndeterminate(false);
            bar.setProgress(permille);
            sb.append(String.format(Locale.ROOT, ": %%%d  (%s / %s)", permille / 10,
                    GameData.humanBytes(done), GameData.humanBytes(total)));
            if (speedBps > 1024) {
                long etaSec = (long) ((total - done) / speedBps);
                sb.append(String.format(Locale.ROOT, "\n%.1f MB/sn, kalan ~%d:%02d",
                        speedBps / (1024 * 1024), etaSec / 60, etaSec % 60));
            }
        } else {
            sb.append(": ").append(GameData.humanBytes(done));
            if (speedBps > 1024)
                sb.append(String.format(Locale.ROOT, "\n%.1f MB/sn", speedBps / (1024 * 1024)));
        }
        if (progNote.length() > 0)
            sb.append("\n").append(progNote);
        if (progEntry.length() > 0)
            sb.append("\n").append(progEntry);
        status.setText(sb.toString());
    }

    private void startGame() {
        GameData.ensureServerIni(dataDir);
        Intent i = new Intent(this, KnightOnlineActivity.class);
        i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(i);
        finish();
    }

    // ---- Yardımcılar ---------------------------------------------------------------------
    private int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
    }
}
