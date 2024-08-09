# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC     := gcc
CFLAGS := -DPT_DEBUG -O2 -std=c99 -pedantic -Wall -Wextra

LUA_DIR := /usr

INSTALL_DIR    := /usr/local
INSTALL_INCDIR := $(INSTALL_DIR)/include
INSTALL_LIBDIR := $(INSTALL_DIR)/lib
INSTALL_BINDIR := $(INSTALL_DIR)/bin

.PHONY: install ptracer_header pt-run libptracer uninstall clean

install: ptracer_header pt-run
	cp pt-run $(INSTALL_BINDIR)

# We need the `ptracer.h` header to be installed first.
ptracer_header: 
	cp lib/ptracer.h $(INSTALL_INCDIR)

pt-run: 
	$(CC) $(CFLAGS) src/pt-run/main.c -o pt-run -llua -lm -Wl,-E -L$(LUA_DIR)/lib

uninstall: 
	rm -rf $(INSTALL_INCDIR)/ptracer.h
	rm -rf $(INSTALL_BINDIR)/pt-run

clean: 
	rm -rf examples/*/*.so
	rm -rf spec/tracebacks/*/*.so
	rm -rf *.so
	rm -rf pt-run

