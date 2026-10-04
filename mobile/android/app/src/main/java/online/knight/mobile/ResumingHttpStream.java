package online.knight.mobile;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * HTTP indirme akışı: bağlantı koparsa (mobil veri kesintisi, "connection abort" vb.)
 * kaldığı bayttan "Range: bytes=N-" ile yeniden bağlanır ve okuyucuya (ZipInputStream)
 * hiç kopmamış gibi devam eder. Sunucu Range desteklemiyorsa (200 döner) eksik baytlar
 * okunup atılarak aynı konuma gelinir.
 */
public final class ResumingHttpStream extends InputStream {
    public interface Listener {
        /** Yeniden bağlanma denemesi (kullanıcıya göstermek için). */
        void onReconnect(int attempt, int maxAttempts, String reason);

        /** true dönerse beklemeden iptal edilir. */
        boolean isCancelled();
    }

    private static final int MAX_ATTEMPTS = 30;        // ~ toplam 10+ dakika bekleme
    private static final int CONNECT_TIMEOUT_MS = 30000;
    private static final int READ_TIMEOUT_MS = 45000;

    private final String url;
    private final Listener listener;
    private HttpURLConnection conn;
    private InputStream cur;
    private long pos;              // şu ana kadar teslim edilen bayt
    private long total = -1;       // toplam uzunluk (bilinmiyorsa -1)
    private int attempts;
    private boolean eof;

    public ResumingHttpStream(String url, Listener listener) {
        this.url = url;
        this.listener = listener;
    }

    /** İlk bağlantıyı kurar; HTTP hatasında IOException fırlatır. */
    public ResumingHttpStream open() throws IOException {
        connect();
        return this;
    }

    public long total() {
        return total;
    }

    public long position() {
        return pos;
    }

    private void connect() throws IOException {
        closeCurrent();
        HttpURLConnection c = (HttpURLConnection) new URL(url).openConnection();
        c.setConnectTimeout(CONNECT_TIMEOUT_MS);
        c.setReadTimeout(READ_TIMEOUT_MS);
        c.setInstanceFollowRedirects(true);
        c.setRequestProperty("User-Agent", "KnightOnlineMobile/0.1");
        c.setRequestProperty("Accept-Encoding", "identity");
        if (pos > 0)
            c.setRequestProperty("Range", "bytes=" + pos + "-");
        c.connect();
        int code = c.getResponseCode();
        long skip = 0;
        if (code == 206) {
            // Content-Range: bytes start-end/total
            String cr = c.getHeaderField("Content-Range");
            if (cr != null) {
                int slash = cr.lastIndexOf('/');
                if (slash > 0 && !cr.substring(slash + 1).trim().equals("*")) {
                    try {
                        total = Long.parseLong(cr.substring(slash + 1).trim());
                    } catch (NumberFormatException ignored) {
                    }
                }
                int sp = cr.indexOf(' '), dash = cr.indexOf('-');
                if (sp > 0 && dash > sp) {
                    try {
                        long start = Long.parseLong(cr.substring(sp + 1, dash).trim());
                        if (start < pos)
                            skip = pos - start;     // sunucu daha geriden başlattı
                        else if (start > pos)
                            throw new IOException("Sunucu Range isteğini yanlış yanıtladı");
                    } catch (NumberFormatException ignored) {
                    }
                }
            }
        } else if (code / 100 == 2) {
            if (pos > 0)
                skip = pos;                           // Range desteklenmiyor: baştan al, atla
            if (total < 0)
                total = c.getContentLengthLong();
        } else {
            c.disconnect();
            throw new IOException("HTTP " + code);
        }
        conn = c;
        cur = c.getInputStream();
        if (skip > 0) {
            byte[] junk = new byte[64 * 1024];
            while (skip > 0) {
                int n = cur.read(junk, 0, (int) Math.min(junk.length, skip));
                if (n < 0)
                    throw new IOException("Akış atlama sırasında bitti");
                skip -= n;
            }
        }
    }

    private void closeCurrent() {
        if (cur != null) {
            try {
                cur.close();
            } catch (IOException ignored) {
            }
            cur = null;
        }
        if (conn != null) {
            conn.disconnect();
            conn = null;
        }
    }

    @Override
    public int read() throws IOException {
        byte[] b = new byte[1];
        int n = read(b, 0, 1);
        return n <= 0 ? -1 : (b[0] & 0xff);
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        if (eof)
            return -1;
        if (len == 0)
            return 0;
        while (true) {
            try {
                if (cur == null)
                    connect();
                int n = cur.read(b, off, len);
                if (n < 0) {
                    if (total >= 0 && pos < total)
                        throw new IOException("Akış erken bitti (" + pos + "/" + total + ")");
                    eof = true;
                    return -1;
                }
                pos += n;
                attempts = 0;
                return n;
            } catch (IOException e) {
                closeCurrent();
                if (total >= 0 && pos >= total) {       // zaten her şey alındı
                    eof = true;
                    return -1;
                }
                attempts++;
                if (attempts > MAX_ATTEMPTS || (listener != null && listener.isCancelled()))
                    throw e;
                if (listener != null)
                    listener.onReconnect(attempts, MAX_ATTEMPTS, e.getMessage());
                // Üstel bekleme: 2, 4, 8, ... en çok 30 sn; iptal kontrolüyle
                long wait = Math.min(30000L, 2000L << Math.min(attempts - 1, 4));
                long until = System.currentTimeMillis() + wait;
                while (System.currentTimeMillis() < until) {
                    if (listener != null && listener.isCancelled())
                        throw new IOException("İptal edildi");
                    try {
                        Thread.sleep(250);
                    } catch (InterruptedException ie) {
                        throw new IOException("İptal edildi");
                    }
                }
            }
        }
    }

    @Override
    public void close() {
        closeCurrent();
        eof = true;
    }
}
