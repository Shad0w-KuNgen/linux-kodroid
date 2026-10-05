#!/usr/bin/env python3
"""ui_tool.py — 2369 istemcisinin UI paketi (UI/ui.hdr + ui.src) ve .istirap UIF'leri için araç.

  ui_tool.py list    <UI dizini> [filtre]        # paket dizini (ad, ofset, boyut)
  ui_tool.py extract <UI dizini> <ad> <çıkış>    # paketten tek dosya çıkar
  ui_tool.py istirap <dosya.istirap> <çıkış.uif> # Pearl Guard dcpUIF çözümü
  ui_tool.py info    <dosya>                      # ilk 64 baytın hex dökümü (UIF başlığı kontrolü)

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
