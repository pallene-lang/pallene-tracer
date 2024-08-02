/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* Static use of the library would suffice. */
#define PT_IMPLEMENTATION
#include <ptracer.h>

/* Here goes user specific macros when Pallene Tracer debug mode is active. */
#ifdef PALLENE_TRACER_DEBUG
#define MODULE_GET_FNSTACK                                       \
    pt_fnstack_t *fnstack = lua_touserdata(L,                    \
        lua_upvalueindex(1))
#else
#define MODULE_GET_FNSTACK 
#endif // PALLENE_TRACER_DEBUG

/* ---------------- LUA INTERFACE FUNCTIONS ---------------- */

#define MODULE_LUA_FRAMEENTER(fnptr)                             \
    MODULE_GET_FNSTACK;                                          \
    PALLENE_TRACER_LUA_FRAMEENTER(L, fnstack, fnptr,             \
        lua_upvalueindex(2), _frame)

/* ---------------- LUA INTERFACE FUNCTIONS END ---------------- */

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */

#define MODULE_C_FRAMEENTER()                                    \
    MODULE_GET_FNSTACK;                                          \
    PALLENE_TRACER_GENERIC_C_FRAMEENTER(L, fnstack, _frame)

#define MODULE_C_SETLINE()                                       \
    PALLENE_TRACER_GENERIC_C_SETLINE(fnstack)

#define MODULE_C_FRAMEEXIT()                                     \
    PALLENE_TRACER_FRAMEEXIT(fnstack)

/* ---------------- FOR C INTERFACE FUNCTIONS END ---------------- */

void trigger_pallene_stack_overflow(lua_State *L, int count) {
    MODULE_C_FRAMEENTER();

    /* We are not supposed to use this macro. */
    /* It is used so that we can bypass compiler 
       warnings for infinite recursion as we are 
       deliberately triggering the callstack error. */
    if(count < PALLENE_TRACER_MAX_CALLSTACK) {
        MODULE_C_SETLINE();
        trigger_pallene_stack_overflow(L, count + 1);
    }

    MODULE_C_FRAMEEXIT();
}

void module_fn(lua_State *L) {
    MODULE_C_FRAMEENTER();

    /* Set line number to current active frame in the Pallene callstack and
       call the function which is already in the Lua stack. */
    MODULE_C_SETLINE();
    trigger_pallene_stack_overflow(L, 0);

    MODULE_C_FRAMEEXIT();
}

int module_fn_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(module_fn_lua);

    /* Dispatch */
    module_fn(L);

    return 0;
}

int luaopen_module(lua_State *L) {
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);

    lua_newtable(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- module_fn ---- */
    lua_pushlightuserdata(L, fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, module_fn_lua, 2);
    lua_setfield(L, -2, "module_fn");

    return 1;
}
