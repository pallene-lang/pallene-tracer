# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the MIT license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: MIT

OUTPUT_FILE = component.so

CC         = cc
CFLAGS     = -O2 -std=c99 -pedantic -Wall -Wextra -Wformat-security
LIBFLAGS   = -fPIC -shared

ifneq ($(findstring debug, $(MAKECMDGOALS)), )
	CFLAGS += -DPT_DEBUG
endif

.PHONY: all debug clean

all: $(OUTPUT_FILE)
debug: $(OUTPUT_FILE)

clean:
	rm -rf $(OUTPUT_FILE)

%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBFLAGS) $< -o $@

%.c: ptracer.h
