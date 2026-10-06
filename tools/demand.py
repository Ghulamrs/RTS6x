# undefined global symbols of ELF32 little-endian objects: prints "symbol<TAB>object" lines
import struct, sys
for p in sys.argv[1:]:
    d = open(p, 'rb').read()
    if d[:4] != b'\x7fELF': continue
    shoff, = struct.unpack_from('<I', d, 0x20); shentsize, shnum = struct.unpack_from('<HH', d, 0x2e)
    secs = [struct.unpack_from('<IIIIIIIIII', d, shoff + i * shentsize) for i in range(shnum)]
    for s in secs:
        if s[1] != 2: continue                      # SHT_SYMTAB
        strtab = secs[s[6]]; off, size, ent = s[4], s[5], s[9] or 16
        for k in range(1, size // ent):
            name, value, sz, info, other, shndx = struct.unpack_from('<IIIBBH', d, off + k * ent)
            if shndx == 0 and (info >> 4) in (1, 2):   # undefined, GLOBAL or WEAK
                n = d[strtab[4] + name:].split(b'\0', 1)[0].decode()
                print(f"{n}\t{p.rsplit('/',1)[-1]}")
