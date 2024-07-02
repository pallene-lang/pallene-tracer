# Copyright (c) 2024, The Pallene Developers
# Pallene Tracer is licensed under the BSD-3-Clause license.
# Please refer to the LICENSE and AUTHORS files for details
# SPDX-License-Identifier: BSD-3-Clause 

LUA_INCDIR := /usr/local/include/

install:
	cp -f lib/ptracer.h $(LUA_INCDIR)

remove: 
	rm -rf $(LUA_INCDIR)/ptracer.h
