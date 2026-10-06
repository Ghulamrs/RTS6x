#!/bin/sh
# Spec: none - src/math on the C6000 against src/math on the host: math-values.cpp built both ways
# (clang++ over the classes; cpp11 at -O0 and -O2, asm6x, lnk6x with build/rts6x.lib, run on
# vm6747sim) must print the same bits. The host side is the one math-check.sh measures.
cd "$(dirname "$0")/../../.." || exit 2
OUT=${OUT:-../build/RTS6x-math/host}
mkdir -p "$OUT"
CPP11=${CPP11:-../C++Optimize/cpp11.exe} ASM6X=${ASM6X:-../ASM6x/build/asm6x.exe}
LNK6X=${LNK6X:-../LNK6x/build/lnk6x.exe} VMSIM=${VMSIM:-../VM6747-sim/vm6747.exe}
clang++ -DHOST -std=c++14 -O2 -ffp-contract=off -o "$OUT/values-host" tests/m3/host/math-values.cpp \
    $(ls src/math/*.cpp | grep '/[A-Z]') src/helpers/SoftFloat.cpp src/helpers/FloatRounding.cpp src/misc/ErrorNumber.cpp || exit 1
"$OUT/values-host" > "$OUT/values-host.txt"
bad=0
for level in -O0 -O2; do
    o="$OUT/values$level"
    "$CPP11" -arch tms6747 -nologo $level -S tests/m3/host/math-values.cpp -o "$o.s" && "$ASM6X" "$o.s" -o "$o.obj" &&
    "$LNK6X" -mv6740 --abi=eabi --rom_model tests/link/flat.cmd "$o.obj" -l build/rts6x.lib -o "$o.out" > "$o.log" 2>&1 || { echo "math-target-check: $level did not build"; bad=1; continue; }
    "$VMSIM" --run "$o.out" > "$o.txt"
    if cmp -s "$OUT/values-host.txt" "$o.txt"; then echo "math-target-check: $level, $(wc -l < "$o.txt" | tr -d ' ') results bit for bit as on the host"
    else echo "math-target-check: $level differs:"; diff "$OUT/values-host.txt" "$o.txt" | head -10; bad=1; fi
done
exit $bad
