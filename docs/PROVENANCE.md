# Provenance: where every line of RTS6x comes from

RTS6x is written from **interface specifications**, never from another run-time
library's source. This file states the rule, how it is checked, and what may be read.

## The rule

1. Every source file in `src/` and `tools/ar6x/` opens with a `Spec:` line in its
   first five lines, naming the specification sections it implements, or `none`
   with the reason when it implements no specification (a test, a version string).
2. No text is copied, translated or paraphrased from another run-time library.
3. A specification's short reference sequence (an algorithm in prose or a few
   lines of C that define behaviour) is implemented in RTS6x's own code and words.

## What may be read

The specifications listed in `ANALYSIS.md` section 2, our own tools' source
(cpp11, c90, ASM6x, LNK6x, vm6747sim, VM6747), and our own tests. Facts our tools
already record about a format (lnk6x's `rle24`, cpp11's exception tables) are used
as those tools describe them.

## What is never opened while RTS6x is written

TI's run-time source (`rtssrc.zip`) and its objects' disassembly; libgcc; newlib
and libgloss; glibc, uClibc, musl; Linux `arch/c6x` code; LLVM libunwind,
libc++abi; libcxxrt; libsupc++.

## How it is checked

`tools/provenance`, run by every build and by `tests/run.sh`, fails when a source
lacks its `Spec:` line or carries a mark of another library: a licence or copyright
line, or the name of a file from another run-time. It cannot prove a negative; it
makes the rule something a run says rather than something remembered.
