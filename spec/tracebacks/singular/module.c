/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* Static use of the library would suffice. */
#define PT_IMPLEMENTATION
#include <ptracer.h>

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */

#define MODULE_C_FRAMEENTER()                                    \
    PALLENE_TRACER_C_FRAMEENTER(L, fnstack, __func__, __FILE__, _frame_c)

#define MODULE_SETLINE()                                         \
    pallene_tracer_setline(fnstack, __LINE__ + 1)

#define MODULE_C_FRAMEEXIT()                                     \
    pallene_tracer_frameexit(fnstack)

/* ---------------- FOR C INTERFACE FUNCTIONS END ---------------- */

/* ---------------- LUA INTERFACE FUNCTIONS ---------------- */

#define MODULE_LUA_FRAMEENTER(fnptr)                             \
    pt_fnstack_t *fnstack = lua_touserdata(L,                    \
        lua_upvalueindex(1));                                    \
    int _base = lua_gettop(L);                                   \
    PALLENE_TRACER_LUA_FRAMEENTER(L, fnstack, fnptr,             \
        lua_upvalueindex(2), _frame_lua)                         \
    MODULE_C_FRAMEENTER()

/* ---------------- LUA INTERFACE FUNCTIONS END ---------------- */

void lifes_good_fn(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    MODULE_SETLINE();
    luaL_error(L, "Life's !good");

    MODULE_C_FRAMEEXIT();
}

int singular_fn_1(lua_State *L) {
    MODULE_LUA_FRAMEENTER(singular_fn_1);

    /* Call some C function. */
    MODULE_SETLINE();
    lifes_good_fn(L, fnstack);

    return 0;
}

int luaopen_module(lua_State *L) {
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);

    lua_newtable(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- singular_fn_1 ---- */
    lua_pushlightuserdata(L, fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, singular_fn_1, 2);
    lua_setfield(L, -2, "singular_fn_1");

    return 1;
}
