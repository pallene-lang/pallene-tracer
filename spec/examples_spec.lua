-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

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
    examples/dispatch/module.c:49: in function 'some_untracked_c_function'
    examples/dispatch/module.c:93: in function 'module_fn_2'
    examples/dispatch/main.lua:10: in function 'lua_callee_1'
    examples/dispatch/module.c:62: in function 'module_fn_1'
    examples/dispatch/main.lua:15: in function 'wrapper'
    C: in function 'xpcall'
    examples/dispatch/main.lua:19: in <main>
    C: in function '<?>'
]])
end)

it("Singular", function()
    assert_example("singular", [[
Runtime error: examples/singular/main.lua:10: Life's !good
Stack traceback:
    examples/singular/module.c:52: in function 'lifes_good_fn'
    examples/singular/module.c:62: in function 'singular_fn_1'
    examples/singular/main.lua:10: in function 'some_lua_fn'
    examples/singular/main.lua:15: in function 'wrapper'
    C: in function 'xpcall'
    examples/singular/main.lua:19: in <main>
    C: in function '<?>'
]])
end)


it("Multi-module", function()
    assert_example("multimod", [[
Runtime error: examples/multimod/main.lua:11: Error from another module!
Stack traceback:
    examples/multimod/module_b.c:50: in function 'another_mod_fn'
    examples/multimod/main.lua:11: in function 'some_lua_fn'
    examples/multimod/module_a.c:50: in function 'some_mod_fn'
    examples/multimod/main.lua:16: in function 'wrapper'
    C: in function 'xpcall'
    examples/multimod/main.lua:20: in <main>
    C: in function '<?>'
]])
end)

it("Pallene Stack Overflow", function()
    assert_example("psoverflow", [[
Runtime error: examples/psoverflow/main.lua:10: pallene callstack overflow
Stack traceback:
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'

    ... (Skipped 99983 frames) ...

    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:54: in function 'trigger_pallene_stack_overflow'
    examples/psoverflow/module.c:66: in function 'module_fn'
    examples/psoverflow/main.lua:10: in function 'wrapper'
    C: in function 'xpcall'
    examples/psoverflow/main.lua:14: in <main>
    C: in function '<?>'
]])
end)

it("Depth recursion", function()
    assert_example("depth_recursion", [[
Runtime error: examples/depth_recursion/main.lua:11: Depth reached 0!
Stack traceback:
    C: in function 'error'
    examples/depth_recursion/main.lua:11: in function 'lua_fn'
    examples/depth_recursion/module.c:64: in function 'module_fn'
    examples/depth_recursion/main.lua:14: in function 'lua_fn'
    examples/depth_recursion/module.c:64: in function 'module_fn'
    examples/depth_recursion/main.lua:14: in function 'lua_fn'
    examples/depth_recursion/module.c:64: in function 'module_fn'
    examples/depth_recursion/main.lua:14: in function 'lua_fn'
    examples/depth_recursion/module.c:64: in function 'module_fn'
    examples/depth_recursion/main.lua:14: in function 'lua_fn'
    examples/depth_recursion/module.c:64: in function 'module_fn'
    examples/depth_recursion/main.lua:14: in function 'lua_fn'
    examples/depth_recursion/main.lua:21: in function 'wrapper'
    C: in function 'xpcall'
    examples/depth_recursion/main.lua:25: in <main>
    C: in function '<?>'
]])
end)

it("Anonymous Lua Fn", function()
    assert_example("anon_lua", [[
Runtime error: examples/anon_lua/main.lua:9: Error from an untracked C function, which has no trace in Lua callstack!
Stack traceback:
    examples/anon_lua/module.c:49: in function 'some_untracked_c_function'
    examples/anon_lua/module.c:93: in function 'module_fn_2'
    examples/anon_lua/main.lua:9: in function '<?>'
    examples/anon_lua/module.c:62: in function 'module_fn_1'
    examples/anon_lua/main.lua:13: in function '<?>'
    C: in function 'xpcall'
    examples/anon_lua/main.lua:17: in <main>
    C: in function '<?>'
]])
end)
