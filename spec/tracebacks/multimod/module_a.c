/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* This time we would be doing dynamic linking. */
#include <ptracer.h>

/* ---------------- LUA INTERFACE FUNCTIONS ---------------- */

#define MODULE_LUA_FRAMEENTER(fnptr)                             \
    pt_fnstack_t *fnstack = lua_touserdata(L,                    \
        lua_upvalueindex(1));                                    \
    int _base = lua_gettop(L);                                   \
    PALLENE_TRACER_LUA_FRAMEENTER(L, fnstack, fnptr,             \
        lua_upvalueindex(2), _frame)

/* ---------------- LUA INTERFACE FUNCTIONS END ---------------- */

/* ---------------- C INTERFACE FUNCTIONS ---------------- */

#define MODULE_C_FRAMEENTER()                                    \
    PALLENE_TRACER_C_FRAMEENTER(L, fnstack, __func__, __FILE__, _frame)

#define MODULE_SETLINE()                                         \
    pallene_tracer_setline(fnstack, __LINE__ + 1)

#define MODULE_C_FRAMEEXIT()                                     \
    pallene_tracer_frameexit(fnstack)

/* ---------------- C INTERFACE FUNCTIONS END ---------------- */

void some_mod_fn(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    // Other code...

    MODULE_SETLINE();
    lua_call(L, 0, 0);

    // Other code...

    MODULE_C_FRAMEEXIT();
}

int some_mod_fn_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(some_mod_fn_lua);

    /* Look at the macro definition. */
    if(_base < 1) 
        luaL_error(L, "Expected atleast 1 argument");

    lua_pushvalue(L, 1);
    if(luai_unlikely(lua_isfunction(L, -1) == 0))
        luaL_error(L, "Expected the first argument to be a function");

    /* Dispatch. */
    some_mod_fn(L, fnstack);

    return 0;
}

int luaopen_module_a(lua_State *L) {
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);

    lua_newtable(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- singular_fn_1 ---- */
    lua_pushlightuserdata(L, fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, some_mod_fn_lua, 2);
    lua_setfield(L, -2, "some_mod_fn");

    return 1;
}

