/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the BSD-3-Clause license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: BSD-3-Clause 
 */

/* Static use of the library would suffice. */
#define PT_IMPLEMENTATION
#include <ptracer.h>

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */
#define MODULE_C_FRAMEENTER()                           \
    static pt_fn_details_t _details = {                 \
        .fn_name = __FUNCTION__,                        \
        .filename = __FILE__                            \
    };                                                  \
    pt_frame_t _frame_c = {                             \
        .type = PALLENE_TRACER_FRAME_TYPE_C,            \
        .shared = { .details = &_details }              \
    };                                                  \
    pallene_tracer_frameenter(L, cont, &_frame_c)
#define MODULE_SETLINE()                                \
    pallene_tracer_setline(cont, __LINE__ + 1)
#define MODULE_C_FRAMEEXIT()                            \
    pallene_tracer_frameexit(cont)

/* ---------------- FOR LUA INTERFACE FUNCTIONS ---------------- */
#define PREPARE_FINALIZER()                             \
    int _base = lua_gettop(L);                          \
    lua_pushvalue(L, lua_upvalueindex(2));              \
    lua_toclose(L, -1); /* The finalizer fn will run whenever out of scope. */
#define MODULE_LUA_FRAMEENTER(sig)                      \
    pt_cont_t *cont = (pt_cont_t *)                     \
        lua_touserdata(L, lua_upvalueindex(1));         \
    pt_frame_t _frame_lua = {                           \
        .type = PALLENE_TRACER_FRAME_TYPE_LUA,          \
        .shared = { .frame_sig = sig }                  \
    };                                                  \
    pallene_tracer_frameenter(L, cont, &_frame_lua);    \
    MODULE_C_FRAMEENTER();                              \
    PREPARE_FINALIZER()
/* The finalizer will get rid of all the C interface frames
   as well. */
#define MODULE_LUA_FRAMEEXIT()                          \
    lua_settop(L, _base)

void lifes_good_fn(lua_State *L, pt_cont_t *cont) {
    MODULE_C_FRAMEENTER();

    MODULE_SETLINE();
    luaL_error(L, "Life's !good");

    MODULE_C_FRAMEEXIT();
}

int singular_fn_1(lua_State *L) {
    MODULE_LUA_FRAMEENTER(singular_fn_1);

    /* Call some C function. */
    MODULE_SETLINE();
    lifes_good_fn(L, cont);

    MODULE_LUA_FRAMEEXIT();
    return 0;
}

int luaopen_examples_singular_module(lua_State *L) {
    /* Our stack. */
    pt_cont_t *cont = pallene_tracer_init(L);

    lua_newtable(L);
    int table = lua_gettop(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- singular_fn_1 ---- */
    lua_pushlightuserdata(L, (void *) cont);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, singular_fn_1, 2);
    lua_setfield(L, -2, "singular_fn_1");

    return 1;
}
