# RTS6x

Our own run-time support library for the TMS320C6747, written from published
interface specifications - no TI, GNU or LLVM run-time code. A program built by
cpp11 or c90, ASM6x and LNK6x links `rts6x.lib` in place of TI's `rts6740`.

- Phase 1, the analysis: `docs/ANALYSIS.md`
- Phase 2, the organisation and the milestones: `docs/ORGANISATION.md`

Built so far: `build/rts6x.lib`, and `build/printf6x.lib` - `printf`, `fprintf`, `sprintf` and
`wprintf` alone, with the classes under them - whose output matches the host's for every C
conversion, on vm6747sim and on TI's CCS 5.5 simulator. `make check` runs every test.
