# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC := gcc

.PHONY : all
.SILENT:

all: 
	$(CC) -std=c99 -pedantic -Wextra -Wall -fPIC -DPT_DEBUG -shared module.c -o module.so
