# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_BIN = demo1.elf
EE_OBJS = demo1.o
EE_LIBS = -ldraw -lgraph -lmath3d -lmf -lpacket -ldma -lpadx

all: $(EE_BIN) sio2man.irx mtapman.irx padman.irx
	$(EE_STRIP) --strip-all $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) padman.irx mtapman.irx sio2man.irx
	
sio2man.irx:
	cp $(PS2SDK)/iop/irx/sio2man.irx $@
	
mtapman.irx:
	cp $(PS2SDK)/iop/irx/mtapman.irx $@

padman.irx:
	cp $(PS2SDK)/iop/irx/padman.irx $@

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
