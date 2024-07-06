-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the BSD-3-Clause license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: BSD-3-Clause

local util = require "misc.util"

local function assert_example(example, expected_content)
    local makefile = "examples/"..example.."/Makefile";
    local ok, err = util.execute("make -f "..makefile)
    assert(ok, err)

    local luafile = util.shell_quote("examples/"..example.."/main.lua")
    local ok, err, _, err_content = util.outputs_of_execute("lua "..luafile)
    assert(ok, err)
    assert.are.same(expected_content, err_content)
end

it("Dispatch", function()
    assert_example("dispatch", [[
Runtime error: examples/dispatch/main.lua:10: Error from an untracked C function, which has no trace in Lua callstack!
Stack traceback:
    examples/dispatch/module.c:51: in function 'some_untracked_c_function'
    examples/dispatch/module.c:94: in function 'module_fn_2'
    examples/dispatch/main.lua:10: in function 'lua_callee_1'
    examples/dispatch/module.c:64: in function 'module_fn_1'
    examples/dispatch/main.lua:15: in function 'wrapper'
    C: in function 'xpcall'
    examples/dispatch/main.lua:19: in <main>
    C: in function '<?>'
]])
end)
