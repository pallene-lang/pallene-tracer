-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the BSD-3-Clause license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: BSD-3-Clause

local mod_a = require "examples.multimod.module_a"
local mod_b = require "examples.multimod.module_b"

-- luacheck: globals some_lua_fn
function some_lua_fn()
    mod_b.another_mod_fn()
end

-- luacheck: globals wrapper
function wrapper()
    mod_a.some_mod_fn(some_lua_fn)
end

-- luacheck: globals pallene_tracer_debug_traceback
xpcall(wrapper, pallene_tracer_debug_traceback)
