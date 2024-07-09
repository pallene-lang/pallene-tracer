# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC := gcc
CURRENT_DIR := $(shell dirname $(MAKEFILE_LIST))
LIB_DIR := /usr/local/lib

.PHONY :
.SILENT:

all: 
	$(CC) -fPIC $(CFLAGS) -shared module_a.c -o module_a.so -lptracer -Wl,-rpath=$(LIB_DIR) -L$(LIB_DIR)
	$(CC) -fPIC $(CFLAGS) -shared module_b.c -o module_b.so -lptracer -Wl,-rpath=$(LIB_DIR) -L$(LIB_DIR)
