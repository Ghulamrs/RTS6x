# RTS6x review: read-only, 2026-10-08

Reviewer: a Fable subagent, run on the project at /Users/g.r.akhtar/Developer/Claude/RTS6x. Nothing in the project was modified.

## Executive summary

**Verdict: a sound, carefully built library.** The code matches its documents closely. The clean-room rule shows in every file: each has a `Spec:` line naming ISO C, SPRAB89B, Itanium or EHABI sections, and there's no borrowed structure or vocabulary anywhere. The numerically hard parts are mathematically right as far as could be checked by hand:

- exact `printf` float digits
- correctly rounded `strtod`, with Clinger and powers-of-five fast paths and a BigNumber fallback
- exact `divd`/`divf`, using a Goldschmidt estimate plus a modular correction
- Knuth D for 64-bit division

The test suite is stronger than most hobby runtimes. It covers both link models and both optimisation levels, checks hashes against the host, and has a register-clobber probe for the ABI helpers. **No Critical defects were found.** What did turn up: a handful of conformance corner cases, a few design constraints worth writing down, and some drift between the docs and process and the code.

**Top 5 issues**

1. **(Medium) Output calls to a non-writable stream report success.** `fprintf`, `printf`, `fputs`, `puts`, `fputc` and `wprintf` on a stream without `Writable` get `-1` from `Stream::writeDescriptor()` and build `OutputSink(-1)`. The sink treats `fd_ < 0` as a *memory* sink, so `finish()` returns the character count. The caller then returns a positive value or 0 instead of a negative value or `EOF`. `ferror()` is set, but the return value is wrong.
2. **(Medium) A build rule stated in ORGANISATION §4 is not implemented.** The rule says "the archive defines every symbol of `demand-2026-10-06.txt` that the current milestone covers". Nothing in `Makefile`, `build.cmd`, `tests/run.sh` or `tools/` checks the demand list; only `tools/provenance` runs.
3. **(Medium, design constraint) The unwinder only handles the canonical cpp11/c90 frame.** `UnwindEntry::unwind` refuses any frame whose pop mask lacks A15 *and* B3. For PR2, `frameMask()` accepts exactly the 4-opcode sequence `MV FP,SP; POP mask; RETURN`, and any other shape goes to `std::terminate`. This is fine while cpp11 always emits that frame, but it's an unwritten contract between RTS6x and the compiler.
4. **(Low–Medium) ORGANISATION §3 promises symbols that don't exist.** Six symbols listed under *eh* aren't defined anywhere in `src/`: `_Unwind_RaiseException`, `_Unwind_Resume`, `_Unwind_Complete`, `_Unwind_DeleteException`, `__cxa_begin_cleanup` and `__cxa_type_match`. `__cxa_bad_cast` and `__cxa_bad_typeid` are also missing.
5. **(Low) Several docs are stale.** `tests/cases/` is empty. The `misc/` layout in the repo map no longer matches the tree. `rts6x-1.0.dat` is now `rts6x-1.1.dat`. The registers test is described with the wrong method, and D9 is listed before D8.

## Findings

| ID | Sev | Area | Location | Issue | Why it matters / suggested fix |
|---|---|---|---|---|---|
| F1 | Medium | stdio | `src/stdio/OutputSink.cpp:62-73`, `Stream.h:26-29`, `vfprintf.cpp:211-213`, `fputs.cpp:47-50`, `puts.cpp`, `fputc.cpp`, `wprintf.cpp` | `writeDescriptor()` returns -1 for a read-only stream. `OutputSink` with fd<0 behaves as a counting memory sink, so `finish()` returns the count. | `fprintf(stdin,"x")` returns 1 and `fputs` returns 0, instead of a negative value or EOF (ISO C 7.19.6.1/14, 7.19.7.4). Have `OutputSink` remember a refused descriptor and return -1 from `finish()`, or have the entry points test `fd < 0` first. |
| F2 | Medium | build/process | `docs/ORGANISATION.md` §4; `Makefile`, `build.cmd`, `tests/run.sh` | The "archive defines every demand symbol" check isn't implemented anywhere. | A regression that drops a member would only be caught by a suite run in another repo. Add `tools/demand-check`, which compares `ar6x -t` output against the list, and call it from `run.sh`. |
| F3 | Medium | eh/ABI | `src/eh/UnwindEntry.cpp:203-243` | Only frames with A15 (bit 12) and B3 (bit 5) in the pop mask can be unwound. PR2 works only when the first 4 opcodes are exactly `D0 8x xx E7`. | Any function compiled without a frame pointer, or with a stack-adjust opcode, ends in `terminate`. Document the contract in ANALYSIS §4 and add a test with a leaf or `-Os` function between the throw and the catch. |
| F4 | Low | docs/eh | `docs/ORGANISATION.md` §3 "eh" | `_Unwind_RaiseException`, `_Unwind_Resume`, `_Unwind_Complete`, `_Unwind_DeleteException`, `__cxa_begin_cleanup` and `__cxa_type_match` are promised, but none are defined. | Define them or strike them from the plan. M6's "every header declaration defined" can't be claimed for these. |
| F5 | Low | cxx | `src/cxx/` | No `__cxa_bad_cast`, `__cxa_bad_typeid`, `__cxa_vec_*` or `operator delete(void*, size_t)`. | Harmless if cpp11 never emits them, and the demand list says it doesn't today. Verify how cpp11 compiles a failing `dynamic_cast<D&>`, because it must throw `std::bad_cast` somehow. |
| F6 | Low | exit/signal | `src/exit/Termination.cpp:77-80`, `src/misc/SignalTable.cpp:120-128` | `abort()` halts without raising `SIGABRT`. | ISO C 7.20.4.1 says `abort` raises SIGABRT, so a `signal(SIGABRT, h)` handler never runs. Route `abort` through `SignalTable::deliver(SIGABRT)`, with a recursion guard. |
| F7 | Low | exit | `src/exit/Termination.cpp:66-75` | `exit()` neither flushes nor closes open streams. | Safe today because writes are unbuffered, but 7.20.4.3 requires closing them. Either close `_ftable[3..]` in `exit` or document the behaviour. |
| F8 | Low | eh | `src/eh/ExceptionThrow.cpp:167-168` | A typed dynamic exception spec such as `throw(A)` is never checked. Only an empty spec acts as a barrier. | `void f() throw(A) { throw B(); }` propagates B instead of calling `unexpected()`. This is fine if cpp11 only emits noexcept rows. |
| F9 | Low | time | `src/time/Calendar.cpp:94-95` | `total <= 24855` allows `total*86400 + h*3600 + …` to reach 2147558399, which is above INT_MAX. The same happens at the negative end. | Signed overflow (undefined behaviour) in the last or first ~21 hours of the 32-bit `time_t` range. Compute in `long long`. |
| F10 | Low | stdio | `src/stdio/StreamOpen.cpp:133` | `parseMode("")` reads `mode[1]` past the terminator. | One-line guard: `if (!mode[0]) return false`. |
| F11 | Low | stdio | `src/stdio/StreamPosition.cpp:104-106` | `seek()` drops the read-ahead before asking the host. If the host refuses, the stream's position no longer matches the host's. | Ask the host first, and drop the buffer only on success. |
| F12 | Low | scanf | `src/stdio/ScanNumber.cpp:32-42, 69-83`; `ScanFloatText.cpp` | `%x`/`%i` on "0xg" consumes the `x` and fails. `%f` on "1.5e" converts 1.5 after consuming the `e`. | These are one-pushback limits. Document them, or buffer the lookahead. |
| F13 | Low | tests | `tests/m3/clock.c:34` | The "too small" strftime test uses an 8-byte buffer for "Friday", which fits. | The return-0 path is never exercised. Use a 4-byte buffer. |
| F14 | Low | docs | `docs/ORGANISATION.md` §2, §5, §6 | Stale: `tests/cases/` is empty, `misc/` doesn't match the actual `exit/`, `time/` and `locale/`, `rts6x-1.0.dat` is now 1.1, the registers-test method is described wrongly, and D9 is listed before D8. | Update the map. |
| F15 | Low | tests | `tests/printf/formats.expected:8` | The `%a` expectations encode macOS libc choices. | Both choices are legal, but a Linux run won't reproduce the file. Say so in the file header. |
| F16 | Info | stdio/perf | `Stream.h:3`, `OutputSink.h:30` | Writes are unbuffered, so every `fputc` is a CIO trap. | Part of why `printf-float` runs at 2.08× TI's time. A line-buffered stdout would help. |
| F17 | Info | helpers/ABI | `divi.s`, `remi.s`, `divremi.s`, `tests/helpers/registers.cpp:72-79` | The per-helper register sets are consistent between the code and the probe test. | This couldn't be checked against SPRAB89B Table 8-9 itself. Worth re-reading the table once. |
| F18 | Info | helpers | `src/helpers/WideDivision.cpp` | The 64-bit division helpers are compiled C++, so they clobber whatever cpp11 clobbers. | Safe because cpp11 treats them as ordinary calls. State this in ANALYSIS §5. |
| F19 | Info | stdlib | `src/stdlib/Heap.cpp:55-64` | The size calculation could underflow if the heap symbol were smaller than the alignment slack. | It can't happen today, and a guard costs nothing. |
| F20 | Info | stdlib | `src/stdlib/FloatTextDecimal.cpp:127-129` | `ERANGE` is set for every subnormal result. That's allowed, but the comment justifying it is wrong. | Fix the comment. |
| F21 | Info | tools | `tools/ar6x/ar6x.cpp:63` | `strnlen` is POSIX, not ISO C++14. | It builds on all three hosts, but a 4-line helper would remove the dependency. |
| F22 | Info | git | `.gitattributes` | There's no `*.cmd text eol=crlf` rule. The handover relies on a `core.autocrlf=false` note instead. | Add the rule. |
| F23 | Info | git | local branch `fix/drop-padding` | The branch was merged, but the local copy is still there. | Cosmetic. |
| F24 | Info | stdio | `src/stdio/vfprintf.cpp:209` | The function is declared `vfprintf(void *stream, ...)` because cpp11's `<stdarg.h>` declares it that way. | Fix the header so the real prototype is used. |
| F25 | Info | tools/speed | `tools/speed` | One run per benchmark, measured on the whole image (startup and I/O included), with no repetition or variance. | Adequate for a ratio table. Note that it includes CIO cost. |
| F26 | Info | docs | `docs/MATH.md` | The "within 0.503 ulp" claim is backed by the table (tanh 0.502981). | No action needed. |

## Detailed notes

### 1. Architecture vs ANALYSIS / ORGANISATION
- The layout matches §2 except as noted in F14. There are 170 `extern "C"` entry points, and all 142 measured demand symbols are defined.
- The M0–M9 claims in the handovers match the tree. M10 and M11 are correctly marked as not started.
- D1–D9 are honoured: assembly appears only where it's listed, there are no virtuals or static constructors, and the only exceptions thrown inside the library come from `__rts6x_bad_alloc`.

### 2. Runtime correctness
- **Boot:** B15 is aligned, B14 is set, the cinit walk and `.init_array` order are right, and the program ends with `exit(main())`. AMR/CSR/IER aren't initialised. That doesn't matter on the simulators, but it's worth a comment for real hardware.
- **stdio:** Format-spec parsing, the `%#o`/`%#x`/`%.0d`/`%p`/`%s`/`%n` edge cases, the exact `DecimalDigits`, ties-to-even rounding, `%g` carry handling, `%a` for subnormals and `snprintf(0,0)` are all correct. The scanner is correct apart from F12. On streams, see F1, F7, F10 and F11.
- **stdlib:** The heap (first fit with coalescing), the `strtol` overflow logic, the per-base digit tables, the `strtod` fast and slow paths, `qsort` (median-of-three with a depth limit and heapsort fallback) and `rand` were all verified.
- **math:** The methods match MATH.md. They weren't re-measured, since there's no toolchain in the sandbox.
- **setjmp/longjmp, locale/ctype, time:** All correct apart from F9.
- **C++ support:** `operator new` and its handler loop, guards (SPRAB89B 10.4), atexit/`__cxa_atexit` LIFO, the RTTI vtable layout and `__dynamic_cast` (5.2.7/8) are correct.
- **Exceptions:** Two-phase EHABI, register capture, install latency, index sort with a malloc-failure fallback, the 48-byte header and pointer catch adjustment are correct. See F3, F4 and F8.
- **ABI:** EABI throughout, with consistent argument and return registers. `divd`/`divf` use only caller-saved registers.
- **WideDivision:** The quotient estimate was proven to be in {Q, Q+1}, so one correction is enough.
- **Reentrancy:** The library is single-threaded throughout. That's fine for the target, but say it once in ANALYSIS.

### 3. Clean-room hygiene
`tools/provenance` passes. Greps for distinctive TI, GNU and LLVM names found nothing beyond the protocol-required `__TI_*`, `C$$IO$$` and `__c6xabi_*`. **Verdict: clean.**

### 4. C++14 / portability
`src/` is written at C++11 level. `tools/ar6x` builds clean with `g++ -std=c++14 -Wall -Wextra -Werror -pedantic`. A host syntax check compiled 225 of 238 sources. The other 13 fail only because of host header or 64-bit pointer issues, which is expected for a 32-bit target. The 14 warnings are all benign.

### 5. Build-system consistency
`Makefile`, `build.cmd` and `rts6x.vcxproj` build the same file sets with the same flags, and `printf6x.members` is consistent. `tools/seal check` found 378 files with 0 differences, matching 3346A221. Gaps: F2, and `build.cmd` has no `clean` target.

### 6. Tests and tools
- Coverage is strong.
- These functions aren't tested by name: `freopen`, `getc`, `getchar`, `gets`, `putc`, `ctime`, `localtime`, `vprintf`, `vfprintf`, `_fileno` and `__cxa_guard_abort`.
- The 3 referee differences are plausibly caused by host paths. Those cases live in the c90 suite, so they couldn't be re-checked here.

### 7. Code quality
Files are small, the headers are good and the comments are honest. Two comment/code mismatches turned up: `UnwindTable.h:52` says "(begin, end]" while the code uses `[begin, end)`, and F20.

### 8. Git
69 commits and a clean working tree. No binaries were ever committed, and the largest blob is under 200 kB.

## Not verified
- No C6000 build or run was done, because cpp11, asm6x, lnk6x, sim6747 and TI's tools aren't available. The 171/0 and 815/818 results, the accuracy tables and the cycle counts come from the handover.
- SPRAB89B Table 8-9, the PR3 layout and LNK6x's rle24 encoder couldn't be checked against their sources.
- The cpp11 behaviour the runtime depends on is outside the permitted folder.

## Read-only confirmation
The only commands run in the RTS6x folder were `ls`, `find`, `cat`, `head`, `wc`, `grep`, `git --no-optional-locks log/ls-files`, and one `cp -a` to a private scratch copy outside the folder. All builds and checks ran in that copy. No other path on the Mac was accessed.
