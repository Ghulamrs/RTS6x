#!/bin/sh
# Spec: none - the host accuracy check of src/math: math-accuracy.cpp built with the host's clang++
# (no FMA contraction, so its arithmetic is the C674x's binary64 one), run over N inputs per
# function against libm, then every differing input and a sample judged by math-reference.py.
#   tests/m3/host/math-check.sh [N]         N defaults to 2000000
#   FTZ=1 tests/m3/host/math-check.sh       also run under flush-to-zero (the C674x's subnormals)
cd "$(dirname "$0")/../../.." || exit 2
N=${1:-2000000}
OUT=${OUT:-../build/RTS6x-math/host}
mkdir -p "$OUT"
CXX=${CXX:-clang++}
SRCS="tests/m3/host/math-accuracy.cpp $(ls src/math/*.cpp | grep '/[A-Z]') src/helpers/SoftFloat.cpp src/helpers/FloatRounding.cpp src/misc/ErrorNumber.cpp"
$CXX -std=c++14 -O2 -ffp-contract=off -Wall -Wextra -Werror -o "$OUT/math-accuracy" $SRCS || exit 1
"$OUT/math-accuracy" "$N" --dump "$OUT/dump.txt" || exit 1
python3 tests/m3/host/math-reference.py "$OUT/dump.txt" || exit 1
if [ "${FTZ:-0}" = 1 ]; then
    # Built at -O0: at -O2 the host compiler turns a test of the bits into a floating compare,
    # which flush-to-zero then answers differently - something cpp11 does not do.
    $CXX -std=c++14 -O0 -ffp-contract=off -o "$OUT/math-accuracy-O0" $SRCS || exit 1
    "$OUT/math-accuracy-O0" 300000 --all "$OUT/plain.txt" > /dev/null
    "$OUT/math-accuracy-O0" 300000 --ftz --all "$OUT/ftz.txt" > /dev/null
    echo "flush-to-zero: results that differ, by function (inputs the host's own arithmetic flushed, and"
    echo "atan2's quotient, which the C674x divides in software):"
    diff "$OUT/plain.txt" "$OUT/ftz.txt" | grep '^>' | awk '{print "  " $2}' | sort | uniq -c
fi
