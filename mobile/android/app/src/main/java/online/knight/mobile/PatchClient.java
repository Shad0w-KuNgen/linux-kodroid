package online.knight.mobile;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * Orijinal Knight Online başlatıcı (Launcher) protokolünün istemci tarafı; VersionManager
 * (giriş sunucusu, port 15100) ile konuşur.
 *
 * Çerçeve: AA 55 | int16 LE uzunluk | payload | 55 AA.  str2 = int16 LE uzunluk + baytlar.
 *  - LS_VERSION_REQ (0x01)            → [0x01][int16 sonSürüm]
 *  - LS_DOWNLOADINFO_REQ (0x02)[int16 istemciSürümü]
 *                                     → [0x02][str2 URL][str2 PATH][int16 adet][adet × str2 dosya]
 * İndirme adresi: http://{URL}{PATH}{dosya}
 */
public final class PatchClient implements AutoCloseable {
    public static final int DEFAULT_PORT = 15100;
    private static final byte LS_VERSION_REQ = 0x01;
    private static final byte LS_DOWNLOADINFO_REQ = 0x02;

    public static final class DownloadInfo {
        public String url = "";
        public String path = "";
        public final List<String> files = new ArrayList<>();

        /** Tam http adresi. URL "http://" ile başlamıyorsa eklenir, PATH başı/sonu '/' ile tamamlanır. */
        public String fileUrl(String name) {
            String base = url.trim();
            if (!base.startsWith("http://") && !base.startsWith("https://"))
                base = "http://" + base;
            while (base.endsWith("/"))
                base = base.substring(0, base.length() - 1);
            String p = path.trim().replace('\\', '/');
            if (!p.startsWith("/"))
                p = "/" + p;
            if (!p.endsWith("/"))
                p = p + "/";
            return base + p + name.trim();
        }
    }

    private final Socket socket;
    private final DataInputStream in;
    private final OutputStream out;

    public PatchClient(String host, int port, int timeoutMs) throws IOException {
        socket = new Socket();
        socket.connect(new InetSocketAddress(host, port), timeoutMs);
        socket.setSoTimeout(timeoutMs);
        socket.setTcpNoDelay(true);
        in = new DataInputStream(socket.getInputStream());
        out = socket.getOutputStream();
    }

    /** Sunucudaki son yama sürümü. */
    public int queryLatestVersion() throws IOException {
        send(new byte[] { LS_VERSION_REQ });
        byte[] p = receive();
        if (p.length < 3 || p[0] != LS_VERSION_REQ)
            throw new IOException("Beklenmeyen sürüm yanıtı");
        return readShort(p, 1);
    }

    /** Verilen istemci sürümünden sonraki yama dosyaları. */
    public DownloadInfo queryDownloadInfo(int clientVersion) throws IOException {
        send(new byte[] { LS_DOWNLOADINFO_REQ, (byte) (clientVersion & 0xff), (byte) ((clientVersion >> 8) & 0xff) });
        byte[] p = receive();
        if (p.length < 1 || p[0] != LS_DOWNLOADINFO_REQ)
            throw new IOException("Beklenmeyen indirme bilgisi yanıtı");
        int[] pos = { 1 };
        DownloadInfo info = new DownloadInfo();
        info.url = readStr2(p, pos);
        info.path = readStr2(p, pos);
        int count = readShort(p, pos[0]);
        pos[0] += 2;
        if (count < 0 || count > 4096)
            throw new IOException("Geçersiz dosya sayısı: " + count);
        for (int i = 0; i < count; i++)
            info.files.add(readStr2(p, pos));
        return info;
    }

    // ---- çerçeveleme ---------------------------------------------------------------------
    private void send(byte[] payload) throws IOException {
        ByteArrayOutputStream b = new ByteArrayOutputStream(payload.length + 6);
        b.write(0xAA);
        b.write(0x55);
        b.write(payload.length & 0xff);
        b.write((payload.length >> 8) & 0xff);
        b.write(payload);
        b.write(0x55);
        b.write(0xAA);
        out.write(b.toByteArray());
        out.flush();
    }

    private byte[] receive() throws IOException {
        // Başlık AA 55'i bul (araya çöp girmişse kaydır)
        int b1 = in.readUnsignedByte();
        int b2 = in.readUnsignedByte();
        int guard = 0;
        while (!(b1 == 0xAA && b2 == 0x55)) {
            if (++guard > 1024)
                throw new IOException("Paket başlığı bulunamadı");
            b1 = b2;
            b2 = in.readUnsignedByte();
        }
        int len = in.readUnsignedByte() | (in.readUnsignedByte() << 8);
        if (len < 0 || len > 65535)
            throw new IOException("Geçersiz paket uzunluğu");
        byte[] payload = new byte[len];
        in.readFully(payload);
        int e1 = in.readUnsignedByte(), e2 = in.readUnsignedByte();
        if (e1 != 0x55 || e2 != 0xAA)
            throw new IOException("Paket sonu bozuk");
        return payload;
    }

    private static int readShort(byte[] p, int off) throws IOException {
        if (off + 2 > p.length)
            throw new IOException("Paket kısa");
        return (short) ((p[off] & 0xff) | ((p[off + 1] & 0xff) << 8));
    }

    private static String readStr2(byte[] p, int[] pos) throws IOException {
        int len = readShort(p, pos[0]);
        pos[0] += 2;
        if (len < 0 || pos[0] + len > p.length)
            throw new IOException("Paket içindeki metin uzunluğu geçersiz");
        String s = new String(p, pos[0], len, StandardCharsets.ISO_8859_1);
        pos[0] += len;
        return s;
    }

    @Override
    public void close() {
        try {
            socket.close();
        } catch (IOException ignored) {
        }
    }

    /** Test için: ham InputStream üzerinden tek çerçeve okur (birim test). */
    static byte[] readFrame(InputStream raw) throws IOException {
        DataInputStream d = new DataInputStream(raw);
        int b1 = d.readUnsignedByte(), b2 = d.readUnsignedByte();
        if (b1 != 0xAA || b2 != 0x55)
            throw new IOException("başlık");
        int len = d.readUnsignedByte() | (d.readUnsignedByte() << 8);
        byte[] p = new byte[len];
        d.readFully(p);
        d.readUnsignedByte();
        d.readUnsignedByte();
        return p;
    }
}
