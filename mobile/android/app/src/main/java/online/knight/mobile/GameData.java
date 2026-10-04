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

    /** Veri dizini: önce harici (Android/data/.../files), yoksa dahili (/data/data/.../files). */
    public static File dataDir(Context ctx) {
        File ext = ctx.getExternalFilesDir(null);
        File in = ctx.getFilesDir();
        if (hasGameData(in) && !hasGameData(ext))
            return in;                       // "adb shell run-as ... cp" ile yüklenmiş veri
        return ext != null ? ext : in;
    }

    /** Dizinde oyun verisi var mı? (Server.ini ya da UI klasörü yeterli.) */
    public static boolean hasGameData(File dir) {
        if (dir == null || !dir.isDirectory())
            return false;
        File[] list = dir.listFiles();
        if (list == null)
            return false;
        for (File f : list) {
            String n = f.getName().toLowerCase(Locale.ROOT);
            if (n.equals("server.ini") || (f.isDirectory() && n.equals("ui")))
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
