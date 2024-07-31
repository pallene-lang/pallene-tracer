/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* Static use of the library would suffice. */
#define PT_IMPLEMENTATION
#include <ptracer.h>

/* ---------------- LUA INTERFACE FUNCTIONS ---------------- */

#define MODULE_LUA_FRAMEENTER(fnptr)                             \
    pt_fnstack_t *fnstack = lua_touserdata(L,                    \
        lua_upvalueindex(1));                                    \
    int _base = lua_gettop(L);                                   \
    PALLENE_TRACER_LUA_FRAMEENTER(L, fnstack, fnptr,             \
        lua_upvalueindex(2), _frame)

/* ---------------- LUA INTERFACE FUNCTIONS END ---------------- */

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */

#define MODULE_C_FRAMEENTER()                                    \
    PALLENE_TRACER_C_FRAMEENTER(L, fnstack, __func__, __FILE__, _frame)

#define MODULE_SETLINE()                                         \
    pallene_tracer_setline(fnstack, __LINE__ + 1)

#define MODULE_C_FRAMEEXIT()                                     \
    pallene_tracer_frameexit(fnstack)

/* ---------------- FOR C INTERFACE FUNCTIONS END ---------------- */

void module_fn(lua_State *L, pt_fnstack_t *fnstack, int depth) {
    MODULE_C_FRAMEENTER();

    if(depth == 0) 
        lua_pushinteger(L, depth);
    else lua_pushinteger(L, depth - 1);

    /* Set line number to current active frame in the Pallene callstack and
       call the function which is already in the Lua stack. */
    MODULE_SETLINE();
    lua_call(L, 1, 0);

    MODULE_C_FRAMEEXIT();
}

int module_fn_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(module_fn_lua);

    /* Look at the macro definitions. */
    if(luai_unlikely(_base < 2)) 
        luaL_error(L, "Expected atleast 2 parameters");

    /* ---- `lua_fn` ---- */
    lua_pushvalue(L, 1);
    if(luai_unlikely(lua_isfunction(L, -1) == 0)) 
        luaL_error(L, "Expected the first parameter to be a function");

    lua_pushvalue(L, 2);
    if(luai_unlikely(lua_isinteger(L, -1) == 0)) 
        luaL_error(L, "Expected the second parameter to be an integer");

    int depth = lua_tointeger(L, -1);
    lua_pop(L, 1);

    /* Dispatch. */
    module_fn(L, fnstack, depth);

    return 0;
}

int luaopen_module(lua_State *L) {
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);

    lua_newtable(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- module_fn_1 ---- */
    lua_pushlightuserdata(L, fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, module_fn_lua, 2);
    lua_setfield(L, -2, "module_fn");

    return 1;
}
