# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = Compilation.elf
EE_OBJS = Compilation.o
EE_LIBS = -ldraw -lgraph -lmath3d -lmf -lpacket -ldma

OUT_DIR = ./bin/
MK_DIR = mkdir -p $(OUT_DIR)

all: 
	$(MK_DIR)
	+$(MAKE) $(EE_BIN)
	$(EE_STRIP) --strip-all $(EE_BIN)
	mv $(EE_BIN) $(OUT_DIR)
	mv $(EE_OBJS) $(OUT_DIR)

clean:
	rm -Rf $(OUT_DIR)$(EE_BIN) $(OUT_DIR)$(EE_OBJS)

run: $(OUT_DIR)$(EE_BIN)
	ps2client execee host:$(OUT_DIR)$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
