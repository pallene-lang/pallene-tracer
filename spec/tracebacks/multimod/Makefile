# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC := gcc
LIB_DIR := /usr/local/lib

.PHONY : all
.SILENT:

all: 
	$(CC) -std=c99 -pedantic -Wall -Wextra -fPIC -DPT_DEBUG -shared module_a.c -o module_a.so
	$(CC) -std=c99 -pedantic -Wall -Wextra -fPIC -DPT_DEBUG -shared module_b.c -o module_b.so
