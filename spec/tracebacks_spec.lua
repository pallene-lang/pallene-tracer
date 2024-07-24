-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

local util = require "misc.util"

local function assert_example(example, expected_content)
    local cdir = "spec/tracebacks/"..example.."/";
    local ok, err = util.execute("cd "..cdir.."&& make")
    assert(ok, err)

    local ok, err, _, err_content = util.outputs_of_execute("cd "..cdir.." && lua main.lua")
    assert(ok, err)
    assert.are.same(expected_content, err_content)
end

it("Dispatch", function()
    assert_example("dispatch", [[
Runtime error: main.lua:10: Error from an untracked C function, which has no trace in Lua callstack!
Stack traceback:
    module.c:48: in function 'some_untracked_c_function'
    module.c:92: in function 'module_fn_2'
    main.lua:10: in function 'lua_callee_1'
    module.c:61: in function 'module_fn_1'
    main.lua:15: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:19: in <main>
    C: in function '<?>'
]])
end)

it("Singular", function()
    assert_example("singular", [[
Runtime error: main.lua:10: Life's !good
Stack traceback:
    module.c:51: in function 'lifes_good_fn'
    module.c:61: in function 'singular_fn_1'
    main.lua:10: in function 'some_lua_fn'
    main.lua:15: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:19: in <main>
    C: in function '<?>'
]])
end)


it("Multi-module", function()
    assert_example("multimod", [[
Runtime error: main.lua:11: Error from another module!
Stack traceback:
    module_b.c:49: in function 'another_mod_fn'
    main.lua:11: in function 'some_lua_fn'
    module_a.c:49: in function 'some_mod_fn'
    main.lua:16: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:20: in <main>
    C: in function '<?>'
]])
end)

it("Pallene Stack Overflow", function()
    assert_example("psoverflow", [[
Runtime error: main.lua:10: pallene callstack overflow
Stack traceback:
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'

    ... (Skipped 99983 frames) ...

    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:53: in function 'trigger_pallene_stack_overflow'
    module.c:65: in function 'module_fn'
    main.lua:10: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:14: in <main>
    C: in function '<?>'
]])
end)

it("Depth recursion", function()
    assert_example("depth_recursion", [[
Runtime error: main.lua:11: Depth reached 0!
Stack traceback:
    C: in function 'error'
    main.lua:11: in function 'lua_fn'
    module.c:63: in function 'module_fn'
    main.lua:14: in function 'lua_fn'
    module.c:63: in function 'module_fn'
    main.lua:14: in function 'lua_fn'
    module.c:63: in function 'module_fn'
    main.lua:14: in function 'lua_fn'
    module.c:63: in function 'module_fn'
    main.lua:14: in function 'lua_fn'
    module.c:63: in function 'module_fn'
    main.lua:14: in function 'lua_fn'
    main.lua:21: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:25: in <main>
    C: in function '<?>'
]])
end)

it("Anonymous Lua Fn", function()
    assert_example("anon_lua", [[
Runtime error: main.lua:9: Error from an untracked C function, which has no trace in Lua callstack!
Stack traceback:
    module.c:48: in function 'some_untracked_c_function'
    module.c:92: in function 'module_fn_2'
    main.lua:9: in function '<?>'
    module.c:61: in function 'module_fn_1'
    main.lua:13: in function '<?>'
    C: in function 'xpcall'
    main.lua:17: in <main>
    C: in function '<?>'
]])
end)