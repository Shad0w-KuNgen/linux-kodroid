package online.knight.mobile;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;

import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

/**
 * APK kendini güncelleme. Sunucudaki bir JSON bildirimi (apk.json) okunur:
 * <pre>
 * { "versionCode": 42, "versionName": "0.1.42",
 *   "url": "http://86.105.4.195/ko/mobile/KnightOnline.apk",
 *   "sha256": "…", "notes": "Neler değişti" }
 * </pre>
 * versionCode kurulu olandan büyükse APK kaldığı yerden devam eden indiriciyle önbelleğe alınır,
 * sha256 doğrulanır ve Android paket yükleyicisi açılır (REQUEST_INSTALL_PACKAGES + FileProvider).
 * CI her derlemede apk.json (versionCode/versionName/sha256) üretir; sunucuya APK ile birlikte konur.
 */
public final class ApkUpdater {
    public static final String DEFAULT_MANIFEST_URL = "http://" + GameData.DEFAULT_SERVER_IP + "/ko/mobile/apk.json";

    public static final class Manifest {
        public int versionCode;
        public String versionName = "";
        public String url = "";
        public String sha256 = "";
        public String notes = "";
    }

    private ApkUpdater() {}

    public static int installedVersionCode(Context ctx) {
        try {
            PackageInfo pi = ctx.getPackageManager().getPackageInfo(ctx.getPackageName(), 0);
            return Build.VERSION.SDK_INT >= 28 ? (int) pi.getLongVersionCode() : pi.versionCode;
        } catch (PackageManager.NameNotFoundException e) {
            return 0;
        }
    }

    public static String installedVersionName(Context ctx) {
        try {
            PackageInfo pi = ctx.getPackageManager().getPackageInfo(ctx.getPackageName(), 0);
            return pi.versionName == null ? "?" : pi.versionName;
        } catch (PackageManager.NameNotFoundException e) {
            return "?";
        }
    }

    /** Bildirimi indirir (kısa zaman aşımı; başarısızlık oyunu engellemez). */
    public static Manifest fetchManifest(String manifestUrl, int timeoutMs) throws IOException {
        HttpURLConnection c = (HttpURLConnection) new URL(manifestUrl).openConnection();
        c.setConnectTimeout(timeoutMs);
        c.setReadTimeout(timeoutMs);
        c.setInstanceFollowRedirects(true);
        c.setRequestProperty("User-Agent", "KnightOnlineMobile/0.1");
        c.connect();
        int code = c.getResponseCode();
        if (code / 100 != 2)
            throw new IOException("HTTP " + code);
        byte[] buf = new byte[4096];
        java.io.ByteArrayOutputStream bos = new java.io.ByteArrayOutputStream();
        try (InputStream in = c.getInputStream()) {
            int n;
            while ((n = in.read(buf)) > 0 && bos.size() < 64 * 1024)
                bos.write(buf, 0, n);
        }
        try {
            JSONObject o = new JSONObject(new String(bos.toByteArray(), StandardCharsets.UTF_8));
            Manifest m = new Manifest();
            m.versionCode = o.optInt("versionCode", 0);
            m.versionName = o.optString("versionName", "");
            m.url = o.optString("url", "");
            m.sha256 = o.optString("sha256", "").trim().toLowerCase(java.util.Locale.ROOT);
            m.notes = o.optString("notes", "");
            if (m.versionCode <= 0 || m.url.isEmpty())
                throw new IOException("apk.json eksik alan (versionCode/url)");
            // Göreli adres: bildirimin bulunduğu dizine göre
            if (!m.url.startsWith("http://") && !m.url.startsWith("https://"))
                m.url = new URL(new URL(manifestUrl), m.url).toString();
            return m;
        } catch (org.json.JSONException e) {
            throw new IOException("apk.json çözümlenemedi: " + e.getMessage());
        }
    }

    /** İndirilen APK'nın konacağı dosya (FileProvider "apk" yolu altında). */
    public static File apkFile(Context ctx, Manifest m) {
        File dir = new File(ctx.getCacheDir(), "apk");
        dir.mkdirs();
        return new File(dir, "KnightOnline-" + m.versionCode + ".apk");
    }

    public static String sha256Hex(File f) throws IOException {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] buf = new byte[256 * 1024];
            try (FileInputStream in = new FileInputStream(f)) {
                int n;
                while ((n = in.read(buf)) > 0)
                    md.update(buf, 0, n);
            }
            StringBuilder sb = new StringBuilder();
            for (byte b : md.digest())
                sb.append(String.format(java.util.Locale.ROOT, "%02x", b));
            return sb.toString();
        } catch (java.security.NoSuchAlgorithmException e) {
            throw new IOException(e);
        }
    }

    /** Paket yükleyicisini açar. Android 8+ "bilinmeyen kaynak" iznini kullanıcıdan ister. */
    public static Intent installIntent(Context ctx, File apk) {
        Uri uri = androidx.core.content.FileProvider.getUriForFile(ctx, ctx.getPackageName() + ".fileprovider", apk);
        Intent i = new Intent(Intent.ACTION_VIEW);
        i.setDataAndType(uri, "application/vnd.android.package-archive");
        i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_ACTIVITY_NEW_TASK);
        return i;
    }

    public static boolean canInstallUnknown(Context ctx) {
        return Build.VERSION.SDK_INT < 26 || ctx.getPackageManager().canRequestPackageInstalls();
    }

    public static Intent unknownSourcesSettingsIntent(Context ctx) {
        Intent i = new Intent(android.provider.Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES);
        i.setData(Uri.parse("package:" + ctx.getPackageName()));
        return i;
    }
}
