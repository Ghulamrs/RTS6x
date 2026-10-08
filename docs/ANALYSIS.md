# RTS6x - phase 1: analysis

Our own run-time library for the TMS320C6747, written from published interface
specifications and measured from our own tools, so that a program built by
**cpp11 (or c90) + ASM6x + LNK6x** and run on **sim6747** contains no TI code.
Today lnk6x links every program against TI's `rts6740_elf_eh.lib`, which TI's
`mklib` built from TI's run-time source and TI's own compiler. RTS6x replaces it.

Written 2026-10-06. Phase 2 (organisation) and phase 3 (implementation) follow
from this document.

## 1. The rule: specifications, never implementations

Nothing in RTS6x is copied, translated or paraphrased from another run-time
library. The sources read are **interface specifications**: what a symbol is
called, what it takes, what it must do, and the byte layout of a table. No
implementation of any of them is opened while RTS6x is written.

| read | not read, for this project |
| --- | --- |
| The specifications in section 2 | TI's `rtssrc.zip` (the source `rts6740` is built from) |
| Our own tools' source: cpp11, c90, ASM6x, LNK6x, sim6747, VM6747 | libgcc, including GCC's C6X helper routines |
| Our own test cases and their measured behaviour | newlib and libgloss (which has a tic6x port) |
| | glibc, uClibc, musl, Linux `arch/c6x` code |
| | LLVM libunwind, libc++abi, libcxxrt, libsupc++ |

Short reference sequences in a specification (the EABI's `__cxa_guard_acquire`,
the `copy_in` routine, the RLE algorithm) define behaviour; RTS6x implements that
behaviour in its own code and its own words.

## 2. The specifications

| # | Document | What it fixes for RTS6x |
| --- | --- | --- |
| S1 | **C6000 Embedded ABI**, TI SPRAB89B (Aug 2025), [ti.com/lit/an/sprab89b](https://www.ti.com/lit/an/sprab89b/sprab89b.pdf) | ch.3 calling convention; ch.8 helper functions and their register conventions; ch.9 C library ABI points (`_ftable`, errno, jmp_buf, locale); ch.10 C++ ABI deviations; ch.11 exception tables; ch.18 copy tables and `.cinit` |
| S2 | **Generic (Itanium) C++ ABI**, cited by S1 ch.10: [refspecs.linux-foundation.org/cxxabi-1.83.html](http://refspecs.linux-foundation.org/cxxabi-1.83.html), current text at [itanium-cxx-abi.github.io](https://itanium-cxx-abi.github.io/cxx-abi/abi.html) | RTTI classes and their layout (2.9.5), `__dynamic_cast`, `__cxa_atexit`, guards (3.3.2), `__cxa_pure_virtual`, `operator new`/`delete` |
| S3 | **Itanium C++ ABI: Exception Handling**, [itanium-cxx-abi.github.io/cxx-abi/abi-eh.html](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html) | `__cxa_allocate_exception`, `__cxa_throw`, `__cxa_begin_catch`/`end_catch`, `__cxa_rethrow`, `__cxa_free_exception`, the caught-exceptions stack, `std::terminate` |
| S4 | **Exception Handling ABI for the Arm Architecture**, [github.com/ARM-software/abi-aa](https://github.com/ARM-software/abi-aa/blob/main/ehabi32/ehabi32.rst) (S1 ch.11 builds on it) | the unwinder: `_Unwind_Control_Block`, `_Unwind_RaiseException`/`Resume`/`Complete`, the two phases, personality routine protocol, `__cxa_begin_cleanup`/`__cxa_end_cleanup`, `__cxa_type_match`, `__cxa_call_unexpected` |
| S5 | **ISO C (C90; C99 draft N1256)**, [open-std.org/jtc1/sc22/wg14](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf) | every C library function's required behaviour |
| S6 | **ISO C++11 draft N3337**, [open-std.org](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3337.pdf) | `<new>`, `<exception>`, `<typeinfo>` behaviour |
| S7 | **IEEE 754** behaviour, as C99 Annex F (in S5) states it, which S1 ch.8.1 requires | the floating helpers, `printf`/`strtod` conversions, `<math.h>` |
| S8 | **TMS320C674x CPU and Instruction Set**, TI SPRUFE8 | the instructions RTS6x's assembly uses, delay slots |
| S9 | Linux `man-pages` (sections 3), as the second reading of S5 | edge cases: `printf` flags, `strtod` forms, `qsort` |

S1 is TI's, but it is an ABI specification published so other toolchains can
interoperate; it says so in its introduction. S2-S4 and S9 are what the Linux and
GNU world maintain; S1 points at S2 and S3 itself.

## 3. What our compilers' code asks of a runtime - measured

Every case of both tms6747 suites was compiled (cpp11 at -O0, -O1, -O2 and -Os;
c90 once), assembled by asm6x, and the undefined symbols of the 762 objects read
from their ELF symbol tables (`tools/demand.py`). **142 symbols**, listed in
`demand-2026-10-06.txt`. Grouped, with how many of the 357 + 405 programs use each
group:

| group | symbols | note |
| --- | --- | --- |
| integer helpers | `__c6xabi_divi divu remi remu divlli remlli remull` | 32 programs use `divi`, 29 `remi`; ch.8.3 register limits apply |
| floating helpers | `__c6xabi_divd divf fixdu fixdull fixfull trunc nround` | most float work is hardware on the C674x |
| other ABI helpers | `__c6xabi_abort_msg errno_addr` | `assert` and `errno` |
| unwinding | `__c6xabi_unwind_cpp_pr3` (754 objects), `pr2` (large functions) | every function cpp11 and c90 compile carries an EXIDX entry |
| C++ exceptions | `__cxa_allocate_exception free_exception throw rethrow begin_catch end_catch end_cleanup call_unexpected` | S3 + S4 |
| C++ other | `__cxa_atexit guard_acquire guard_release pure_virtual __dynamic_cast __dso_handle`, `_Znwj _Znaj _ZdlPv _ZdaPv` | S2; `j` is the 32-bit `size_t` |
| RTTI | vtables of `__class_type_info`, `__si_`, `__vmi_`, `__pointer_`, `__enum_type_info`; `_ZTI` of `i j c a d Pi Pv Pc PKc PKv` | the runtime defines the type_info objects of fundamental types |
| startup and exit | `abort atexit` (and, through lnk6x, `_c_int00`) | section 4 |
| stdio | `printf fprintf sprintf snprintf vprintf puts putchar fputs fputc fgetc fgets gets ungetc scanf fscanf sscanf fopen fclose fflush fread fwrite fseek ftell rewind feof ferror remove close _ftable` | `printf` alone is in 417 objects |
| stdlib | `malloc calloc free abs labs atoi strtod qsort bsearch` | |
| string, ctype | `memcpy memmove memset memcmp strlen strcpy strcat strcmp strncmp`, `isalpha isdigit islower isupper isspace ispunct isxdigit tolower toupper` | |
| math | `sqrt sin cos tan asin acos atan atan2 sinh cosh tanh exp log log10 pow fmod floor ceil fabs frexp ldexp modf` | c90's cases |
| setjmp, time, locale, signal | `setjmp longjmp`, `mktime localtime strftime difftime`, `setlocale localeconv`, `signal raise` | `hd_standard_headers` |

The **interface is larger than the demand**: everything our headers declare
(`lib/*.h` for C, `include/` for C++) is callable by a user program. Phase 2 lists
it header by header; the measured set above is the first milestone, the declared
set the second.

## 4. The contracts with our own tools

These are fixed by tools we already have, and RTS6x must meet them as they stand
(or the tool changes in the same round).

**LNK6x**
- Entry point `_c_int00`, unless `-e` names another.
- Linker-defined symbols the startup reads: `__TI_STACK_END`, `__TI_STACK_SIZE`,
  `__TI_STATIC_BASE` (B14, the data page pointer), `__TI_CINIT_Base`/`Limit`,
  `__TI_Handler_Table_Base`/`Limit`, `__TI_INITARRAY_Base`/`Limit`,
  `__TI_SYSMEM_SIZE` and the `.sysmem` section (the heap), `__c_args__` and
  `.args` (`--args=N`, `-1` meaning no arguments).
- Under `--rom_model` lnk6x encodes `.data` into `.cinit` (S1 ch.18) and pulls the
  handlers **by name**: `__TI_decompress_rle24`, `__TI_decompress_none`,
  `__TI_zero_init`. RTS6x defines exactly those three. `rle24` is the RLE of S1
  18.2.1 widened to a 24-bit long run; `rle24_encode` in `LNK6x/src/link.cpp` states
  the stream exactly (an escape byte, literals, a count of 1-3 for that many escape
  bytes, 4-255 for a run, 0 for a 16- or 24-bit length, `E 00 00 00` to end), and
  RTS6x's decoder is written against that description.
- lnk6x has rules shaped by TI's library members (`memory.obj`'s own `.sysmem`,
  `args_main`, 7.4.4 handler ordering). Phase 2 lists them; RTS6x either meets
  them or lnk6x gains an RTS6x mode.
- lnk6x reads `ar` archives. RTS6x needs an archiver of its own (phase 2): the
  host's `ar` would be borrowed code in the build.

**sim6747** (and TI's simulator, which uses the same protocol)
- `C$$EXIT`: a label the program reaches when it ends; A4 is the status.
  `--main-status` reads `main`'s return or `exit`'s argument instead, because TI's
  boot calls `exit(1)`; RTS6x can simply call `exit(main(...))`.
- `C$$IO$$` and `_CIOBUF_`: the host channel. The program writes a request into
  `_CIOBUF_` - `[length:4][command:1][parameters:8][data]` - and calls `C$$IO$$`;
  the host answers in the same buffer as `[length:4][parameters:8][data]`.
  Commands 0xF0-0xF9: open, close, read, write, lseek, unlink, getenv, rename,
  time, clock. `sim6747/src/C6xHost.cpp` is the host side and the reference.

**cpp11 and c90**
- Calling convention and frame layout: S1 ch.3-4, as cpp11 emits them
  (`TMS6747.md`). RTS6x's assembly follows the same rules.
- The EXIDX/EXTAB tables cpp11 writes (`Tms6747::emitExceptionTable`): PR3's
  24-bit word, or PR2's byte codes past 0xF000 bytes; per call-site row, one catch
  descriptor per handler type, then a cleanup descriptor; `catch (...)` as type -1;
  a `-2` row for a terminate scope; a noexcept function's FESPEC row naming no
  type, which calls `__cxa_call_unexpected`. RTS6x's personality routines decode
  exactly this (S1 11.3-11.6).
- **Landing pads**: every descriptor names a trampoline cpp11 writes,
  `MVK selector, B4; B pad` - the selector being the handler's index, 0 for a
  cleanup. The runtime enters the trampoline with the exception in A4 and B15 as
  it was at the throwing call; cpp11's pad stores A4 and B4 and dispatches on B4.
  What A4 holds (the control block, S4) is what cpp11 passes back to
  `__cxa_begin_catch` and `__cxa_end_cleanup`.
- **The frame the unwinder walks** (`UnwindEntry::unwind`): every function cpp11 or c90
  emits that calls anything stores the caller's A15 in the word at the caller's B15,
  makes that word its frame pointer, and saves B3 below it with the other callee-saved
  registers in TI's pop order (`Tms6747::savedRegs`, `unwindWord`) - so a frame between
  a throw and its catch always has A15 (bit 12) and B3 (bit 5) in its pop mask, and a
  PR2 frame is exactly `MV FP,SP; POP mask; RETURN`, the same mask as byte codes. A
  function that calls nothing saves neither and gets no index entry, and it can never
  be between a throw and a catch. The unwinder refuses any other shape with
  `std::terminate`, which is the contract made loud rather than a wrong frame walked;
  `tests/m5/frames.cpp` puts every frame shape cpp11 writes between a throw and its
  handler, at -O0 and -O2 (-Os shares -O2's frame code). A compiler that emits another
  frame changes `UnwindEntry.cpp` in the same round.
- **What cpp11 cannot emit, so RTS6x does not define**: `dynamic_cast` to a reference
  is refused by name (`ParserExprNew.cpp`, `dynamicCast`), and `typeid` of a
  polymorphic glvalue reads the vptr with no null test, so neither `__cxa_bad_cast` nor
  `__cxa_bad_typeid` is ever called; a dynamic exception specification `throw(T)` is
  refused (`ParserConst.cpp`), so the only specification row is a `noexcept`/`throw()`
  one allowing nothing, and that is the only barrier `Exception::search` tests -
  `std::unexpected` is reached through `__cxa_call_unexpected` and never with a list to
  check. Each becomes work here the day cpp11 emits it.
- RTTI records cpp11 emits follow S2 2.9.5; `__dynamic_cast` and catch matching
  read them.
- Header coupling: `lib/stdio.h` declares `FILE` as a 24-byte record and
  `stdin`/`stdout`/`stderr` as `&_ftable[0..2]` (S1 9.18 requires `_ftable`). The
  layout is ours to keep or change, header and runtime together. `errno` must be
  `*__c6xabi_errno_addr()` (S1 9.5). cpp11's `lib/stdarg.h` declares
  `vfprintf(void *, const char *, va_list)` - the stream as `void *`, `FILE` not being
  in scope there - and RTS6x defines it so (`vfprintf.cpp`); the prototype is the
  header's to mend, in C++Optimize, not RTS6x's.

**The VM6747 emulator** runs assembly with a native runtime and never links. It
stays as it is: it is the reference RTS6x's results are compared with, besides
`.expected`.

## 5. What the specifications require beyond the measured symbols

- **Helper register limits (S1 8.3)**: `divi`, `divu`, `remi`, `remu` and the
  `divrem` pair may change only the registers listed in Table 8-9. cpp11 may rely
  on it, so these are hand-written in assembly and their register use is checked
  by a test, not by inspection. (D2 was exactly a caller trusting A5 across
  `remi`; the table allows `remi` to change A5.) The per-helper sets in `divi.s`,
  `remi.s` and `divremi.s` and in `tests/helpers/registers.cpp` were taken from
  Table 8-9 when they were written and agree with each other; the review of
  2026-10-08 could not re-read the table itself, so a re-reading against SPRAB89B is
  worth one pass. The 64-bit helpers (`WideDivision.cpp`) are compiled C++ with no
  register limit: cpp11 calls them as ordinary functions and saves what it needs.
- **Guards (S1 10.4)**: the first byte of a 32-bit word; non-zero is "done".
- **Constructors return `this` (S1 10.5)**; array new/delete helpers take
  constructors returning `void *`.
- **Floating behaviour (S1 8.1)**: round to nearest, no exceptions, signalling
  NaN treated as quiet.
- **Unwinding (S1 11.5)**: the virtual SP, the POP bitmask in safe-debug order,
  the 24-bit PR3/PR4 word, `MV FP, SP`, B3 restore; landing pads as section 4
  states them.

## 6. Findings made during the analysis

1. **cpp11 writes `.ref .S1` / `.ref .S2`** into its assembly: the unit field of
   `BNOP .S2 ...` is taken for a symbol. Harmless today (nothing refers to it),
   a cpp11 bug to fix.
2. **sim6747 does serve file I/O** (open, read, write, lseek, unlink, rename).
   So G1 - "no CIO file I/O" - is not the full explanation of `include-streams`
   failing on it; re-measure once RTS6x has its own `fopen`.
3. **TI's `rts6740` hands a thrown pointer to a base-class handler unadjusted**
   (C1b). RTS6x will adjust it, as S2/S3 require; `throw-pointer-base-adjust` can
   then hold clang's answer on this target too.
4. **lnk6x carries knowledge of TI's library members** (section 4). Phase 2
   decides how much of it stays.
5. **Where our tools learned TI's formats.** Some comments in cpp11 and lnk6x
   say a format was read off TI's runtime: cpp11's table emitter cites TI's
   `tdeh_pr_common.cpp`, and lnk6x's `rle24` was "read off dis6x" of rts6740.
   These are data formats our tools match so that TI's runtime could run our
   programs; no TI code is in either tool. RTS6x takes the formats from the
   specifications and from our tools' own descriptions, and once RTS6x runs, those
   comments can cite RTS6x instead.

## 7. Behaviour the review of 2026-10-08 recorded, and that stands

- **Single-threaded throughout.** `errno`, the heap, the FILE table, the caught-exception
  chain, the signal and atexit tables are plain statics; the C6747 runs one thread.
- **One character of pushback in the scanner** (`InputSource`, as ISO C 7.19.6.2/9 allows):
  `%x` on `0xg` takes the `0x`, finds no digit and fails, where macOS's library answers 0 and
  gives the `x` back - two characters of lookahead, which 7.19.6.2/9 does not require; `%f` on
  `1.5e` converts 1.5 with the `e` consumed, as the hosts do (`tests/m3/scan.c`). Recorded, not
  mended: the fix is a second pushback slot in `InputSource` and in `Stream::unget`.
- **Writes are unbuffered**: every `fputc` is one `C$$IO$$` trap, which is part of
  `printf-float`'s 2.08x of TI's cycles. A line-buffered stdout is the next speed work
  if any; it changes when output reaches the host beside stderr and at `exit`, so it is
  a round of its own, not a fix.

## 8. Size of the work, roughly in the order of phase 3

| part | contents | sizing |
| --- | --- | --- |
| 1 startup | `_c_int00`, `.cinit` handlers, `.init_array`, `main(argc, argv)`, `exit`/`atexit`/`abort`, `C$$EXIT` | small, part assembly |
| 2 host I/O | `_CIOBUF_`, `C$$IO$$`, open/read/write/lseek/close | small |
| 3 helpers | integer division and remainder (32, 64-bit, register-limited), float conversions and division, `trunc`/`nround` | medium, assembly |
| 4 strings and memory | `string.h`, `ctype.h`, `malloc`/`free`/`calloc` on `.sysmem` | small to medium |
| 5 stdio | FILE table, buffering, `printf` family **with correct floating output** (shortest-exact and fixed digits), `scanf` family, `strtod` | the largest C part |
| 6 math | `<math.h>` to C90 plus what the C++ headers use | medium; accuracy is the work |
| 7 C++ support | `operator new`/`delete`, `__cxa_atexit`, guards, `__cxa_pure_virtual`, RTTI classes and `__dynamic_cast` | medium |
| 8 exceptions | unwinder, PR2/PR3 (PR0/1/4 for completeness), `__cxa_*`, `std::terminate`, `std::exception`, `bad_alloc` | the largest C++ part |
| 9 the rest | setjmp/longjmp, time, locale, signal, `getenv` | small |

Acceptance, for each part and at the end: both tms6747 suites link against RTS6x
only - `-l rts6x.lib`, no `-i ~/c6747-lib` - and pass on sim6747 at every level,
with TI's simulator used as a referee and never linked into anything.
