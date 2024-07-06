-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

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
