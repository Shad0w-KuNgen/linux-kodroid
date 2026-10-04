package online.knight.mobile;

import android.content.Context;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Oyun verisinin (UI/, Data/, Server.ini, ...) nerede olduğunu bulur ve bir zip akışını
 * veri dizinine açar. Android 11+ sürümlerinde adb ile Android/data altına atılan dosyaların
 * sahibi "shell" olduğu için oyun bunları okuyamıyor; bu yüzden veri uygulamanın kendisi
 * tarafından (dosya seçici ya da URL) içe aktarılır.
 */
public final class GameData {
    private GameData() {}

    /** Varsayılan giriş sunucusu (OpenKO, port 15100). Server.ini'de IP0 yoksa/127.0.0.1 ise yazılır. */
    public static final String DEFAULT_SERVER_IP = "86.105.4.195";
    /** Kurulum ekranında önerilen veri paketi adresi. */
    public static final String DEFAULT_DATA_URL = "http://" + DEFAULT_SERVER_IP + "/knightonline-mobile.zip";

    private static final String DEFAULT_SERVER_INI = "[Server]\r\nCount=1\r\nIP0=" + DEFAULT_SERVER_IP
            + "\r\n\r\n[Version]\r\nFiles=1299\r\n\r\n[Join]\r\nRegistration site=https://github.com/Open-KO/KnightOnline\r\n";

    /** Veri dizini: önce harici (Android/data/.../files), yoksa dahili (/data/data/.../files). */
    public static File dataDir(Context ctx) {
        File ext = ctx.getExternalFilesDir(null);
        File in = ctx.getFilesDir();
        if (hasGameData(in) && !hasGameData(ext))
            return in;                       // "adb shell run-as ... cp" ile yüklenmiş veri
        return ext != null ? ext : in;
    }

    /** Dizinde oyun verisi var mı? (UI klasörü ölçüt; Server.ini tek başına yetmez, uygulama onu kendisi de yazar.) */
    public static boolean hasGameData(File dir) {
        if (dir == null || !dir.isDirectory())
            return false;
        File[] list = dir.listFiles();
        if (list == null)
            return false;
        for (File f : list) {
            String n = f.getName().toLowerCase(Locale.ROOT);
            if (f.isDirectory() && n.equals("ui"))
                return true;
        }
        return false;
    }

    public interface Progress {
        /** @return false ise iptal edilir. */
        boolean onProgress(long bytesDone, long bytesTotal, String currentEntry);
    }

    /**
     * Zip akışını dizine açar. Girdi adlarındaki '\' '/' olarak düzeltilir, ".." içeren
     * yollar atlanır. bytesTotal bilinmiyorsa -1 geçin.
     */
    public static void extractZip(InputStream raw, long bytesTotal, File dir, Progress progress)
            throws IOException {
        if (!dir.isDirectory() && !dir.mkdirs())
            throw new IOException("Dizin oluşturulamadı: " + dir);
        CountingInputStream counting = new CountingInputStream(raw);
        ZipInputStream zip = new ZipInputStream(counting);
        byte[] buf = new byte[256 * 1024];
        String canonicalRoot = dir.getCanonicalPath();
        ZipEntry entry;
        int entries = 0;
        while ((entry = zip.getNextEntry()) != null) {
            String name = entry.getName().replace('\\', '/');
            while (name.startsWith("/"))
                name = name.substring(1);
            if (name.isEmpty() || name.contains("..") || name.startsWith("__MACOSX/")) {
                zip.closeEntry();
                continue;
            }
            File out = new File(dir, name);
            if (!out.getCanonicalPath().startsWith(canonicalRoot + File.separator)) {
                zip.closeEntry();
                continue;
            }
            if (entry.isDirectory()) {
                out.mkdirs();
            } else {
                File parent = out.getParentFile();
                if (parent != null && !parent.isDirectory() && !parent.mkdirs())
                    throw new IOException("Dizin oluşturulamadı: " + parent);
                try (java.io.FileOutputStream fos = new java.io.FileOutputStream(out)) {
                    int n;
                    while ((n = zip.read(buf)) > 0) {
                        fos.write(buf, 0, n);
                        if (progress != null && !progress.onProgress(counting.count, bytesTotal, name))
                            throw new IOException("İptal edildi");
                    }
                }
            }
            zip.closeEntry();
            entries++;
            if (progress != null && !progress.onProgress(counting.count, bytesTotal, name))
                throw new IOException("İptal edildi");
        }
        if (entries == 0)
            throw new IOException("Zip boş ya da geçersiz");
        flattenSingleTopDir(dir);
        ensureServerIni(dir);
    }

    /** Dizindeki Server.ini'yi (harf duyarsız) bulur; yoksa null. */
    public static File findServerIni(File dir) {
        File[] list = dir.listFiles();
        if (list != null)
            for (File f : list)
                if (f.isFile() && f.getName().equalsIgnoreCase("server.ini"))
                    return f;
        return null;
    }

    private static boolean isPlaceholderIp(String ip) {
        if (ip == null)
            return true;
        ip = ip.trim();
        return ip.isEmpty() || ip.equals("127.0.0.1") || ip.equals("0.0.0.0") || ip.equalsIgnoreCase("localhost");
    }

    /**
     * Server.ini yoksa varsayılanı yazar; varsa ve [Server] IP0 boş ya da yerel adres (127.0.0.1) ise
     * IP0'ı DEFAULT_SERVER_IP yapar. Başka bir sunucu adresi elle yazılmışsa dokunmaz.
     * @return dosya yazıldı/değişti mi
     */
    public static boolean ensureServerIni(File dir) {
        if (dir == null)
            return false;
        File ini = findServerIni(dir);
        try {
            if (ini == null) {
                if (!dir.isDirectory() && !dir.mkdirs())
                    return false;
                writeText(new File(dir, "Server.ini"), DEFAULT_SERVER_INI);
                return true;
            }
            java.util.List<String> lines = readLines(ini);
            boolean inServer = false, changed = false, sawServer = false, sawIp0 = false, sawCount = false;
            for (int i = 0; i < lines.size(); i++) {
                String t = lines.get(i).trim();
                if (t.startsWith("[")) {
                    if (inServer && !sawIp0) {
                        lines.add(i, "IP0=" + DEFAULT_SERVER_IP);
                        sawIp0 = changed = true;
                        i++;
                    }
                    inServer = t.equalsIgnoreCase("[Server]");
                    if (inServer)
                        sawServer = true;
                    continue;
                }
                if (!inServer || t.isEmpty() || t.startsWith(";"))
                    continue;
                int eq = t.indexOf('=');
                if (eq < 0)
                    continue;
                String key = t.substring(0, eq).trim(), val = t.substring(eq + 1);
                if (key.equalsIgnoreCase("IP0")) {
                    sawIp0 = true;
                    if (isPlaceholderIp(val)) {
                        lines.set(i, "IP0=" + DEFAULT_SERVER_IP);
                        changed = true;
                    }
                } else if (key.equalsIgnoreCase("Count")) {
                    sawCount = true;
                    int n = 0;
                    try {
                        n = Integer.parseInt(val.trim());
                    } catch (NumberFormatException ignored) {
                    }
                    if (n < 1) {
                        lines.set(i, "Count=1");
                        changed = true;
                    }
                }
            }
            if (inServer && !sawIp0) {
                lines.add("IP0=" + DEFAULT_SERVER_IP);
                sawIp0 = changed = true;
            }
            if (!sawServer) {
                lines.add("");
                lines.add("[Server]");
                lines.add("Count=1");
                lines.add("IP0=" + DEFAULT_SERVER_IP);
                sawCount = changed = true;
            } else if (!sawCount) {
                // [Server] var ama Count yok: başlığın hemen altına ekle
                for (int i = 0; i < lines.size(); i++)
                    if (lines.get(i).trim().equalsIgnoreCase("[Server]")) {
                        lines.add(i + 1, "Count=1");
                        break;
                    }
                changed = true;
            }
            if (changed) {
                StringBuilder sb = new StringBuilder();
                for (String l : lines)
                    sb.append(l).append("\r\n");
                writeText(ini, sb.toString());
            }
            return changed;
        } catch (IOException e) {
            return false;
        }
    }

    private static java.util.List<String> readLines(File f) throws IOException {
        java.util.List<String> out = new java.util.ArrayList<>();
        try (java.io.BufferedReader r = new java.io.BufferedReader(
                new java.io.InputStreamReader(new java.io.FileInputStream(f), "ISO-8859-1"))) {
            String l;
            while ((l = r.readLine()) != null)
                out.add(l);
        }
        return out;
    }

    private static void writeText(File f, String text) throws IOException {
        try (java.io.FileOutputStream fos = new java.io.FileOutputStream(f)) {
            fos.write(text.getBytes("ISO-8859-1"));
        }
    }

    /**
     * Zip "KnightOnline/UI/..." gibi tek bir üst klasörle paketlenmişse içeriğini kök dizine taşır.
     */
    static void flattenSingleTopDir(File dir) {
        if (hasGameData(dir))
            return;
        File[] list = dir.listFiles();
        if (list == null)
            return;
        for (File sub : list) {
            if (!sub.isDirectory() || !hasGameData(sub))
                continue;
            File[] children = sub.listFiles();
            if (children == null)
                continue;
            for (File c : children) {
                File dst = new File(dir, c.getName());
                if (dst.exists())
                    deleteRecursive(dst);
                c.renameTo(dst);
            }
            sub.delete();
            return;
        }
    }

    public static void deleteRecursive(File f) {
        File[] kids = f.listFiles();
        if (kids != null)
            for (File k : kids)
                deleteRecursive(k);
        f.delete();
    }

    public static String humanBytes(long b) {
        if (b < 0)
            return "?";
        if (b < 1024L * 1024)
            return String.format(Locale.ROOT, "%d KB", b / 1024);
        if (b < 1024L * 1024 * 1024)
            return String.format(Locale.ROOT, "%.1f MB", b / (1024.0 * 1024));
        return String.format(Locale.ROOT, "%.2f GB", b / (1024.0 * 1024 * 1024));
    }

    /** Okunan bayt sayısını sayan basit sarmalayıcı (ilerleme çubuğu için). */
    static final class CountingInputStream extends java.io.FilterInputStream {
        long count;

        CountingInputStream(InputStream in) {
            super(in);
        }

        @Override
        public int read() throws IOException {
            int r = in.read();
            if (r >= 0)
                count++;
            return r;
        }

        @Override
        public int read(byte[] b, int off, int len) throws IOException {
            int r = in.read(b, off, len);
            if (r > 0)
                count += r;
            return r;
        }

        @Override
        public long skip(long n) throws IOException {
            long r = in.skip(n);
            count += r;
            return r;
        }
    }
}
