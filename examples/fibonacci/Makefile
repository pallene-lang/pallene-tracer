# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT 

CC := gcc

.PHONY : all debug clean
.SILENT:

all: 
	$(CC) -std=c99 -pedantic -Wall -Wextra -fPIC -O2 -shared fibonacci.c -o fibonacci.so

debug: 
	$(CC) -std=c99 -pedantic -Wall -Wextra -fPIC -O2 -DPT_DEBUG -shared fibonacci.c -o fibonacci.so

clean: 
	rm -rf fibonacci.so
