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

void some_untracked_c_function(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    MODULE_SETLINE();
    luaL_error(L, "Error from an untracked C function, which has no trace in Lua callstack!");

    MODULE_C_FRAMEEXIT();
}

void module_fn_1(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    // Other code...

    /* Set line number to current active frame in the Pallene callstack and
       call the function which is already in the Lua stack. */
    MODULE_SETLINE();
    lua_call(L, 0, 0);

    // Other code...

    MODULE_C_FRAMEEXIT();
}

int module_fn_1_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(module_fn_1_lua);

    /* Peep at the macro defintion for `_base`. */
    if(luai_unlikely(_base < 1)) 
        luaL_error(L, "Expected atleast 1 parameters");

    lua_pushvalue(L, 1);
    if(luai_unlikely(lua_isfunction(L, -1) == 0)) 
        luaL_error(L, "Expected parameter 1 to be a function");

    /* Now dispatch to an actual C function. */
    module_fn_1(L, fnstack);

    MODULE_LUA_FRAMEEXIT();
    return 0;
}

void module_fn_2(lua_State *L, pt_fnstack_t *fnstack) {
    MODULE_C_FRAMEENTER();

    // Other code...

    MODULE_SETLINE();
    some_untracked_c_function(L, fnstack);

    // Other code...

    MODULE_C_FRAMEEXIT();
}

int module_fn_2_lua(lua_State *L) {
    MODULE_LUA_FRAMEENTER(module_fn_2_lua);

    /* Dispatch. */
    module_fn_2(L, fnstack);

    MODULE_LUA_FRAMEEXIT();
    return 0;
}

int luaopen_module(lua_State *L) {
    /* Our stack. */
    pt_fnstack_t *fnstack = pallene_tracer_init(L);

    lua_newtable(L);
    int table = lua_gettop(L);

    /* One very good way to integrate our stack userdatum and finalizer
      object is by using Lua upvalues. */
    /* ---- module_fn_1 ---- */
    lua_pushlightuserdata(L, fnstack);
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, module_fn_1_lua, 2);
    lua_setfield(L, table, "module_fn_1");

    /* ---- module_fn_2 ---- */
    lua_pushlightuserdata(L, fnstack);
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, module_fn_2_lua, 2);
    lua_setfield(L, table, "module_fn_2");

    return 1;
}