/*
 * Copyright (c) 2024, The Pallene Developers
 * Pallene Tracer is licensed under the MIT license.
 * Please refer to the LICENSE and AUTHORS files for details
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define PT_IMPLEMENTATION
#include "ptracer.h"

/* Traceback ellipsis top threshold. How many frames should we print
   first to trigger ellipsis? */
#ifndef PT_RUN_TRACEBACK_TOP_THRESHOLD
#define PT_RUN_TRACEBACK_TOP_THRESHOLD           10
#endif // PT_RUN_TRACEBACK_TOP_THRESHOLD

/* This should always be 2 fewer than top threshold, for symmetry.
   Becuase we will always have 2 tail frames lingering around at
   at the end which is not captured by '_countlevels'. Lua also
   do it like this. */
#ifndef PT_RUN_TRACEBACK_BOTTOM_THRESHOLD
#define PT_RUN_TRACEBACK_BOTTOM_THRESHOLD        8
#endif // PT_RUN_TRACEBACK_BOTTOM_THRESHOLD

static void pt_run_usage(FILE *where) {
    fprintf(where, "Usage: pt-run [-h | --help] <lua_script> [<args>] ...\n\n");
}

static void pt_run_help() {
    pt_run_usage(stdout);
    printf("Pallene Tracer runner for call-stack backtrace\n\n");
    printf("Arguments:\n"
           "    lua_script                Lua file to debug\n"
           "    args ...                  Arguments passed to Lua script\n\n");
    printf("Options:\n"
           "    -h, --help                Show this help message and exit\n");
}

/* Global table name deduction. Can we find a function name? */
static bool pt_run_findfield(lua_State *L, int fn_idx, int level) {
    if(level == 0 || !lua_istable(L, -1))
        return false;

    lua_pushnil(L);    /* Initial key. */

    while(lua_next(L, -2)) {
        /* We are only interested in String keys. */
        if(lua_type(L, -2) == LUA_TSTRING) {
            /* Avoid "_G" recursion in global table. The global table is also part of
               global table :). */
            if(!strcmp(lua_tostring(L, -2), "_G")) {
                /* Remove value and continue. */
                lua_pop(L, 1);
                continue;
            }

            /* Is it the function we are looking for? */
            if(lua_rawequal(L, fn_idx, -1)) {
                /* Remove value and keep name. */
                lua_pop(L, 1);
                return true;
            }
            /* If not go one level deeper and get the value recursively. */
            else if(pt_run_findfield(L, fn_idx, level - 1)) {
                /* Remove the table but keep name. */
                lua_remove(L, -2);

                /* Add a "." in between. */
                lua_pushliteral(L, ".");
                lua_insert(L, -2);

                /* Concatenate last 3 values, resulting "table.some_func". */
                lua_concat(L, 3);

                return true;
            }
        }

        /* Pop the value. */
        lua_pop(L, 1);
    }

    return false;
}

/* Pushes a function name if found in the global table and returns true.
   Returns false otherwise. */
/* Expects the function to be pushed in the stack. */
static bool pt_run_pgf_name(lua_State *L) {
    int top = lua_gettop(L);

    /* Start from the global table. */
    lua_pushglobaltable(L);

    if(pt_run_findfield(L, top, 2)) {
        lua_remove(L, -2);
        return true;
    }

    lua_pop(L, 1);
    return false;
}

/* Returns the maximum number of levels in Lua stack. */
static int pt_run_countlevels(lua_State *L) {
    lua_Debug ar;
    int li = 1, le = 1;

    /* Find an upper bound */
    while (lua_getstack(L, le, &ar)) {
        li = le, le *= 2;
    }

    /* Do a binary search */
    while (li < le) {
        int m = (li + le) / 2;

        if (lua_getstack(L, m, &ar)) li = m + 1;
        else le = m;
    }

    return le - 1;
}

/* Counts the number of white and black frames in the Pallene call stack. */
static void pt_run_countframes(pt_fnstack_t *fnstack, int *mwhite, int *mblack) {
    *mwhite = *mblack = 0;

    for(int i = 0; i < fnstack->count; i++) {
        *mwhite += (fnstack->stack[i].type == PALLENE_TRACER_FRAME_TYPE_C);
        *mblack += (fnstack->stack[i].type == PALLENE_TRACER_FRAME_TYPE_LUA);
    }
}

/* Responsible for printing and controlling some of the traceback fn parameters. */
static void pt_run_dbg_print(const char *buf, bool *ellipsis, int *pframes, int nframes) {
    /* We have printed the frame, even tho it might not be visible ;). */
    (*pframes)++;

    /* Should we print? Are we at any point in top or bottom printing threshold? */
    bool should_print = (*pframes <= PT_RUN_TRACEBACK_TOP_THRESHOLD)
        || ((nframes - *pframes) <= PT_RUN_TRACEBACK_BOTTOM_THRESHOLD);

    if(luai_likely(should_print))
        fprintf(stderr, "%s", buf);
    else if(*ellipsis) {
        fprintf(stderr, "\n    ... (Skipped %d frames) ...\n\n",
            nframes - (PT_RUN_TRACEBACK_TOP_THRESHOLD
            + PT_RUN_TRACEBACK_BOTTOM_THRESHOLD));

        *ellipsis = false;
    }
}

#define DBG_PRINT() pt_run_dbg_print(buf, &ellipsis, &pframes, nframes)
/* Pallene Tracer explicit traceback function to show Pallene call-stack
   tracebacks. */
int pt_run_debug_traceback(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, PALLENE_TRACER_CONTAINER_ENTRY);
    pt_fnstack_t *fnstack = (pt_fnstack_t *) lua_touserdata(L, -1);
    pt_frame_t *stack = fnstack->stack;
    /* The point where we are in the Pallene stack. */
    int index = fnstack->count - 1;
    lua_pop(L, 1);

    /* Max number of white and black frames. */
    int mwhite, mblack;
    pt_run_countframes(fnstack, &mwhite, &mblack);
    /* Max levels of Lua stack. */
    int mlevel = pt_run_countlevels(L);

    /* Total frames we are going to print. */
    /* Black frames are used for switching and we will start from
       Lua stack level 1. */
    int nframes = mlevel + mwhite - mblack - 1;
    /* Amount of frames printed. */
    int pframes = 0;
    /* Should we print ellipsis? */
    bool ellipsis = nframes > (PT_RUN_TRACEBACK_TOP_THRESHOLD
        + PT_RUN_TRACEBACK_BOTTOM_THRESHOLD);

    /* Buffer to store for a single frame line to be printed. */
    int _BUFSIZ = 1023;
    char buf[_BUFSIZ + 1];

    const char *message = lua_tostring(L, 1);
    fprintf(stderr, "Runtime error: %s\nStack traceback:\n", message);

    lua_Debug ar;
    int top = lua_gettop(L);
    int level = 1;

    while(lua_getstack(L, level++, &ar)) {
        /* Get information regarding the frame: name, source, linenumbers etc. */
        lua_getinfo(L, "Slnf", &ar);

        /* If the frame is a C frame. */
        if(lua_iscfunction(L, -1)) {
            if(index >= 0) {
                /* Check whether this frame is tracked (C interface frames). */
                int check = index;
                while(stack[check].type != PALLENE_TRACER_FRAME_TYPE_LUA)
                    check--;

                /* If the frame matches, we switch to printing Pallene frames. */
                if(lua_tocfunction(L, -1) == stack[check].shared.c_fnptr) {
                    /* Now print all the frames in Pallene stack. */
                    for(; index > check; index--) {
                        snprintf(buf, _BUFSIZ, "    %s:%d: in function '%s'\n",
                            stack[index].shared.details->filename,
                            stack[index].line, stack[index].shared.details->fn_name);
                        DBG_PRINT();
                    }

                    /* 'check' idx is guaranteed to be a Lua interface frame.
                       Which is basically our 'stack' index at this point. So,
                       we simply ignore the Lua interface frame. */
                    index--;

                    /* We are done. */
                    lua_settop(L, top);
                    continue;
                }
            }

            /* Then it's an untracked C frame. */
            if(pt_run_pgf_name(L))
                lua_pushfstring(L, "%s", lua_tostring(L, -1));
            else lua_pushliteral(L, "<?>");

            snprintf(buf, _BUFSIZ, "    C: in function '%s'\n", lua_tostring(L, -1));
            DBG_PRINT();
        } else {
            /* It's a Lua frame. */

            /* Do we have a name? */
            if(*ar.namewhat != '\0')
                lua_pushfstring(L, "function '%s'", ar.name);
            /* Is it the main chunk? */
            else if(*ar.what == 'm')
                lua_pushliteral(L, "<main>");
            /* Can we deduce the name from the global table? */
            else if(pt_run_pgf_name(L))
                lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
            else lua_pushliteral(L, "function '<?>'");

            snprintf(buf, _BUFSIZ, "    %s:%d: in %s\n", ar.short_src,
                ar.currentline, lua_tostring(L, -1));
            DBG_PRINT();
        }

        lua_settop(L, top);
    }

    return 0;
}
#undef DBG_PRINT

int main(int argc, char **argv) {
    int res = EXIT_SUCCESS;

    /* If no argument was passed. */
    if(argc < 2) {
        pt_run_usage(stderr);
        fprintf(stderr, "pt-run: missing argument <lua_script>, abort...\n");

        res = EXIT_FAILURE;
        goto out;
    }

    /* Option parsing. */
    if(!strncmp(argv[1], "-", 1)) {
        char *match = argv[1] + 1;

        if(!strcmp(match, "h") || !strcmp(match, "-help"))
            pt_run_help();
        else {
            pt_run_usage(stderr);
            fprintf(stderr, "pt-run: unknown option '%s', abort...\n", argv[1]);
        }

        res = EXIT_FAILURE;
        goto out;
    }

    /* Initalize Lua. */
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    /* Initialize Pallene tracer. */
    (void) pallene_tracer_init(L); /* We are not doing anything with the Userdata. */
    /* The init function pushes the finalizer object onto the Lua stack. Which
       we also do not need. */
    lua_pop(L, 1);

    /* The traceback function. */
    lua_pushcfunction(L, pt_run_debug_traceback);
    int traceback_fn = lua_gettop(L);

    /* Load the Lua file we wish to debug. */
    int status = luaL_loadfile(L, argv[1]);
    if(status != LUA_OK)  {
        fprintf(stderr, "pt-run: %s\n", lua_tostring(L, -1));

        res = EXIT_FAILURE;
        goto finalize;
    }

    /* Push arguments to Lua stack. */
    for(int i = 2; i < argc; i++)
        lua_pushstring(L, argv[i]);

    /* Moment of truth. */
    status = lua_pcall(L, argc - 2, LUA_MULTRET, traceback_fn);
    if(status != LUA_OK)
        res = EXIT_FAILURE;

finalize:
    /* Cleanup. */
    lua_close(L);
out:
    return res;
}
