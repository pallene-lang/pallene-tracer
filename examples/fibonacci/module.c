/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

#define PT_IMPLEMENTATION
#include <ptracer.h>

#ifdef MODULE_DEBUG
/* ---------------- LUA INTERFACE ---------------- */

#define MODULE_LUA_FRAMEENTER(fnptr)                             \
    pt_fnstack_t *fnstack = lua_touserdata(L,                    \
        lua_upvalueindex(1));                                    \
    int _base = lua_gettop(L);                                   \
    PALLENE_TRACER_LUA_FRAMEENTER(L, fnstack, fnptr,             \
        lua_upvalueindex(2), _frame)

/* ---------------- LUA INTERFACE END ---------------- */

/* ---------------- C INTERFACE ---------------- */

#define MODULE_C_PROTO(signature, ...)    signature(pt_fnstack_t *fnstack, __VA_ARGS__)
#define MODULE_C_CALL(fn_name, ...)       fn_name(fnstack, __VA_ARGS__)

#define MODULE_C_FRAMEENTER()                                    \
    PALLENE_TRACER_C_FRAMEENTER(L, fnstack, __func__, __FILE__, _frame)

#define MODULE_C_SETLINE()                                         \
    pallene_tracer_setline(fnstack, __LINE__ + 1)

#define MODULE_C_FRAMEEXIT()                                     \
    pallene_tracer_frameexit(fnstack)

/* ---------------- C INTERFACE END ---------------- */

#else
/* ---------------- LUA INTERFACE ---------------- */
#define MODULE_LUA_FRAMEENTER(_)          int _base = lua_gettop(L);

/* ---------------- C INTERFACE ---------------- */

#define MODULE_C_PROTO(signature, ...)    signature(__VA_ARGS__)
#define MODULE_C_CALL(fn_name, ...)       fn_name(__VA_ARGS__)

#define MODULE_C_FRAMEENTER()
#define MODULE_C_SETLINE()
#define MODULE_C_FRAMEEXIT()

/* ---------------- C INTERFACE END ---------------- */
#endif // MODULE_DEBUG

#define MODULE_C_RET(expression)      \
    MODULE_C_FRAMEEXIT();             \
    return (expression)

MODULE_C_PROTO(int fib, lua_State *L, int n) {
    MODULE_C_FRAMEENTER();

    if(n <= 1) {
        MODULE_C_RET(n);
    }

    MODULE_C_SETLINE();
    MODULE_C_RET(MODULE_C_CALL(fib, L, n - 1) + MODULE_C_CALL(fib, L, n - 2));
}

int fib_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(fib_lua);

    /* Check the macro definitions for '_base'. */
    if(_base < 1) {
        luaL_error(L, "Expected atleast 1 parameter");
    }

    lua_pushvalue(L, 1);
    if(luai_unlikely(lua_isinteger(L, -1) == 0)) {
        luaL_error(L, "Expected the first argument to be an integer");
    }

    /* Dispatch. */
    int result = MODULE_C_CALL(fib, L, lua_tointeger(L, -1));
    lua_pushinteger(L, result);

    return 1;
}

int luaopen_module(lua_State *L) {
#ifdef MODULE_DEBUG
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);
#endif // MODULE_DEBUG

    lua_newtable(L);
    int table = lua_gettop(L);

    /* ---- fib ---- */
#ifdef MODULE_DEBUG
    /* One very good way to integrate our stack userdatum and finalizer
       object is by using Lua upvalues. */
    lua_pushlightuserdata(L, fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, fib_lua, 2);
#else
    lua_pushcfunction(L, fib_lua);
#endif // MODULE_DEBUG
    lua_setfield(L, table, "fib");

    return 1;
}
