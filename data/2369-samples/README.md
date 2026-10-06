# 2369 (ISTIRAP) harita örnekleri

ISTIRAP 2369 istemcisinin `Zones/` klasöründen değiştirilmeden alındı. Biçim çözümlemesi için.

- `moradon.gtd`: zemin (625206 B)
- `moradon.opd`: nesne/çarpışma (5919521 B)
- `moradon.opdext`: ek nesne verisi (424509 B)
- `upc_el_ba.n3anim`: 2369 oyuncu animasyonu (14516 B, kayıt başına DES'li parça)
- `upc_el_ba.1298.n3anim`: aynı dosyanın 1.298 sürümü (8822 B, düz), karşılaştırma için
- `upc_el_ba.n3joint`, `upc_el_ba.n3chr`, `npc_el_ba.n3Anim`: 2369 Chr örnekleri
- `2017_esl_a001_0.gtt`: moradon.gtd'nin ilk GTT döşeme dokusu (DTex)
- `moradon.tct`, `moradon.tlt`: Moradon zemin renk/ışık dosyaları
- `mob_kecoon.n3joint/.n3anim`, `mob_snowman.n3joint/.n3anim`: mob modeli örnekleri (NPC_Looks satır 100 = kecoon)
- `DTex/`: 2369 istemcisinin bütün zemin dokuları (1102 dosya, 1096 GTT; tüm haritalar). Moradon'un kullandığı 42 GTT'nin adları moradon.gtd ofset 607016'dan 260 bayt adımla.
- `fx/ice_cast0_3.fxb` + `fx/object/31110ice_cm/`: fx paketinden. Not: "ice_cm.n32hape" yazım hatası orijinal fxb'nin içinde (ofset 380), okuma hatası değil.
