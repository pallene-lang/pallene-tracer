#ifndef MODULE_INCLUDE_HEADER
#define MODULE_INCLUDE_HEADER

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

#endif // MODULE_INCLUDE_HEADER
