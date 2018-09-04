# ---------------------------------------------------------------------
# diffiv flags

SPI = 2.0
CFLAGS = -Wall -O3 $(OSFLAGS)

# ---------------------------------------------------------------------
# OS parsing

ifeq ($(OS),Windows_NT)
	OSFLAGS = -shared
	GCC = x86_64-w64-mingw32-gcc.exe
	OUT = build/diffiv_windows.plugin
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		OSFLAGS = -shared -fPIC -DSYSTEM=OPUNIX
		OUT = build/diffiv_unix.plugin
	endif
	ifeq ($(UNAME_S),Darwin)
		OSFLAGS = -bundle -DSYSTEM=APPLEMAC
		OUT = build/diffiv_macosx.plugin
	endif
	GCC = gcc
endif

# ---------------------------------------------------------------------
# Rules

all: clean links diffiv

links:
	rm -f  src/lib
	rm -f  src/spi
	ln -sf ../lib src/lib
	ln -sf lib/spi-$(SPI) src/spi

diffiv: src/diffiv.c src/spi/stplugin.c
	mkdir -p ./build
	mkdir -p ./lib/plugin
	$(GCC) $(CFLAGS) -o $(OUT) src/spi/stplugin.c src/diffiv.c
	cp build/*plugin lib/plugin/

.PHONY: clean
clean:
	rm -f $(OUT)
