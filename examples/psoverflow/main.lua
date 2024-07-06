-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the BSD-3-Clause license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: BSD-3-Clause

local module = require "examples.psoverflow.module"

-- luacheck: globals wrapper
function wrapper()
    module.module_fn()
end

-- luacheck: globals pallene_tracer_debug_traceback
xpcall(wrapper, pallene_tracer_debug_traceback)
