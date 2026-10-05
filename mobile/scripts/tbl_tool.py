#!/usr/bin/env python3
"""tbl_tool.py — Knight Online .tbl tablo çözümleme/analiz aracı.

Kullanım:
  tbl_tool.py info  <dosya.tbl|dizin>     # XOR katmanını çöz, başlığı ve 2. katman belirtilerini yaz
  tbl_tool.py dump  <dosya.tbl> [N]       # çözülen tabloyu CSV benzeri yazdır (ilk N satır)
  tbl_tool.py xor   <giriş> <çıkış>       # yalnız XOR katmanını çöz (ham baytlar)

Katman 1 (tüm KO istemcileri): akış XOR'u, key_r=0x0816, c1=0x6081, c2=0x1608 (N3TableBaseImpl.cpp).
Katman 2 (2xxx istemcileri, 2369/ISTIRAP dahil): ham dosya = [16 bayt sabit başlık][uint32 BE uzunluk]
[8 baytlık DES blokları] (IP/FP'siz DES, sabit tur anahtarları; ko_tbl_des.py). Çözülen veri = [5 bayt önek]
[standart N3 tablosu]; ardından akış XOR'u (0x0418/0x8041/0x1804). Bu dosyalarda klasik XOR katmanı YOKTUR
(XOR sonrası görülen 4429ae6e... öneki, sabit başlığın XOR'lanmış hâlidir).
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ko_tbl_des  # noqa: E402

DT_NAMES = {1: 'char', 2: 'byte', 3: 'short', 4: 'word', 5: 'int', 6: 'dword', 7: 'string', 8: 'float', 9: 'double'}
DT_SIZE = {1: 1, 2: 1, 3: 2, 4: 2, 5: 4, 6: 4, 8: 4, 9: 8}


def xor_layer(data: bytes) -> bytes:
    key_r, c1, c2 = 0x0816, 0x6081, 0x1608
    out = bytearray(len(data))
    for i, b in enumerate(data):
        out[i] = b ^ (key_r >> 8)
        key_r = ((b + key_r) * c1 + c2) & 0xFFFF
    return bytes(out)


def header_ok(d: bytes):
    """N3TableBase başlığı: int32 sütun sayısı, sütun başına int32 tür, int32 satır sayısı (ilk sütun dword)."""
    if len(d) < 8:
        return False, 'kısa'
    n = struct.unpack_from('<I', d, 0)[0]
    if n == 0 or n > 256 or len(d) < 4 + 4 * n + 4:
        return False, f'sütun sayısı {n}'
    types = list(struct.unpack_from('<%dI' % n, d, 4))
    if any(t not in DT_NAMES for t in types) or types[0] != 6:
        return False, f'geçersiz sütun türü {types[:8]}'
    rows = struct.unpack_from('<I', d, 4 + 4 * n)[0]
    if rows > 5_000_000:
        return False, f'satır sayısı {rows}'
    return True, f'{n} sütun {rows} satır türler {[DT_NAMES[t] for t in types]}'


LAYER2_PREFIX_LEN = 5  # çözülen veri: [uint32 ?][uint8 sütun sayısı] + standart tablo


def decode(data: bytes):
    if ko_tbl_des.is_layer2(data):
        d2 = ko_tbl_des.decrypt(data)
        ok2, why2 = header_ok(d2[LAYER2_PREFIX_LEN:])
        if ok2:
            return d2[LAYER2_PREFIX_LEN:], 'des+xor', why2 + f' önek={d2[:LAYER2_PREFIX_LEN].hex()}'
        return None, 'des?', f'DES çözüldü ama başlık geçersiz: {why2}; ilk16={d2[:16].hex()}'
    d = xor_layer(data)
    ok, why = header_ok(d)
    if ok:
        return d, 'xor', why
    return None, 'unknown', f'{why}; ilk16={d[:16].hex()} boyut%8={len(d) % 8}'


def rows_of(d: bytes):
    n = struct.unpack_from('<I', d, 0)[0]
    types = list(struct.unpack_from('<%dI' % n, d, 4))
    pos = 4 + 4 * n
    rows = struct.unpack_from('<I', d, pos)[0]
    pos += 4
    for _ in range(rows):
        row = []
        for t in types:
            if t == 7:
                ln = struct.unpack_from('<I', d, pos)[0]
                pos += 4
                row.append(d[pos:pos + ln].decode('cp1254', 'replace'))
                pos += ln
            else:
                fmt = {1: 'b', 2: 'B', 3: 'h', 4: 'H', 5: 'i', 6: 'I', 8: 'f', 9: 'd'}[t]
                row.append(struct.unpack_from('<' + fmt, d, pos)[0])
                pos += DT_SIZE[t]
        yield row


def cmd_info(path):
    files = [path]
    if os.path.isdir(path):
        files = sorted(os.path.join(path, f) for f in os.listdir(path) if f.lower().endswith('.tbl'))
    counts = {}
    for f in files:
        data = open(f, 'rb').read()
        _, kind, why = decode(data)
        counts[kind] = counts.get(kind, 0) + 1
        print(f'{os.path.basename(f):40s} {len(data):9d}  {kind:14s} {why}')
    print('özet:', counts)


def cmd_dump(path, n):
    d, kind, why = decode(open(path, 'rb').read())
    if d is None:
        print('çözülemedi:', kind, why)
        return 1
    print('#', kind, why)
    for i, row in enumerate(rows_of(d)):
        if i >= n:
            break
        print(';'.join(str(c) for c in row))
    return 0


def main(argv):
    if len(argv) < 3:
        print(__doc__)
        return 2
    cmd = argv[1]
    if cmd == 'info':
        cmd_info(argv[2])
    elif cmd == 'dump':
        return cmd_dump(argv[2], int(argv[3]) if len(argv) > 3 else 20)
    elif cmd == 'xor':
        open(argv[3], 'wb').write(xor_layer(open(argv[2], 'rb').read()))
    else:
        print(__doc__)
        return 2
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
