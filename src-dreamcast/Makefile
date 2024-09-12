DC_TOOLCHAIN=$(DREAMSDK_HOME)/msys/1.0/opt/toolchains/dc

# misc libraries goes here
# DC_LIBS=-lfastmem
# required by FontAtlas (the order is important)
DC_LIBS:=$(DC_LIBS) -lfreetype -lbz2 -lpng -lz
# required by SoundBridge
DC_LIBS:=$(DC_LIBS) -ltremor# -logg
# required by LuaScript
DC_LIBS:=$(DC_LIBS) -llua

INCLUDE=-Iinclude

SOURCES=$(wildcard src/*.c)
SOURCES:=$(SOURCES) $(wildcard src/utils/*.c)
SOURCES:=$(SOURCES) $(wildcard src/sound/*.c)
SOURCES:=$(SOURCES) $(wildcard src/platform/*.c)
SOURCES:=$(SOURCES) $(wildcard src/image/*.c)
SOURCES:=$(SOURCES) $(wildcard src/game/common/*.c)
SOURCES:=$(SOURCES) $(wildcard src/game/funkin/*.c)
SOURCES:=$(SOURCES) $(wildcard src/game/gameplay/helpers/*.c)
SOURCES:=$(SOURCES) $(wildcard src/game/gameplay/*.c)
SOURCES:=$(SOURCES) $(wildcard src/game/helpers/*.c)
SOURCES:=$(SOURCES) $(wildcard src/game/*.c)
SOURCES:=$(SOURCES) $(wildcard src/font/*.c)
SOURCES:=$(SOURCES) $(wildcard src/animation/*.c)
SOURCES:=$(SOURCES) $(wildcard src/externals/fontatlas/*.c)
SOURCES:=$(SOURCES) $(wildcard src/externals/luascript/*.c)
SOURCES:=$(SOURCES) $(wildcard src/externals/sndbridge/*/*.c)
SOURCES:=$(SOURCES) $(wildcard src/externals/sndbridge/*.c)

# separate FFGraph and VideoPlayer to compile with agressive optimizations
SOURCES_FFGRAPH=$(wildcard src/externals/ffgraph/*.c)
SOURCES_FFGRAPH:=$(SOURCES_FFGRAPH) $(wildcard src/video/*.c)

TARGET=output/kdy_e.elf

KDY_FLAGS=-DSNDBRINDE_TREMOR -DFRAME_POINTERS#-DNO_FORCE_FLOAT64_IMPLEMENTATION

# ***** debug flags *****
KDY_FLAGS2=-ggdb3 -Wa,-g -Wl,-g -DDEBUG#-Wpadded
KDY_FLAGS3=-O0 -gdwarf-5 -fno-omit-frame-pointer -fvar-tracking -fvar-tracking-assignments -fstack-protector-all -fno-eliminate-unused-debug-symbols -fno-eliminate-unused-debug-types
KDY_FLAGS3_FFGPRAPH=$(KDY_FLAGS3)
KDY_FLAGS_LINKER=

# ***** release flags *****
KDY_FLAGS2_RELEASE=-ffunction-sections -fdata-sections
KDY_FLAGS3_RELEASE=-O2 -freorder-blocks-algorithm=simple -flto=auto
KDY_FLAGS3_FFGPRAPH_RELEASE=-Ofast -ffast-math -freorder-blocks-algorithm=simple -flto=auto
KDY_FLAGS_LINKER_RELEASE=-freorder-blocks-algorithm=simple -flto=auto

KOS_INCLUDE=-I$(DC_TOOLCHAIN)/kos/include
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos/kernel/arch/dreamcast/include
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos/addons/include
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos-ports/include
# required by SoundBridge
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos-ports/include/ogg
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos/kernel/arch/dreamcast/sound
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos/kernel/arch/dreamcast/include/dc/sound
# required by FontAtlas
KOS_INCLUDE:=$(KOS_INCLUDE) -I$(DC_TOOLCHAIN)/kos-ports/include/freetype2

ifeq (build-release,$(filter build-release,$(MAKECMDGOALS)))
#	change to release flags
	KDY_FLAGS2=$(KDY_FLAGS2_RELEASE)
	KDY_FLAGS3=$(KDY_FLAGS3_RELEASE)
	KDY_FLAGS3_FFGPRAPH=$(KDY_FLAGS3_FFGPRAPH_RELEASE)
	KDY_FLAGS_LINKER=$(KDY_FLAGS_LINKER_RELEASE)
endif

OBJECTS=$(SOURCES:src/%.c=output/obj/%.o)
OBJECTS_FFGRAPH=$(SOURCES_FFGRAPH:src/%.c=output/obj_ffgraph/%.o)

LIB_DIRS=-L$(DC_TOOLCHAIN)/kos/lib/dreamcast
LIB_DIRS:=$(LIB_DIRS) -L$(DC_TOOLCHAIN)/kos/addons/lib/dreamcast
LIB_DIRS:=$(LIB_DIRS) -L$(DC_TOOLCHAIN)/kos-ports/lib


#
# Note: KallistiOS and the toolchain must be recompiled to support 64bit floating-point.
# Use "NO_FORCE_FLOAT64_IMPLEMENTATION" macro to keep double as float (aka sizeof(double) == 4)
#
SH4_FP_MODE=-m4-single-only# for SH4 FPU that only supports single-precision arithmetic.
#SH4_FP_MODE=-m4-single# for SH4 FPU assuming the floating-point unit is in single-precision mode by default.

FLAGS1=-std=c17 -Wall -Wextra -Wno-sign-compare -Wsign-conversion -Wfloat-conversion -Wstring-compare $(KDY_FLAGS2) $(KDY_FLAGS)
FLAGS2=-fno-builtin -ml $(SH4_FP_MODE) -matomic-model=soft-imask -ftls-model=local-exec -D_arch_dreamcast -D_arch_sub_pristine
FLAGS3=-fno-builtin -Wl,-Ttext=0x8c010000 -T$(DC_TOOLCHAIN)/kos/utils/ldscripts/shlelf.xc -nodefaultlibs $(KDY_FLAGS_LINKER)
KALLISTI=-Wl,--start-group -lkallisti -lc -lgcc -lm -Wl,--end-group



output/obj/%.o: src/%.c
	$(info compile $<)
	@mkdir -p $(dir $@)
	@sh-elf-gcc $(FLAGS1) $(KOS_INCLUDE) $(INCLUDE) $(FLAGS2) $(KDY_FLAGS3) -c $< -o $@

output/obj_ffgraph/%.o : src/%.c
	$(info ffgraph compile $<)
	@mkdir -p $(dir $@)
	@sh-elf-gcc $(FLAGS1) $(KOS_INCLUDE) $(INCLUDE) $(FLAGS2) $(KDY_FLAGS3_FFGPRAPH) -c $< -o $@

$(TARGET): $(OBJECTS_FFGRAPH) $(OBJECTS)
	$(info linking $(TARGET))
	@sh-elf-gcc $(FLAGS1) $(KOS_INCLUDE) $(INCLUDE) $(FLAGS2) $(FLAGS3) $(LIB_DIRS) $(KDY_FLAGS2) -o $(TARGET) $(OBJECTS_FFGRAPH) $(OBJECTS) $(DC_LIBS) $(KALLISTI)

.PHONY: clean
clean:
	@rm -f $(OBJECTS)
	@rm -f -v output/kdy_e.cdi
	@rm -f -v output/kdy_e.bin
	@rm -f -v output/kdy_e.elf
	@echo -e "\033[48;5;81m obj folder was cleared \033[0m"

.PHONY: remove
remove: clean
	@$(rm) $(TARGET)
	@$(info $(TARGET) deleted)


build-debug: $(TARGET)
	@$(info building cdi...)
#	@mkdir -p output/cd_root
#	makeip -g "KDMY-ENGINE DEBUG" -c "KAPODAMY" -f output/IP.bin
	elf2bin $(TARGET) output/kdy_e.bin
	scramble output/kdy_e.bin output/cd_root/1ST_READ.BIN
#	rm -f -v output/kdy_e.cdi
#	the "--joliet", "--rock" or "--joliet-rock" is required to get proper filenames
	makedisc output/kdy_e.cdi ./output/cd_root output/IP.BIN "KDMY-ENGINE" --data --rock ./cd_root_sort_rules.txt>/dev/null

	@echo -e "\033[38;5;15m\033[37;42m objs+elf+cdi build success \033[0m"


build-release: $(TARGET)
	@$(info building cdi...)
#	@mkdir -p output/cd_root
	makeip -f ip.txt output/IP.bin
	elf2bin $(TARGET) output/kdy_e.bin
	scramble output/kdy_e.bin output/cd_root/1ST_READ.BIN
#	rm -f -v output/kdy_e.cdi
#	the "--joliet", "--rock" or "--joliet-rock" is required to get proper filenames
	makedisc output/kdy_e.cdi ./output/cd_root output/IP.BIN "KDMY-ENGINE" --data --rock ./cd_root_sort_rules.txt>/dev/null


	@echo -e "\033[38;5;15m\033[37;42m objs+elf+cdi build success \033[0m"
