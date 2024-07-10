/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* This time we would be doing dynamic linking. */
#include <ptracer.h>

/* ---------------- FOR LUA INTERFACE FUNCTIONS ---------------- */
/* The finalizer fn will run whenever out of scope. */
#define PREPARE_FINALIZER()                             \
    int _base = lua_gettop(L);                          \
    lua_pushvalue(L, lua_upvalueindex(2));              \
    lua_toclose(L, -1)
#define MODULE_LUA_FRAMEENTER(sig)                      \
    pt_fnstack_t *fnstack = lua_touserdata(L,           \
        lua_upvalueindex(1));                           \
    pt_frame_t _frame = {                               \
        .type = PALLENE_TRACER_FRAME_TYPE_LUA,          \
        .shared = { .c_fnptr = sig }                    \
    };                                                  \
    pallene_tracer_frameenter(L, fnstack, &_frame);     \
    PREPARE_FINALIZER()
#define MODULE_LUA_FRAMEEXIT()                          \
    lua_settop(L, _base)

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */
#define MODULE_C_FRAMEENTER()                           \
    static pt_fn_details_t _details = {                 \
        .fn_name = __func__,                            \
        .filename = __FILE__                            \
    };                                                  \
    pt_frame_t _frame = {                               \
        .type = PALLENE_TRACER_FRAME_TYPE_C,            \
        .shared = { .details = &_details }              \
    };                                                  \
    pallene_tracer_frameenter(L, fnstack, &_frame)
#define MODULE_SETLINE()                                \
    pallene_tracer_setline(fnstack, __LINE__ + 1)
#define MODULE_C_FRAMEEXIT()                            \
    pallene_tracer_frameexit(fnstack)

void another_mod_fn(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    // Other code...

    MODULE_SETLINE();
    luaL_error(L, "Error from another module!");

    // Other code...

    MODULE_C_FRAMEEXIT();
}

int another_mod_fn_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(another_mod_fn_lua);

    /* Dispatch. */
    another_mod_fn(L, fnstack);

    MODULE_LUA_FRAMEEXIT();
    return 0;
}

int luaopen_examples_multimod_module_b(lua_State *L) {
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);

    lua_newtable(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- singular_fn_1 ---- */
    lua_pushlightuserdata(L, (void *) fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, another_mod_fn_lua, 2);
    lua_setfield(L, -2, "another_mod_fn");

    return 1;
}
