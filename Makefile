# RTS6x: rts6x.lib built by our own tools only - cpp11 for C++, asm6x for assembly, ar6x to pack.
# ar6x is built first with the host's compiler, like ASM6x and LNK6x. Objects live outside the
# checkout, as the siblings keep theirs; CPP11=, ASM6X=, LNK6X=, VMSIM= name the tools.
ifeq ($(origin CXX),default)
  ifneq ($(shell command -v clang++ 2>/dev/null),)
    CXX := clang++
  else
    CXX := g++
  endif
endif
HOSTFLAGS = -std=c++14 -O2 -g -Wall -Wextra -Werror -pedantic
CPP11  ?= ../C++Optimize/cpp11.exe
ASM6X  ?= ../ASM6x/build/asm6x.exe
OBJDIR ?= ../build/RTS6x/obj
BINDIR ?= build
AR6X    = $(BINDIR)/ar6x.exe
LIB     = $(BINDIR)/rts6x.lib
CPPSRC  = $(shell find src -name '*.cpp' | sort)
# eh-none/ is the stand-in for the unwinder: in printf6x.lib, and in rts6x.lib until M5 brings the real one.
ASMSRC  = $(shell find src -name '*.s' | sort)
OBJS    = $(patsubst src/%.cpp,$(OBJDIR)/%.obj,$(CPPSRC)) $(patsubst src/%.s,$(OBJDIR)/%.obj,$(ASMSRC))
HEADERS = $(shell find src -name '*.h')

# printf6x.lib: printf, fprintf, sprintf and wprintf, with only what they need under them -
# the formatter's classes, the FILE table and the host channel.
PRINTFSRC = src/host/CioChannel.cpp $(addprefix src/stdio/,DecimalDigits.cpp FormatCharacter.cpp FormatFloat.cpp FormatInteger.cpp FormatSpec.cpp Formatter.cpp IntegerDigits.cpp OutputSink.cpp fprintf.cpp ftable.cpp printf.cpp sprintf.cpp wprintf.cpp)
PRINTFOBJ = $(patsubst src/%.cpp,$(OBJDIR)/%.obj,$(PRINTFSRC)) $(OBJDIR)/host/cio.obj $(OBJDIR)/eh-none/pr3.obj
PRINTFLIB = $(BINDIR)/printf6x.lib

all: $(LIB) $(PRINTFLIB)

$(PRINTFLIB): $(AR6X) $(PRINTFOBJ) tools/provenance
	sh tools/provenance
	$(AR6X) -r $@ $(PRINTFOBJ)

$(AR6X): tools/ar6x/ar6x.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(HOSTFLAGS) -o $@ $<

# Every C++ source compiled to assembly, then assembled: the .s is kept beside the object to read.
$(OBJDIR)/%.obj: src/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP11) -arch tms6747 -nologo -O2 -Isrc/internal -S $< -o $(@:.obj=.s)
	$(ASM6X) $(@:.obj=.s) -o $@

$(OBJDIR)/%.obj: src/%.s
	@mkdir -p $(dir $@)
	$(ASM6X) $< -o $@

$(LIB): $(AR6X) $(OBJS) tools/provenance
	sh tools/provenance
	$(AR6X) -r $@ $(OBJS)

check: $(LIB)
	sh tests/run.sh

clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all check clean
