-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

local util = require "spec.util"

local function assert_test(example, expected_content)
    assert(util.execute("make --quiet tests"))

    local cdir  = util.shell_quote("spec/tracebacks/"..example)
    local ptrun = util.shell_quote("../../../src/pt-run")
    local ok, _, output_content, err_content =
        util.outputs_of_execute(string.format("cd %s && %s main.lua", cdir, ptrun))
    assert(not ok, output_content)
    assert.are.same(expected_content, err_content)
end

it("Dispatch", function()
    assert_test("dispatch", [[
Runtime error: main.lua:9: Error from a C function, which has no trace in Lua callstack!
Stack traceback:
    spec/tracebacks/dispatch/module.c:48: in function 'some_oblivious_c_function'
    spec/tracebacks/dispatch/module.c:92: in function 'module_fn_2'
    main.lua:9: in function 'lua_callee_1'
    spec/tracebacks/dispatch/module.c:61: in function 'module_fn_1'
    main.lua:12: in <main>
]])
end)

it("Singular", function()
    assert_test("singular", [[
Runtime error: main.lua:9: Life's !good
Stack traceback:
    spec/tracebacks/singular/module.c:49: in function 'lifes_good_fn'
    spec/tracebacks/singular/module.c:59: in function 'singular_fn'
    main.lua:9: in function 'some_lua_fn'
    main.lua:12: in <main>
]])
end)


it("Multi-module", function()
    assert_test("multimod", [[
Runtime error: main.lua:10: Error from another module!
Stack traceback:
    spec/tracebacks/multimod/module_b.c:19: in function 'another_mod_fn'
    main.lua:10: in function 'some_lua_fn'
    spec/tracebacks/multimod/module_a.c:20: in function 'some_mod_fn'
    main.lua:13: in <main>
]])
end)

it("Depth recursion", function()
    assert_test("depth_recursion", [[
Runtime error: main.lua:10: Depth reached 0!
Stack traceback:
    C: in function 'error'
    main.lua:10: in function 'lua_fn'
    spec/tracebacks/depth_recursion/module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    spec/tracebacks/depth_recursion/module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    spec/tracebacks/depth_recursion/module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    spec/tracebacks/depth_recursion/module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    spec/tracebacks/depth_recursion/module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    main.lua:16: in <main>
]])
end)

it("Anonymous Lua Fn", function()
    assert_test("anon_lua", [[
Runtime error: main.lua:9: Error from a C function, which has no trace in Lua callstack!
Stack traceback:
    spec/tracebacks/anon_lua/module.c:48: in function 'some_oblivious_c_function'
    spec/tracebacks/anon_lua/module.c:92: in function 'module_fn_2'
    main.lua:9: in function '<?>'
    spec/tracebacks/anon_lua/module.c:61: in function 'module_fn_1'
    main.lua:12: in <main>
]])
end)

it("Traceback Ellipsis", function()
    assert_test("ellipsis", [[
Runtime error: C stack overflow
Stack traceback:
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'

    ... (Skipped 380 frames) ...

    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    spec/tracebacks/ellipsis/module.c:52: in function 'module_fn'
    main.lua:9: in function 'lua_fn'
    main.lua:12: in <main>
]])
end)
