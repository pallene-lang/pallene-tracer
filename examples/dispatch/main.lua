-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the BSD-3-Clause license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: BSD-3-Clause

local module = require "examples.dispatch.module"

-- luacheck: globals lua_callee_1 module_fn_2
function lua_callee_1()
    module.module_fn_2()
end

-- luacheck: globals wrapper module_fn_1
function wrapper()
    module.module_fn_1(lua_callee_1)
end

-- luacheck: globals pallene_tracer_debug_traceback
xpcall(wrapper, pallene_tracer_debug_traceback)
