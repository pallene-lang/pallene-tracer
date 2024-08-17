/*
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>

#define PT_IMPLEMENTATION
#include "ptracer.h"

/* ---------------- PALLENE TRACER LUA INERFACE ---------------- */
#define CON_LUA_FRAMEENTER(fnptr)                       \
    PALLENE_TRACER_LUA_FRAMEENTER(L, fnstack, fnptr,    \
        lua_upvalueindex(1), _frame)
/* ---------------- PALLENE TRACER LUA INERFACE END ---------------- */

/* ---------------- PALLENE TRACER C INTERFACE ---------------- */

#define CON_C_FRAMEENTER()                              \
    PALLENE_TRACER_GENERIC_C_FRAMEENTER(fnstack, _frame)

#define CON_C_SETLINE()                                 \
    PALLENE_TRACER_GENERIC_C_SETLINE(fnstack)

#define CON_C_FRAMEEXIT()                               \
    PALLENE_TRACER_FRAMEEXIT(fnstack)

/* ---------------- PALLENE TRACER C INTERFACE END ---------------- */

/* This is one way to work with Pallene Tracer call-stack, but certainly not recommended because
   this way you loose the flexibility of using multiple Lua states. */
pt_fnstack_t *fnstack;

static void internal_dfs(lua_State *L, int *visited, int node) {
    CON_C_FRAMEENTER();

    if(visited[node]) {
        CON_C_FRAMEEXIT();
        return;
    }

    visited[node] = 1;

    lua_rawgeti(L, 1, node);  /* get the connected adjacent nodes of current node */
    CON_C_SETLINE();
    int n = luaL_len(L, -1);  /* this line may trigger error */
    lua_pop(L, 1);

    /* for all the adjacent nodes */
    for(int i = 1; i <= n; i++) {
        lua_rawgeti(L, 1, node);
        lua_rawgeti(L, -1, i);
        int adjacent_node = lua_tointeger(L, -1);
        lua_pop(L, 2); /* avoid Lua value-stack overflow by not keeping values on the stack. */

        CON_C_SETLINE();
        internal_dfs(L, visited, adjacent_node);
    }

    CON_C_FRAMEEXIT();
}

static int find_connected_components(lua_State *L, int *visited, int n) {
    CON_C_FRAMEENTER();

    int result = 0;

    for(int i = 1; i <= n; i++) {
        if(!visited[i]) {
            result++;
            CON_C_SETLINE();
            internal_dfs(L, visited, i);
        }
    }

    CON_C_FRAMEEXIT();
    return result;
}

static int find_connected_components_lua(lua_State *L) {
    CON_LUA_FRAMEENTER(find_connected_components_lua);

    if(!lua_istable(L, 1))
        luaL_error(L, "expected the first argument to be table");
    if(!lua_isinteger(L, 2))
        luaL_error(L, "expected the second argument to be integer");

    int nodes = lua_tointeger(L, 2);  /* get total number of nodes */
    int *visited = calloc(nodes + 1, sizeof(int));  /* Lua prefers 1 based indexing */

    /* Dispatch and push the result. */
    lua_pushinteger(L, find_connected_components(L, visited, nodes));

    free(visited);
    return 1;
}

int luaopen_component(lua_State *L) {
    fnstack = pallene_tracer_init(L);

    lua_newtable(L);
    int table = lua_gettop(L);

    /* ---- find_connected_components ---- */
    /* `pallene_tracer_init` function pushes the frameexit finalizer to the stack. */
    lua_pushvalue(L, -2);  /* passing the finalizer object as upvalue is generally the way to go. */
    lua_pushcclosure(L, find_connected_components_lua, 1);
    lua_setfield(L, table, "find_connected_components");

    return 1;
}
