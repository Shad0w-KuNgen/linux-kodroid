#!/usr/bin/env python3
"""ui_tool.py — 2369 istemcisinin UI paketi (UI/ui.hdr + ui.src) ve .istirap UIF'leri için araç.

  ui_tool.py list    <UI dizini> [filtre]        # paket dizini (ad, ofset, boyut)
  ui_tool.py extract <UI dizini> <ad> <çıkış>    # paketten tek dosya çıkar
  ui_tool.py istirap <dosya.istirap> <çıkış.uif> # Pearl Guard dcpUIF çözümü
  ui_tool.py info    <dosya>                      # ilk 64 baytın hex dökümü (UIF başlığı kontrolü)
  ui_tool.py uif     <dosya.uif>                  # UIF ağacını 1264 biçimine göre yürü; sapma noktasında hex bağlamı

Paket: ui.hdr = u32 kayıt sayısı, kayıt: u32 adUzunluk | ad | u32 ofset | u32 boyut.
ui.src kaydı: u32 yolUzunluk | özgün yol | dosya baytları. Boyutun yol başlığını içerip içermediği ardışık
kayıtlardan çıkarılır. .istirap: ilk 4 bayt düz; blok = boyut çiftse 32, tekse 31; her blok RC4 başından;
anahtar = SHA1(parola[:29])[:16] (3 - AntiCheat-Source/Pearl Guard/Pearl.cpp LoadCrypto).
"""
import hashlib
import os
import struct
import sys

PASSWORD = b"(A;dq1DPVFgVs1Aez$VS3R0hge@NvM_TJvblD4.af0h@r4bUzp"[:29]
KEY = hashlib.sha1(PASSWORD).digest()[:16]


def rc4(key, data):
    s = list(range(256))
    j = 0
    for k in range(256):
        j = (j + s[k] + key[k % len(key)]) & 0xFF
        s[k], s[j] = s[j], s[k]
    i = j = 0
    out = bytearray(len(data))
    for n, b in enumerate(data):
        i = (i + 1) & 0xFF
        j = (j + s[i]) & 0xFF
        s[i], s[j] = s[j], s[i]
        out[n] = b ^ s[(s[i] + s[j]) & 0xFF]
    return bytes(out)


def istirap_decrypt(data):
    if len(data) <= 4:
        return data
    block = 32 if len(data) % 2 == 0 else 31
    out = bytearray(data[:4])
    for pos in range(4, len(data), block):
        out += rc4(KEY, data[pos:pos + block])
    return bytes(out)


def find(dirpath, name):
    for f in os.listdir(dirpath):
        if f.lower() == name.lower():
            return os.path.join(dirpath, f)
    return None


def load_index(uidir):
    hdr = open(find(uidir, 'ui.hdr'), 'rb').read()
    count = struct.unpack_from('<I', hdr, 0)[0]
    pos = 4
    entries = {}
    for _ in range(count):
        n = struct.unpack_from('<I', hdr, pos)[0]
        pos += 4
        name = hdr[pos:pos + n].decode('latin-1')
        pos += n
        off, size = struct.unpack_from('<II', hdr, pos)
        pos += 8
        entries[name.lower()] = (name, off, size)
    srcpath = find(uidir, 'ui.src')
    # boyut anlamı
    ordered = sorted(entries.values(), key=lambda e: e[1])
    includes_header = True
    if len(ordered) >= 2:
        with open(srcpath, 'rb') as f:
            f.seek(ordered[0][1])
            plen = struct.unpack('<I', f.read(4))[0]
        if ordered[1][1] == ordered[0][1] + ordered[0][2]:
            includes_header = True
        elif ordered[1][1] == ordered[0][1] + 4 + plen + ordered[0][2]:
            includes_header = False
        else:
            print(f'# uyarı: boyut anlamı çıkarılamadı (ofset1={ordered[1][1]}, ofset0+boyut={ordered[0][1] + ordered[0][2]}, yol={plen})')
    return entries, srcpath, includes_header


def extract(uidir, name):
    entries, srcpath, inc = load_index(uidir)
    e = entries.get(name.lower())
    if not e:
        return None, None
    with open(srcpath, 'rb') as f:
        f.seek(e[1])
        plen = struct.unpack('<I', f.read(4))[0]
        path = f.read(plen).decode('latin-1')
        dlen = (e[2] - 4 - plen) if inc else e[2]
        return f.read(dlen), path


# ---- UIF (1264 biçimi) yürüyücü: OpenKO CN3UI*::Load okuma sırasının birebiri ----
UI_TYPES = {0: 'base', 1: 'button', 2: 'static', 3: 'progress', 4: 'image', 5: 'scrollbar', 6: 'string', 7: 'trackbar',
            8: 'edit', 9: 'area', 10: 'tooltip', 11: 'icon', 12: 'iconmgr', 13: 'iconslot', 14: 'list'}


class UifError(Exception):
    pass


class Walker:
    def __init__(self, data):
        self.d = data
        self.pos = 0
        self.lines = []

    def u32(self):
        if self.pos + 4 > len(self.d):
            raise UifError(f'EOF @{self.pos}')
        v = struct.unpack_from('<I', self.d, self.pos)[0]
        self.pos += 4
        return v

    def i32(self):
        return struct.unpack('<i', struct.pack('<I', self.u32()))[0]

    def i16(self):
        if self.pos + 2 > len(self.d):
            raise UifError(f'EOF @{self.pos}')
        v = struct.unpack_from('<h', self.d, self.pos)[0]
        self.pos += 2
        return v

    def lstr(self, what, maxlen=1024):
        n = self.i32()
        if n < 0 or n > maxlen:
            raise UifError(f'{what}: geçersiz uzunluk {n} @{self.pos - 4}')
        s = self.d[self.pos:self.pos + n].decode('latin-1')
        self.pos += n
        return s

    def skip(self, n):
        self.pos += n

    def base(self, depth, typ):
        start = self.pos
        name = self.lstr('N3 ad', 256)
        cc = self.i16()
        self.i16()
        if cc < 0 or cc > 4096:
            raise UifError(f'çocuk sayısı {cc} @{self.pos - 4}')
        kids = []
        for i in range(cc):
            t = self.u32()
            if t not in UI_TYPES or t >= 10 and t != 14:
                raise UifError(f'bilinmeyen UI türü {t} (çocuk {i}/{cc}) @{self.pos - 4}')
            kids.append(self.node(depth + 1, t))
        ident = self.lstr('ID', 128)
        self.skip(16 + 16 + 4 + 4)  # region, movable, style, reserved
        self.lstr('tooltip', 1024)
        self.lstr('ses açılış', 1024)
        self.lstr('ses kapanış', 1024)
        self.lines.append('  ' * depth + f'{UI_TYPES[typ]} "{ident}" @{start} ({cc} çocuk)')
        return ident

    def node(self, depth, typ):
        ident = self.base(depth, typ)
        if typ == 4:  # image
            self.lstr('doku adı', 1024)
            self.skip(16 + 4)
        elif typ == 6:  # string
            if self.lstr('yazı tipi', 32):
                self.skip(8)
            self.skip(4)
            self.lstr('metin', 8192)
            self.i32()  # satır aralığı (1264)
        elif typ == 1:  # button
            self.skip(16)
            self.lstr('ses on', 1024)
            self.lstr('ses click', 1024)
        elif typ == 2:  # static
            self.lstr('ses click', 1024)
        elif typ == 8:  # edit = static + typing
            self.lstr('ses click', 1024)
            self.lstr('ses typing', 1024)
        elif typ == 9:  # area
            self.i32()
        elif typ == 14:  # list
            if self.lstr('yazı tipi', 32):
                self.skip(16)
        return ident


def cmd_uif(path):
    d = open(path, 'rb').read()
    w = Walker(d)
    try:
        w.node(0, 0)
        ok = w.pos == len(d)
        print('\n'.join(w.lines))
        print(f'# {"TAM" if ok else "KISMİ"}: {w.pos}/{len(d)} bayt tüketildi')
        return 0 if ok else 1
    except UifError as e:
        print('\n'.join(w.lines))
        print(f'# HATA: {e}')
        ctx = max(0, w.pos - 48)
        for i in range(ctx, min(len(d), w.pos + 48), 16):
            c = d[i:i + 16]
            print(f'{i:08x}  {c.hex(" "):48s}  {"".join(chr(b) if 32 <= b < 127 else "." for b in c)}')
        return 1


def main(argv):
    if len(argv) < 3:
        print(__doc__)
        return 2
    cmd = argv[1]
    if cmd == 'list':
        entries, srcpath, inc = load_index(argv[2])
        flt = argv[3].lower() if len(argv) > 3 else ''
        print(f'# {len(entries)} kayıt, boyut başlık {"dahil" if inc else "hariç"}, kaynak {srcpath}')
        for name, off, size in sorted(entries.values(), key=lambda e: e[0].lower()):
            if flt in name.lower():
                print(f'{name:48s} ofset={off:>10d} boyut={size:>9d}')
    elif cmd == 'extract':
        data, path = extract(argv[2], argv[3])
        if data is None:
            print('kayıt yok:', argv[3])
            return 1
        open(argv[4], 'wb').write(data)
        print(f'{argv[3]} -> {argv[4]} ({len(data)} bayt, özgün yol {path})')
    elif cmd == 'istirap':
        data = istirap_decrypt(open(argv[2], 'rb').read())
        open(argv[3], 'wb').write(data)
        print(f'{argv[2]} -> {argv[3]} ({len(data)} bayt) ilk16={data[:16].hex()}')
    elif cmd == 'uif':
        return cmd_uif(argv[2])
    elif cmd == 'info':
        d = open(argv[2], 'rb').read()
        print(f'{argv[2]}: {len(d)} bayt')
        for i in range(0, min(64, len(d)), 16):
            c = d[i:i + 16]
            print(f'{i:04x}  {c.hex(" "):48s}  {"".join(chr(b) if 32 <= b < 127 else "." for b in c)}')
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
