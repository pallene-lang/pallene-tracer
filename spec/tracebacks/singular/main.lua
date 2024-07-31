-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

local module = require "module"

function some_lua_fn()
    module.singular_fn_1()
end

function wrapper()
    some_lua_fn()
end

xpcall(wrapper, pallene_tracer_debug_traceback)
