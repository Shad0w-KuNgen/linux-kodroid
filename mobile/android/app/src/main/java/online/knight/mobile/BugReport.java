package online.knight.mobile;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.util.DisplayMetrics;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * Hata raporu: Log.txt, gpu.txt (GL_RENDERER/GL_EXTENSIONS), Server.ini/Option.ini, son logcat
 * (yalnız bu uygulamanın kayıtları) ve cihaz bilgisi tek bir zip'te; Android paylaşım menüsüyle
 * (WhatsApp, e-posta, Drive) gönderilir. Kullanıcı Android/data klasörüne erişemediği için
 * test hataları bu yoldan toplanır.
 */
public final class BugReport {
    private BugReport() {}

    public static File build(Context ctx, File dataDir) throws IOException {
        File dir = new File(ctx.getCacheDir(), "reports");
        dir.mkdirs();
        // eski raporları temizle
        File[] old = dir.listFiles();
        if (old != null)
            for (File f : old)
                f.delete();
        String stamp = new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.ROOT).format(new Date());
        File out = new File(dir, "ko-mobile-rapor-" + stamp + ".zip");
        try (ZipOutputStream zip = new ZipOutputStream(new FileOutputStream(out))) {
            putText(zip, "cihaz.txt", deviceInfo(ctx, dataDir));
            putFile(zip, "Log.txt", find(dataDir, "Log.txt"));
            putFile(zip, "gpu.txt", find(dataDir, "gpu.txt"));
            putFile(zip, "Server.ini", find(dataDir, "Server.ini"));
            putFile(zip, "Option.ini", find(dataDir, "Option.ini"));
            putText(zip, "logcat.txt", logcat());
            putText(zip, "veri-dizini.txt", listDir(dataDir, 2));
        }
        return out;
    }

    public static Intent shareIntent(Context ctx, File zip) {
        Uri uri = androidx.core.content.FileProvider.getUriForFile(ctx, ctx.getPackageName() + ".fileprovider", zip);
        Intent i = new Intent(Intent.ACTION_SEND);
        i.setType("application/zip");
        i.putExtra(Intent.EXTRA_STREAM, uri);
        i.putExtra(Intent.EXTRA_SUBJECT, "Knight Online Mobile hata raporu " + zip.getName());
        i.putExtra(Intent.EXTRA_TEXT, "Knight Online Mobile hata raporu (Log.txt, gpu.txt, logcat, cihaz bilgisi).");
        i.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        return Intent.createChooser(i, "Hata raporunu gönder");
    }

    // ---- içerik --------------------------------------------------------------------------
    static String deviceInfo(Context ctx, File dataDir) {
        StringBuilder sb = new StringBuilder();
        sb.append("Tarih: ").append(new Date()).append('\n');
        sb.append("Uygulama: ").append(ctx.getPackageName()).append(' ')
                .append(ApkUpdater.installedVersionName(ctx)).append(" (").append(ApkUpdater.installedVersionCode(ctx)).append(")\n");
        sb.append("Android: ").append(Build.VERSION.RELEASE).append(" (API ").append(Build.VERSION.SDK_INT).append(")\n");
        sb.append("Cihaz: ").append(Build.MANUFACTURER).append(' ').append(Build.MODEL).append(" (").append(Build.DEVICE)
                .append(", ").append(Build.BOARD).append(", ").append(Build.HARDWARE).append(")\n");
        sb.append("ABI: ").append(Build.SUPPORTED_ABIS.length > 0 ? Build.SUPPORTED_ABIS[0] : "?").append('\n');
        DisplayMetrics dm = ctx.getResources().getDisplayMetrics();
        sb.append("Ekran: ").append(dm.widthPixels).append('x').append(dm.heightPixels).append(" @").append(dm.densityDpi).append(" dpi\n");
        try {
            ActivityManager am = (ActivityManager) ctx.getSystemService(Context.ACTIVITY_SERVICE);
            ActivityManager.MemoryInfo mi = new ActivityManager.MemoryInfo();
            am.getMemoryInfo(mi);
            sb.append("Bellek: toplam ").append(mi.totalMem / (1024 * 1024)).append(" MB, boş ").append(mi.availMem / (1024 * 1024))
                    .append(" MB, düşük=").append(mi.lowMemory).append('\n');
        } catch (Exception ignored) {
        }
        sb.append("Veri dizini: ").append(dataDir).append('\n');
        File ini = GameData.findServerIni(dataDir);
        sb.append("Sunucu: ").append(GameData.getIniValue(ini, "Server", "IP0", "?")).append(", veri sürümü ")
                .append(GameData.getIniValue(ini, "Version", "Files", "?")).append('\n');
        return sb.toString();
    }

    static String logcat() {
        StringBuilder sb = new StringBuilder();
        try {
            // Android 4.1+ bir uygulama yalnız kendi kayıtlarını okuyabilir; izin gerekmez.
            Process p = Runtime.getRuntime().exec(new String[] { "logcat", "-d", "-v", "time", "-t", "4000" });
            try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream(), StandardCharsets.UTF_8))) {
                String line;
                while ((line = r.readLine()) != null) {
                    sb.append(line).append('\n');
                    if (sb.length() > 2 * 1024 * 1024)
                        break;
                }
            }
            p.destroy();
        } catch (Exception e) {
            sb.append("logcat alınamadı: ").append(e).append('\n');
        }
        return sb.toString();
    }

    static String listDir(File dir, int depth) {
        StringBuilder sb = new StringBuilder();
        listDir(dir, depth, "", sb);
        return sb.toString();
    }

    private static void listDir(File dir, int depth, String indent, StringBuilder sb) {
        File[] list = dir == null ? null : dir.listFiles();
        if (list == null) {
            sb.append(indent).append("(okunamadı: ").append(dir).append(")\n");
            return;
        }
        java.util.Arrays.sort(list);
        int shown = 0;
        for (File f : list) {
            if (++shown > 200) {
                sb.append(indent).append("... (").append(list.length - 200).append(" daha)\n");
                break;
            }
            sb.append(indent).append(f.getName());
            if (f.isDirectory()) {
                File[] kids = f.listFiles();
                sb.append("/  (").append(kids == null ? "?" : String.valueOf(kids.length)).append(" öğe)\n");
                if (depth > 1 && kids != null && kids.length <= 60)
                    listDir(f, depth - 1, indent + "  ", sb);
            } else {
                sb.append("  ").append(f.length()).append(" B\n");
            }
        }
    }

    static File find(File dir, String name) {
        File[] list = dir == null ? null : dir.listFiles();
        if (list != null)
            for (File f : list)
                if (f.isFile() && f.getName().equalsIgnoreCase(name))
                    return f;
        return null;
    }

    private static void putText(ZipOutputStream zip, String name, String text) throws IOException {
        zip.putNextEntry(new ZipEntry(name));
        zip.write(text.getBytes(StandardCharsets.UTF_8));
        zip.closeEntry();
    }

    private static void putFile(ZipOutputStream zip, String name, File f) throws IOException {
        if (f == null || !f.isFile()) {
            putText(zip, name + ".yok.txt", "dosya yok");
            return;
        }
        zip.putNextEntry(new ZipEntry(name));
        byte[] buf = new byte[64 * 1024];
        long max = 8L * 1024 * 1024; // Log.txt çok büyümüşse son 8 MB
        long skip = Math.max(0, f.length() - max);
        try (InputStream in = new FileInputStream(f)) {
            if (skip > 0) {
                in.skip(skip);
                zip.write(("[... ilk " + skip + " bayt atlandı ...]\n").getBytes(StandardCharsets.UTF_8));
            }
            int n;
            while ((n = in.read(buf)) > 0)
                zip.write(buf, 0, n);
        }
        zip.closeEntry();
    }
}
