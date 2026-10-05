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
 * (giriş sunucusu; 1298: port 15100, 2369/ISTIRAP: port 15200) ile konuşur.
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

    private static final byte LS_SERVERLIST = (byte) 0xF5;
    private static final byte LS_NEWS = (byte) 0xF6;

    public static final class ServerInfo {
        public String ip = "";
        public String name = "";
        /** Oyuncu sayısı; -1 = dolu/erişilemez. */
        public int users;
        // 2369 ek alanları
        public String lanIp = "";
        public int serverId, groupId, playerCap, freePlayerCap, screenType;
        public String karusKing = "", karusNotice = "", elmoradKing = "", elmoradNotice = "";
    }

    public static final class NewsItem {
        public String title = "";
        public String message = "";
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

    /** Sunucu listesi: [0xF5][byte adet][adet × (str2 ip, str2 ad, int16 oyuncu)] (1298). */
    public List<ServerInfo> queryServerList() throws IOException {
        return queryServerList(GameData.PROTOCOL_1298);
    }

    /**
     * Sunucu listesi. 1298: yukarıdaki düzen. 2369 (ISTIRAP): istek [0xF5][int16 echo]; yanıt
     * [0xF5][int16 echo][byte adet][adet × (str2 lanIp, str2 ip, str2 ad, int16 oyuncu, int16 sunucuId, int16 grupId,
     * int16 kapasite, int16 serbestKapasite, byte 0, byte ekranTipi, str2 karusKral, str2 karusDuyuru,
     * str2 elmoradKral, str2 elmoradDuyuru)].
     */
    public List<ServerInfo> queryServerList(int protocol) throws IOException {
        boolean v2369 = protocol == GameData.PROTOCOL_2369;
        send(v2369 ? new byte[] { LS_SERVERLIST, 0, 0 } : new byte[] { LS_SERVERLIST });
        byte[] p = receive();
        if (p.length < (v2369 ? 4 : 2) || p[0] != LS_SERVERLIST)
            throw new IOException("Beklenmeyen sunucu listesi yanıtı");
        int[] pos = { 1 };
        if (v2369)
            pos[0] += 2; // echo
        int count = p[pos[0]++] & 0xff;
        List<ServerInfo> list = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            ServerInfo si = new ServerInfo();
            if (v2369) {
                si.lanIp = readStr2(p, pos);
                si.ip = readStr2(p, pos);
                si.name = readStr2(p, pos);
                si.users = readShort(p, pos[0]);
                pos[0] += 2;
                si.serverId = readShort(p, pos[0]);
                pos[0] += 2;
                si.groupId = readShort(p, pos[0]);
                pos[0] += 2;
                si.playerCap = readShort(p, pos[0]);
                pos[0] += 2;
                si.freePlayerCap = readShort(p, pos[0]);
                pos[0] += 2;
                pos[0] += 1; // byte 0
                si.screenType = pos[0] < p.length ? p[pos[0]] & 0xff : 0;
                pos[0] += 1;
                si.karusKing = readStr2(p, pos);
                si.karusNotice = readStr2(p, pos);
                si.elmoradKing = readStr2(p, pos);
                si.elmoradNotice = readStr2(p, pos);
            } else {
                si.ip = readStr2(p, pos);
                si.name = readStr2(p, pos);
                si.users = readShort(p, pos[0]);
                pos[0] += 2;
            }
            list.add(si);
        }
        return list;
    }

    /**
     * Haberler: [0xF6][str2 başlık][str2 içerik]. İçerik: başlık + "#\0\n" + mesaj + "\0\n#\0\n\0\n"
     * blokları (VersionManager Version.ini [NEWS]); "<empty>" = haber yok.
     */
    public List<NewsItem> queryNews() throws IOException {
        send(new byte[] { LS_NEWS });
        byte[] p = receive();
        if (p.length < 1 || p[0] != LS_NEWS)
            throw new IOException("Beklenmeyen haber yanıtı");
        int[] pos = { 1 };
        readStr2(p, pos); // "Login Notice"
        String content = readStr2(p, pos);
        return parseNews(content);
    }

    static List<NewsItem> parseNews(String content) {
        List<NewsItem> items = new ArrayList<>();
        if (content == null || content.isEmpty() || content.equals("<empty>"))
            return items;
        final String START = "#\u0000\n";
        final String END = "\u0000\n#\u0000\n\u0000\n";
        int i = 0;
        while (i < content.length()) {
            int s = content.indexOf(START, i);
            if (s < 0)
                break;
            int e = content.indexOf(END, s + START.length());
            NewsItem n = new NewsItem();
            n.title = content.substring(i, s).trim();
            n.message = (e < 0 ? content.substring(s + START.length()) : content.substring(s + START.length(), e)).trim();
            items.add(n);
            if (e < 0)
                break;
            i = e + END.length();
        }
        if (items.isEmpty()) {
            NewsItem n = new NewsItem();
            n.title = "";
            n.message = content.replace("\u0000", "").trim();
            if (!n.message.isEmpty())
                items.add(n);
        }
        return items;
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
