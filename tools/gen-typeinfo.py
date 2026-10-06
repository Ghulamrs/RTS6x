# Writes src/cxx/vtables.s and src/cxx/fundamental.s: the vtables of the ten type_info classes of
# Itanium C++ ABI 2.9.4, and the type_info objects the runtime provides for every fundamental type,
# a pointer to it and a pointer to const (2.9.4: "the run-time support library").
import os
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'cxx')
CLASSES = ['fundamental', 'array', 'function', 'enum', 'class', 'si_class', 'vmi_class', 'pbase',
           'pointer', 'pointer_to_member']
# The fundamental types, by their mangled codes (2.9.4's list less __int128 and __float128).
CODES = ['v', 'Dn', 'b', 'w', 'Ds', 'Di', 'c', 'a', 'h', 's', 't', 'i', 'j', 'l', 'm', 'x', 'y',
         'f', 'd', 'e']

def vtname(c):
    n = '__' + c + '_type_info'
    return '_ZTVN10__cxxabiv1%d%sE' % (len(n), n)

def string(label, text):
    return ['%s:' % label, '\t.byte\t' + ', '.join(str(ord(ch)) for ch in text) + ', 0']

vt = ['; Spec: Itanium C++ ABI 2.9.4-2.9.5 - the vtables of __cxxabiv1\'s type_info classes; a',
      '; type_info object\'s first word is its class\'s vtable plus 8, the address point. Nothing calls',
      '; through them (RTS6x tells the kinds apart by that address), so the two slots are zero.',
      '; Written by tools/gen-typeinfo.py.', '']
for c in CLASSES:
    vt += ['\t.global\t%s' % vtname(c), '\t.global\t__rts6x_vt_%s' % c]
vt += ['\t.sect\t".const"', '\t.align\t4']
for c in CLASSES:
    vt += ['%s:' % vtname(c), '\t.word\t0', '\t.word\t0', '__rts6x_vt_%s:' % c, '\t.word\t0', '\t.word\t0']
open(os.path.join(ROOT, 'vtables.s'), 'w').write('\n'.join(vt) + '\n')

fu = ['; Spec: Itanium C++ ABI 2.9.4 - the type_info objects of the fundamental types, of a pointer to',
      '; each and of a pointer to const each (__pointer_type_info, flags 1 = __const_mask); 2.9.5 for',
      '; their layouts. Written by tools/gen-typeinfo.py.', '',
      '\t.ref\t__rts6x_vt_fundamental', '\t.ref\t__rts6x_vt_pointer']
for code in CODES:
    for prefix, flags in (('', None), ('P', 0), ('PK', 1)):
        fu += ['\t.global\t_ZTS%s%s' % (prefix, code), '\t.global\t_ZTI%s%s' % (prefix, code)]
fu += ['\t.sect\t".const"']
for code in CODES:
    for prefix, flags in (('', None), ('P', 0), ('PK', 1)):
        fu += string('_ZTS%s%s' % (prefix, code), prefix + code)
fu += ['\t.align\t4']
for code in CODES:
    fu += ['_ZTI%s:' % code, '\t.word\t__rts6x_vt_fundamental', '\t.word\t_ZTS%s' % code]
    for prefix, flags in (('P', 0), ('PK', 1)):
        fu += ['_ZTI%s%s:' % (prefix, code), '\t.word\t__rts6x_vt_pointer', '\t.word\t_ZTS%s%s' % (prefix, code),
               '\t.word\t%d' % flags, '\t.word\t_ZTI%s' % code]
open(os.path.join(ROOT, 'fundamental.s'), 'w').write('\n'.join(fu) + '\n')
