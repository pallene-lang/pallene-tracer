# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the BSD-3-Clause license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: BSD-3-Clause 

CC := gcc
CURRENT_DIR := $(shell realpath --relative-to=$(shell pwd) $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))
LIB_DIR := /usr/local/lib

.PHONY :
.SILENT:

all: 
	$(CC) -fPIC -O2 -shared $(CURRENT_DIR)/module_a.c -o $(CURRENT_DIR)/module_a.so -lptracer -Wl,-rpath=$(LIB_DIR) -L$(LIB_DIR)
	$(CC) -fPIC -O2 -shared $(CURRENT_DIR)/module_b.c -o $(CURRENT_DIR)/module_b.so -lptracer -Wl,-rpath=$(LIB_DIR) -L$(LIB_DIR)
