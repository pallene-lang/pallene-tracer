/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

#define PT_IMPLEMENTATION
#include <ptracer.h>

int luaopen_ptinit(lua_State *L) {
    /* We are not doing anything with the Userdata. */
    (void) pallene_tracer_init(L);
    /* The init function pushes the finalizer object onto the Lua stack. Which
       we also do not need. */
    lua_pop(L, 1);

    return 0;
}
