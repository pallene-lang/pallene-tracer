# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC := gcc
INSTALL_DIR    := /usr/local
INSTALL_INCDIR := $(INSTALL_DIR)/include
INSTALL_LIBDIR := $(INSTALL_DIR)/lib

.PHONY :
.SILENT:

install:
	cp lib/ptracer.h $(INSTALL_INCDIR)
	$(CC) -fPIC -O2 -shared src/ptracer.c -o libptracer.so
	cp libptracer.so $(INSTALL_LIBDIR)
	echo "Done."

uninstall: 
	rm -rf $(INSTALL_INCDIR)/ptracer.h
	rm -rf $(INSTALL_LIBDIR)/libptracer.so
	echo "Done."

clean: 
	rm -rf ptinit/*.o
	rm -rf examples/*/*.so
	rm -rf *.so
