#!/bin/sh
# Every test of RTS6x, against build/rts6x.lib only - no TI library is named anywhere here.
# The tools are found beside this checkout or on PATH; CPP11=, C90=, ASM6X=, LNK6X=, VM=, VMSIM=
# name them. Exit 1 if any test fails, each failure said.
cd "$(dirname "$0")/.." || exit 2
find_tool() { for t in "$@"; do [ -n "$t" ] && [ -x "$t" ] && { echo "$t"; return; }; done; }
CPP11=$(find_tool "${CPP11:-}" ../C++Optimize/cpp11.exe "$(command -v cpp11 2>/dev/null)")
C90=$(find_tool "${C90:-}" ../VM6747/Compiler-Ci/c90.exe "$(command -v c90 2>/dev/null)")
ASM6X=$(find_tool "${ASM6X:-}" ../ASM6x/build/asm6x.exe "$(command -v asm6x 2>/dev/null)")
LNK6X=$(find_tool "${LNK6X:-}" ../LNK6x/build/lnk6x.exe "$(command -v lnk6x 2>/dev/null)")
VM=$(find_tool "${VM:-}" ../VM6747/Emulator/vm6747.exe "$(command -v vm6747 2>/dev/null)")
VMSIM=$(find_tool "${VMSIM:-}" ../VM6747-sim/vm6747.exe "$(command -v vm6747sim 2>/dev/null)")
for need in "cpp11:$CPP11" "c90:$C90" "asm6x:$ASM6X" "lnk6x:$LNK6X" "vm6747:$VM" "vm6747sim:$VMSIM"; do
    [ -n "${need#*:}" ] || { echo "run.sh: no ${need%%:*} - build it or name it"; exit 2; }
done
# D=d runs it all against the Debug pair, rts6xd.lib and printf6xd.lib, built at -O0; R=r against
# the reference pair, rts6xr.lib and printf6xr.lib (make reference: D2's speed files as C++).
V=${R:-}${D:-}
LIB=build/rts6x$V.lib AR6X=build/ar6x.exe OUT=${OUT:-../build/RTS6x/test$V}
[ -f "$LIB" ] || { echo "run.sh: no $LIB - run make first"; exit 2; }
rm -rf "$OUT"; mkdir -p "$OUT"
pass=0 fail=0
ok()  { pass=$((pass + 1)); echo "  ok    $1"; }
bad() { fail=$((fail + 1)); echo "  FAIL  $1"; [ -n "${2:-}" ] && echo "$2" | sed 's/^/        /'; }

# provenance: the rule of docs/PROVENANCE.md
if r=$(sh tools/provenance 2>&1); then ok provenance; else bad provenance "$r"; fi

# ar6x: the index names every symbol the members define, each pointing at its own member
"$AR6X" -t "$LIB" > "$OUT/index.txt"
members=$(grep -c '^member ' "$OUT/index.txt"); syms=$(grep -c '^symbol ' "$OUT/index.txt")
if [ "$members" -gt 0 ] && ! grep -q '^symbol .* ?$' "$OUT/index.txt"; then ok "ar6x index: $members member(s), $syms symbol(s)"
else bad "ar6x index" "$(cat "$OUT/index.txt")"; fi

# layout: src/internal/rts6x.h against cpp11's and c90's headers, run on the emulator
if "$CPP11" -arch tms6747 -nologo -Isrc/internal -S tests/layout/layout.cpp -o "$OUT/layout-cpp11.s" > "$OUT/layout-cpp11.log" 2>&1 &&
   r=$("$VM" "$OUT/layout-cpp11.s" 2>&1); then ok "layout, cpp11's headers"; else bad "layout, cpp11's headers" "$r$(cat "$OUT/layout-cpp11.log")"; fi
if "$C90" -arch tms6747 -Isrc/internal -S tests/layout/layout.c -o "$OUT/layout-c90.s" > "$OUT/layout-c90.log" 2>&1 &&
   r=$("$VM" "$OUT/layout-c90.s" 2>&1); then ok "layout, c90's headers"; else bad "layout, c90's headers" "$r$(grep -i error "$OUT/layout-c90.log")"; fi

# link: lnk6x takes a member out of rts6x.lib and vm6747sim runs the image - 82 is 'R'
"$ASM6X" tests/link/probe.s -o "$OUT/probe.obj" &&
"$LNK6X" -mv6740 --abi=eabi tests/link/flat.cmd "$OUT/probe.obj" -l "$LIB" -o "$OUT/probe.out" > "$OUT/probe.log" 2>&1
"$VMSIM" --run "$OUT/probe.out" > /dev/null 2>&1; st=$?
if [ "$st" = 82 ]; then ok "link: lnk6x with rts6x.lib, run on vm6747sim"; else bad "link" "status $st, wanted 82; $(cat "$OUT/probe.log")"; fi

# printf6x.lib alone: the formats test at -O0 and -O2, linked with it and the stand-in entry only,
# run on vm6747sim, and its output held to the host's for the same calls (formats.expected)
PRINTFLIB=build/printf6x$V.lib
"$ASM6X" tests/link/start.s -o "$OUT/start.obj"
for level in -O0 -O2; do
    o="$OUT/formats$level"
    if "$CPP11" -arch tms6747 -nologo $level -S tests/printf/formats.cpp -o "$o.s" > "$o.log" 2>&1 &&
       "$ASM6X" "$o.s" -o "$o.obj" >> "$o.log" 2>&1 &&
       "$LNK6X" -mv6740 --abi=eabi --ram_model tests/link/flat.cmd "$OUT/start.obj" "$o.obj" -l "$PRINTFLIB" -o "$o.out" >> "$o.log" 2>&1; then
        "$VMSIM" --run --main-status "$o.out" > "$o.txt" 2>&1
        if diff tests/printf/formats.expected "$o.txt" > "$o.diff"; then ok "printf6x.lib: formats $level, as the host prints them"
        else bad "printf6x.lib: formats $level" "$(head -12 "$o.diff")"; fi
    else
        bad "printf6x.lib: formats $level did not build" "$(tail -3 "$o.log")"
    fi
done

# Whole programs on rts6x.lib alone - its own _c_int00, .cinit, constructors, exit - each built by
# its compiler (c90 for .c, cpp11 for .cpp) at -O0 and -O2, linked under both of lnk6x's models, run
# on vm6747sim; output and status (<name>.status) held to the host's. <name>.with: more to link in;
# <name>.input: what standard input reads.
for src in tests/m1/*.c tests/m1/*.cpp tests/m2/*.c tests/m2/*.cpp tests/m3/*.c tests/m3/*.cpp tests/m4/*.cpp tests/m5/*.cpp tests/m6/*.cpp tests/helpers/*.c tests/helpers/*.cpp; do
    [ -f "$src" ] || continue
    dir=$(dirname "$src")
    name=$(basename "$src"); name=${name%.*}
    want=$(cat "$dir/$name.status")
    # A text file's bytes are the host's: <name>.windows.expected is TI's Windows host's answer, if it differs.
    exp="$dir/$name.expected"
    case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) [ -f "$dir/$name.windows.expected" ] && exp="$dir/$name.windows.expected" ;; esac
    for level in -O0 -O2; do
        o="$OUT/$(basename "$dir")-$name$level"
        case "$src" in
            *.c) "$C90" -arch tms6747 $level -S "$src" -o "$o.s" > "$o.log" 2>&1 ;;
            *) "$CPP11" -arch tms6747 -nologo $level -S "$src" -o "$o.s" > "$o.log" 2>&1 ;;
        esac || { bad "$(basename "$dir") $name $level did not compile" "$(grep -i error "$o.log" | head -3)"; continue; }
        "$ASM6X" "$o.s" -o "$o.obj" >> "$o.log" 2>&1 || { bad "$(basename "$dir") $name $level did not assemble" "$(tail -3 "$o.log")"; continue; }
        extra=""
        if [ -f "$dir/$name.with" ]; then
            for w in $(cat "$dir/$name.with"); do "$ASM6X" "$w" -o "$o.$(basename "$w" .s).obj" >> "$o.log" 2>&1; extra="$extra $o.$(basename "$w" .s).obj"; done
        fi
        for model in rom ram; do
            if ! "$LNK6X" -mv6740 --abi=eabi --${model}_model tests/link/flat.cmd "$o.obj" $extra -l "$LIB" -o "$o.$model.out" >> "$o.log" 2>&1; then
                bad "$(basename "$dir") $name $level --${model}_model did not link" "$(tail -3 "$o.log")"; continue
            fi
            input=/dev/null
            [ -f "$dir/$name.input" ] && input="$dir/$name.input"
            "$VMSIM" --run "$o.$model.out" < "$input" > "$o.$model.txt" 2>&1; st=$?
            if [ "$st" = "$want" ] && diff -q "$exp" "$o.$model.txt" > /dev/null; then
                ok "$(basename "$dir") $name $level --${model}_model: output and status $st"
            else
                bad "$(basename "$dir") $name $level --${model}_model: status $st, wanted $want" "$(diff "$exp" "$o.$model.txt" | head -6)"
            fi
        done
    done
done

echo "run.sh: $pass passed, $fail failed"
[ "$fail" = 0 ]
