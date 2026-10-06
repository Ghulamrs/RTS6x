# RTS6x - phase 2: organisation

How RTS6x is laid out, built, tested and brought into the toolchain, and which
module holds which symbol. Phase 1 is `ANALYSIS.md`; this document turns its
findings into a plan phase 3 implements milestone by milestone.

Scope: the **TMS320C6747 only**. On the three host targets the C and C++ runtimes
belong to the operating system (glibc and libstdc++, libSystem and libc++,
Microsoft's CRT), as they do for every compiler there; they are out of scope.

## 1. Decisions

| # | Decision | Why |
| --- | --- | --- |
| D1 | **One library, `rts6x.lib`**: ELF, little-endian, C674x, exception handling always built in | cpp11 always emits unwind tables, so there is no use for a build without it; TI needs two (`rts6740_elf.lib`, `_eh`) only because its compiler can leave them out |
| D2 | **Assembly where the ABI fixes registers, C++ elsewhere.** `.s` files for asm6x: startup, the integer division helpers (register limits, ANALYSIS 5), `setjmp`/`longjmp`, the unwinder's register capture and restore, `C$$IO$$`; and, for speed, the fast path of `__c6xabi_divd`/`divf` (it needs `RCPDP`/`RCPSP`, which cpp11 cannot reach; `docs/DIVISION.md`), `memcpy`, `memmove`, `memset`, `strlen`, `strchr` and `strcmp` (doubleword loads and stores, `CMPEQ4`: what cpp11 cannot pipeline). Everything else in C++11 compiled by **cpp11 -O2**, written as plain C-like code, C symbols under `extern "C"` | cpp11 has no inline assembly, so assembly is separate files; cpp11 is our strongest optimiser for the C6000; C++ is needed for the exception and RTTI parts anyway; one compiler for the whole build |
| D3 | **No foreign code, and the rule is checked.** Every source file opens with a line naming the specification sections it implements. `tools/provenance` fails the build on a source file without one, and on text that marks another library (copyright lines of other vendors, file names from other runtimes) | ANALYSIS 1, made something a run can say instead of something remembered |
| D4 | **Public headers stay with the compilers** (cpp11 `lib/` and `include/`, c90 `lib/`). RTS6x keeps a private header for its own layouts and a **layout test** that compiles against both compilers' headers and checks every shared layout | Headers are per compiler and already ship with them; the layouts they fix (FILE, `_ftable`, `jmp_buf`, `struct tm`, `lconv`, `div_t`) must agree with RTS6x, and a test is what keeps two copies equal |
| D5 | **Our own archiver, `ar6x`**, a small C++14 host program in `tools/ar6x/`, writing the SysV `ar` lnk6x reads (`/` index of big-endian offsets, `//` long names) | The host's `ar` would be borrowed code in the build; the format is small and lnk6x's `archive.cpp` already states it |
| D6 | **One function per member**, or one tight group (the `printf` family over one formatter) | lnk6x pulls whole members; a member with many functions drags them all into every program |
| D7 | **lnk6x unchanged** unless a test shows otherwise. Its rules are generic: the handlers it pulls by name, `.sysmem` sized from `--heap_size` when the allocator brings a `.sysmem` input section, `.cio`, `.args` | ANALYSIS 4 checked each rule; none needs a TI member to exist |
| D9 | **Complete C++ classes, small source files, rich headers.** Each piece of the library is a class (`CioChannel`, `OutputSink`, `FormatSpec`, `Formatter`, `DecimalDigits`, ...) declared in full in its header, its members spread over small `.cpp` files; a free function only where C requires one - `printf` and the rest are `extern "C"` entry points that hand their work to the classes. No virtual functions, no objects built before `main`, no exceptions inside the library: each would pull RTTI, static construction or unwinding into every program | The user's rules for RTS6x (2026-10-06); and lnk6x pulls whole members, so small files keep programs small |
| D8 | **Correct first, then fast.** Floating division and decimal conversion are exact (integer arithmetic on the significand) before any table- or reciprocal-based speed-up; a faster version must pass the same exhaustive or randomised tests | A wrong `printf("%g")` or `1.0/3.0` is the kind of silent difference this product exists to rule out |

## 2. The repository

```
RTS6x/
  README.md
  Makefile               build/rts6x.lib with cpp11, asm6x and ar6x (macOS, Linux)
  build.cmd              the same on Windows
  rts6x-1.0.dat          the seal, as every sibling has (tools/seal)
  docs/
    ANALYSIS.md            phase 1
    ORGANISATION.md        phase 2 (this file)
    demand-2026-10-06.txt  the 142 symbols measured
    PROVENANCE.md          the rule of D3, and the specification list
  src/
    internal/rts6x.h       private declarations and layouts (FILE, the CIO buffer, the heap)
    boot/      startup, data initialisation, arguments, exit       (S1 ch.18, S5)
    host/      the C$$IO$$ channel and the low-level file calls    (vm6747sim's host side)
    helpers/   __c6xabi_* division, conversion, rounding           (S1 ch.8)
    string/    <string.h>, <ctype.h>                               (S5)
    stdlib/    <stdlib.h>: memory, conversion, sort, search, rand  (S5)
    stdio/     FILE table, buffering, printf and scanf families    (S5, S9)
    math/      <math.h>                                            (S5, S7)
    misc/      setjmp, time, locale, signal, errno, assert         (S1 ch.9, S5)
    cxx/       new/delete, guards, atexit, RTTI, dynamic_cast      (S1 ch.10, S2)
    eh/        unwinder, personality routines, __cxa_* exceptions  (S1 ch.11, S3, S4)
  tools/
    ar6x/        the archiver (C++14, built like ASM6x)
    demand.py    the measurement of ANALYSIS 3
    provenance   the check of D3
    seal, seal.json
  tests/
    run.sh       every test below, against build/rts6x.lib only
    cases/       small programs with .expected, one or more per module
    helpers/     register-limit and exhaustive arithmetic tests
    layout/      the header layout test of D4
```

## 3. The modules, symbol by symbol

The **measured** symbols (ANALYSIS 3) are what milestones M1-M5 must deliver; the
**declared** ones (in our headers, not yet called by any test case) follow in M6.
`*` marks a declared-only symbol.

**boot** - `_c_int00` (asm), the cinit walk and its three handlers
`__TI_decompress_none`, `__TI_decompress_rle24`, `__TI_zero_init`, the
`.init_array` constructors, `__c_args__` to `argc`/`argv`, `exit`, `atexit`,
`abort`, `C$$EXIT`, `__c6xabi_abort_msg`, `getenv*`.

**host** - `_CIOBUF_` (in `.cio`), `C$$IO$$`, and the calls over it: `open*`,
`close`, `read*`, `write*`, `lseek*`, `unlink*`, `rename`, `time`, `clock`.

**helpers** - `__c6xabi_divi divu remi remu` and `divremi* divremu*` (asm,
Table 8-9 registers), `divlli divull* remlli remull`, `divd divf`,
`fixdu fixdull fixfull` and the rest of Table 8-1/8-2 that cpp11 or c90 can call,
`trunc nround`, `__c6xabi_errno_addr`.

**string** - `memcpy memmove memset memcmp memchr* strlen strcpy strncpy* strcat
strcmp strncmp strchr* strrchr* strstr* strspn* strcspn* strpbrk* strtok*
strerror*`; ctype `isalpha isdigit islower isupper isspace ispunct isxdigit
isalnum* iscntrl* isgraph* isprint* tolower toupper`.

**stdlib** - `malloc calloc free realloc*` over `.sysmem`; `abs labs atoi atol*
atof* strtol* strtoul* strtod qsort bsearch rand* srand*`.

**stdio** - `_ftable` and buffering; `fopen freopen* fclose fflush fread fwrite
fseek ftell rewind feof ferror clearerr* remove tmpfile* perror*`; `fgetc getc*
getchar* fgets gets ungetc fputc putc* putchar fputs puts`; `printf fprintf
sprintf snprintf vprintf vfprintf* vsprintf*` over one formatter; `scanf fscanf
sscanf` over one scanner.

**math** - `sqrt sin cos tan asin acos atan atan2 sinh cosh tanh exp log log10
pow fmod floor ceil fabs frexp ldexp modf round* trunc*`.

**misc** - `setjmp longjmp` (asm, `jmp_buf` as the headers size it), `mktime
localtime gmtime* asctime* ctime* strftime difftime`, `setlocale localeconv`,
`signal raise`.

**cxx** - `_Znwj _Znaj _ZdlPv _ZdaPv` and the nothrow and placement forms `<new>`
declares, `set_new_handler`, `get_new_handler`; `__cxa_atexit __dso_handle
__cxa_guard_acquire __cxa_guard_release __cxa_pure_virtual`; the type-info classes'
vtables (`__class_type_info __si_class_type_info __vmi_class_type_info
__pointer_type_info __enum_type_info` and the rest of S2 2.9.5), the `_ZTI`
objects of every fundamental type and pointer to one, `__dynamic_cast`.

**eh** - `__c6xabi_unwind_cpp_pr0 pr1 pr2 pr3 pr4`, `_Unwind_RaiseException
_Unwind_Resume _Unwind_Complete _Unwind_DeleteException` and the virtual register
set (asm for the capture and install), `__cxa_allocate_exception
__cxa_free_exception __cxa_throw __cxa_rethrow __cxa_begin_catch __cxa_end_catch
__cxa_begin_cleanup __cxa_end_cleanup __cxa_type_match __cxa_call_unexpected`,
`std::terminate`, `std::unexpected` and their setters.

## 4. Building

`make` (or `build.cmd`) compiles every `src/**/*.cpp` with
`cpp11 -arch tms6747 -O2 -c`, assembles every `src/**/*.s` with `asm6x`, and
archives the objects with `ar6x` into `build/rts6x.lib`. Objects go to
`../build/RTS6x/obj`, as the siblings keep theirs outside the checkout. The tools
are found beside the checkout (`../C++Optimize/cpp11.exe`, `../ASM6x/build`,
`tools/ar6x`) or named by `CPP11=`, `ASM6X=`, `AR6X=`.

Two rules the build itself checks: `tools/provenance` passes (D3), and the archive
defines every symbol of `demand-2026-10-06.txt` that the current milestone covers.

## 5. Testing

| test | what it proves | runs on |
| --- | --- | --- |
| `tests/cases` | each module's behaviour: a program per module with its `.expected` from clang on the Mac, as cpp11's own cases are | vm6747sim, linked with rts6x.lib only |
| `tests/helpers` registers | `divi`, `divu`, `remi`, `remu`, `divremi`, `divremu` change no register outside Table 8-9: run under `vm6747sim --trace`, compare the register file before and after each call | vm6747sim |
| `tests/helpers` arithmetic | division, remainder and conversions against the host's own arithmetic, at the edges (0, ±1, INT_MIN, powers of two, NaN, infinities, subnormals) and on random operands | vm6747sim |
| `tests/layout` | the layouts of D4 agree between RTS6x and both compilers' headers | compile time |
| the two **tms6747 suites** | the acceptance: cpp11's (357 cases, four levels) and c90's (405) link **rts6x.lib only** and pass both legs | vm6747 + vm6747sim |
| the **referee** | the same images run on TI's CCS 5.5 C6747 simulator print the same | Windows box, `tools/c6747/release-check` |

The two suites learn one variable, `RTSLIB` (default: TI's library until M5 passes,
then `rts6x.lib`), so the switch is one line in each.

## 6. Milestones (phase 3)

| | delivers | done when |
| --- | --- | --- |
| **M0** | repository, `ar6x`, Makefile and build.cmd, provenance and layout tests, test runner | an empty `rts6x.lib` builds on three hosts and lnk6x accepts it |
| **M1** | boot, host, `puts`, a minimal `printf` (`%d %s %c %x`), `exit`/`atexit`/`abort` | "hello" and an exit-status program run on vm6747sim with no TI library |
| **M2** | helpers (with the register test), string, ctype, stdlib, the integer `printf` | the c90 suite's integer cases pass on rts6x.lib |
| **M3** | floating `printf`/`scanf`/`strtod` (exact), math, the rest of stdio, misc | the c90 suite passes whole on rts6x.lib |
| **M4** | cxx: new/delete, guards, `__cxa_atexit`, RTTI, `__dynamic_cast` | every cpp11 case that throws nothing passes |
| **M5** | eh: unwinder, PR3 and PR2 (then PR0/1/4), `__cxa_*`, terminate | both suites pass whole at -O0/-O1/-O2/-Os on rts6x.lib; `RTSLIB` default switched |
| **M6** | the declared-only symbols; the referee run on TI's simulator; speed work (D8) | every header declaration defined; referee agrees |
| **M7** | integration: RIDE links rts6x.lib instead of TI's library and ships it; release scripts clone RTS6x; `washout.py`, MASTER.SEAL | an installed RIDE builds and runs a C6747 program with nothing of TI's on the link line |
| **M8** | a Release and a Debug build of every runtime the C6000 links, as the host targets already have: `rts6x.lib`/`printf6x.lib` at -O2 and `rts6xd.lib`/`printf6xd.lib` at -O0 with `_DEBUG`; Shalimar's `shmrt-tms6747` at -O2 and `shmrt-tms6747-debug` at -O0 with `SHM_DEBUG`; RIDE links the pair the configuration names, and the installers ship both | on the Windows PC and the Linux box, from fresh clones: both pairs build, RTS6x's tests pass against each, and an installed RIDE links a C++ and a Shalimar program in Release with the -O2 pair and in Debug with the -O0 pair, each running on vm6747sim |
| **M9** | Shalimar on the C6000 as on the hosts: its runtime packed as `shmrt6x.lib` and `shmrt6xd.lib` beside RTS6x and linked with `-l` (the `.s` kept for the emulator); a Debug build's own session armed through vm6747sim - `SHM_DEBUG` over CIO's getenv, commands on stdin and answers on stderr, a stdin read answering what has arrived | a Shalimar program on the C6000 links the packed runtime in Release and Debug, and in Debug stops at a breakpoint, steps and continues in RIDE as it does on Windows and Linux |
| **M10** | (beyond RTS6x) C and C++ debugging on x86_64-windows: c90 and cpp11 write debug information a Windows debugger reads (CodeView in the object, a PDB at the link), or RIDE drives a debugger that reads the DWARF they write | a c90 and a cpp11 program built for x86_64-windows in Debug stop at a breakpoint, step and show locals in RIDE, as on Linux and the Mac |
| **M11** | (beyond RTS6x) Shalimar's optimiser on arm64-darwin, and an -O2 that does more than -O1 on every host | shalimar -O1 and -O2 change the code on all three hosts, the Shalimar suite passing at each level |

## 7. What changes in the other repositories, and when

| repository | change | milestone |
| --- | --- | --- |
| C++Optimize, VM6747/Compiler-Ci | `RTSLIB` in `tests/tms6747.sh`; later its default | M1, M5 |
| C++Optimize | the `.ref .S1`/`.S2` fix (ANALYSIS 6.1) | any time |
| RIDE-4.7 | link with `rts6x.lib` from the package instead of the CCS compiler directory; ship it | M7 |
| RIDE-4.7 packaging | release scripts clone and build RTS6x; `washout.py`; MASTER.SEAL | M7 |
| RIDE-4.7 | the configuration chooses `rts6x.lib` or `rts6xd.lib`, `shmrt-tms6747` or `shmrt-tms6747-debug`; the workspace, RIDE.sln and the installers carry both | M8 |
| VM6747/Compiler-Si | the C6000 Shalimar runtime built twice, -O2 and -O0 with `SHM_DEBUG` | M8 |
| RIDE-4.7, VM6747-sim | the packed Shalimar runtime; the session through vm6747sim; a stdin read that answers what has arrived | M9 |
| C++Optimize, VM6747/Compiler-Ci, RIDE-4.7 | debug information for x86_64-windows, or a debugger that reads theirs | M10 |
| VM6747/Compiler-Si | the optimiser on arm64-darwin; -O2 | M11 |
| LNK6x | none expected (D7); a fix only if a test needs one | - |
