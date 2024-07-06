-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

local module = require "examples.depth_recursion.module"

-- luacheck: globals lua_fn
function lua_fn(depth)
    if depth == 0 then
        error "Depth reached 0!"
    end

    module.module_fn(lua_fn, depth - 1)
end

-- Should be local.
-- Making it global so that it is visible in the traceback.
-- luacheck: globals wrapper
function wrapper()
    lua_fn(10)
end

-- luacheck: globals pallene_tracer_debug_traceback
xpcall(wrapper, pallene_tracer_debug_traceback)
