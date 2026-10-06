# RTS6x: rts6x.lib built by our own tools only - cpp11 for C++, asm6x for assembly, ar6x to pack.
# Two of each library: rts6x.lib and printf6x.lib at -O2 for a Release build, rts6xd.lib and
# printf6xd.lib at -O0 with _DEBUG for a Debug one - the flags RIDE gives a program in each.
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
DOBJDIR ?= $(OBJDIR)d
BINDIR ?= build
AR6X    = $(BINDIR)/ar6x.exe
LIB     = $(BINDIR)/rts6x.lib
LIBD    = $(BINDIR)/rts6xd.lib
RFLAGS  = -O2 -DNDEBUG=1
DFLAGS  = -O0 -D_DEBUG=1
CPPSRC  = $(shell find src -name '*.cpp' | sort)
# eh-none/ is the stand-in for the unwinder, in printf6x.lib alone; rts6x.lib has the real one, eh/.
ASMSRC  = $(shell find src -name '*.s' -not -path 'src/eh-none/*' | sort)
OBJS    = $(patsubst src/%.cpp,$(OBJDIR)/%.obj,$(CPPSRC)) $(patsubst src/%.s,$(OBJDIR)/%.obj,$(ASMSRC))
DOBJS   = $(patsubst $(OBJDIR)/%,$(DOBJDIR)/%,$(OBJS))
HEADERS = $(shell find src -name '*.h')

# printf6x.lib: printf, fprintf, sprintf and wprintf, with only what they need under them -
# printf6x.members lists the sources, for this file and build.cmd alike.
PRINTFSRC = $(shell cat printf6x.members)
PRINTFOBJ = $(patsubst src/%,$(OBJDIR)/%,$(PRINTFSRC:.cpp=.obj))
PRINTFOBJ := $(PRINTFOBJ:.s=.obj)
PRINTFLIB = $(BINDIR)/printf6x.lib
PRINTFOBJD = $(patsubst $(OBJDIR)/%,$(DOBJDIR)/%,$(PRINTFOBJ))
PRINTFLIBD = $(BINDIR)/printf6xd.lib

all: $(LIB) $(PRINTFLIB) $(LIBD) $(PRINTFLIBD)

$(PRINTFLIB): $(AR6X) $(PRINTFOBJ) printf6x.members tools/provenance
	sh tools/provenance
	$(AR6X) -r $@ $(PRINTFOBJ)

$(PRINTFLIBD): $(AR6X) $(PRINTFOBJD) printf6x.members tools/provenance
	sh tools/provenance
	$(AR6X) -r $@ $(PRINTFOBJD)

$(AR6X): tools/ar6x/ar6x.cpp
	@mkdir -p $(BINDIR)
	$(CXX) $(HOSTFLAGS) -o $@ $<

# Every C++ source compiled to assembly, then assembled: the .s is kept beside the object to read.
$(OBJDIR)/%.obj: src/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP11) -arch tms6747 -nologo $(RFLAGS) -Isrc/internal -S $< -o $(@:.obj=.s)
	$(ASM6X) $(@:.obj=.s) -o $@

$(OBJDIR)/%.obj: src/%.s
	@mkdir -p $(dir $@)
	$(ASM6X) $< -o $@

$(DOBJDIR)/%.obj: src/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CPP11) -arch tms6747 -nologo $(DFLAGS) -Isrc/internal -S $< -o $(@:.obj=.s)
	$(ASM6X) $(@:.obj=.s) -o $@

$(DOBJDIR)/%.obj: src/%.s
	@mkdir -p $(dir $@)
	$(ASM6X) $< -o $@

$(LIB): $(AR6X) $(OBJS) tools/provenance
	sh tools/provenance
	$(AR6X) -r $@ $(OBJS)

$(LIBD): $(AR6X) $(DOBJS) tools/provenance
	sh tools/provenance
	$(AR6X) -r $@ $(DOBJS)

check: all
	sh tests/run.sh
	D=d sh tests/run.sh

clean:
	rm -rf $(OBJDIR) $(DOBJDIR) $(BINDIR)

.PHONY: all check clean
