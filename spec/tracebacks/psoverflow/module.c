/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* Static use of the library would suffice. */
#define PT_IMPLEMENTATION
#include <ptracer.h>

/* ---------------- FOR LUA INTERFACE FUNCTIONS ---------------- */
/* The finalizer fn will run whenever out of scope. */
#define PREPARE_FINALIZER()                             \
    int _base = lua_gettop(L);                          \
    lua_pushvalue(L, lua_upvalueindex(2));              \
    lua_toclose(L, -1)

#define MODULE_LUA_FRAMEENTER(fnptr)                    \
    pt_fnstack_t *fnstack = lua_touserdata(L,           \
        lua_upvalueindex(1));                           \
    pt_frame_t _frame =                                 \
        PALLENE_TRACER_LUA_FRAME(fnptr);                \
    pallene_tracer_frameenter(L, fnstack, &_frame);     \
    PREPARE_FINALIZER()

#define MODULE_LUA_FRAMEEXIT()                          \
    lua_settop(L, _base)

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */
#define MODULE_C_FRAMEENTER()                           \
    static pt_fn_details_t _details =                   \
        PALLENE_TRACER_FN_DETAILS(__func__, __FILE__);  \
    pt_frame_t _frame =                                 \
        PALLENE_TRACER_C_FRAME(_details);               \
    pallene_tracer_frameenter(L, fnstack, &_frame)

#define MODULE_SETLINE()                                \
    pallene_tracer_setline(fnstack, __LINE__ + 1)

#define MODULE_C_FRAMEEXIT()                            \
    pallene_tracer_frameexit(fnstack)

void trigger_pallene_stack_overflow(lua_State *L, pt_fnstack_t *fnstack, int count) {
    MODULE_C_FRAMEENTER();

    /* We are not supposed to use this macro. */
    /* It is used so that we can bypass compiler 
       warnings for infinite recursion as we are 
       deliberately triggering the callstack error. */
    if(count < PALLENE_TRACER_MAX_CALLSTACK) {
        MODULE_SETLINE();
        trigger_pallene_stack_overflow(L, fnstack, count + 1);
    }

    MODULE_C_FRAMEEXIT();
}

void module_fn(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    /* Set line number to current active frame in the Pallene callstack and
       call the function which is already in the Lua stack. */
    MODULE_SETLINE();
    trigger_pallene_stack_overflow(L, fnstack, 0);

    MODULE_C_FRAMEEXIT();
}

int module_fn_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(module_fn_lua);

    /* Dispatch */
    module_fn(L, fnstack);

    MODULE_LUA_FRAMEEXIT();
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