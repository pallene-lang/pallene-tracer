# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC := gcc
INSTALL_DIR    := /usr/local
INSTALL_INCDIR := $(INSTALL_DIR)/include
INSTALL_LIBDIR := $(INSTALL_DIR)/lib

.PHONY: install ptracer_header libptracer uninstall clean

install: ptracer_header libptracer
	cp libptracer.so $(INSTALL_LIBDIR)

# We need the `ptracer.h` header to be installed first.
ptracer_header: 
	cp lib/ptracer.h $(INSTALL_INCDIR)

libptracer:
	$(CC) -fPIC -DPT_DEBUG -O2 -shared src/ptracer.c -o libptracer.so

uninstall: 
	rm -rf $(INSTALL_INCDIR)/ptracer.h
	rm -rf $(INSTALL_LIBDIR)/libptracer.so

clean: 
	rm -rf ptinit/*.o
	rm -rf examples/*/*.so
	rm -rf *.so
