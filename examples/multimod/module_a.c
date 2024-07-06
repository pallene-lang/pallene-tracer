/* 
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT 
 */

/* This time we would be doing dynamic linking. */
#include <ptracer.h>

/* ---------------- FOR LUA INTERFACE FUNCTIONS ---------------- */
#define PREPARE_FINALIZER()                             \
    int _base = lua_gettop(L);                          \
    lua_pushvalue(L, lua_upvalueindex(2));              \
    lua_toclose(L, -1); /* The finalizer fn will run whenever out of scope. */
#define MODULE_LUA_FRAMEENTER(sig)                      \
    pt_cont_t *cont = (pt_cont_t *)                     \
        lua_touserdata(L, lua_upvalueindex(1));         \
    pt_frame_t _frame = {                               \
        .type = PALLENE_TRACER_FRAME_TYPE_LUA,          \
        .shared = { .frame_sig = sig }                  \
    };                                                  \
    pallene_tracer_frameenter(L, cont, &_frame);        \
    PREPARE_FINALIZER()
#define MODULE_LUA_FRAMEEXIT()                          \
    lua_settop(L, _base)

/* ---------------- FOR C INTERFACE FUNCTIONS ---------------- */
#define MODULE_C_FRAMEENTER()                           \
    static pt_fn_details_t _details = {                 \
        .fn_name = __FUNCTION__,                        \
        .filename = __FILE__                            \
    };                                                  \
    pt_frame_t _frame = {                               \
        .type = PALLENE_TRACER_FRAME_TYPE_C,            \
        .shared = { .details = &_details }              \
    };                                                  \
    pallene_tracer_frameenter(L, cont, &_frame)
#define MODULE_SETLINE()                                \
    pallene_tracer_setline(cont, __LINE__ + 1)
#define MODULE_C_FRAMEEXIT()                            \
    pallene_tracer_frameexit(cont)

void some_mod_fn(lua_State *L, pt_cont_t *cont) {
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
    some_mod_fn(L, cont);

    MODULE_LUA_FRAMEEXIT();
    return 0;
}

int luaopen_examples_multimod_module_a(lua_State *L) {
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
    lua_pushcclosure(L, some_mod_fn_lua, 2);
    lua_setfield(L, -2, "some_mod_fn");

    return 1;
}

