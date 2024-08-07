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
Runtime error: main.lua:9: Error from a C function, which has no trace in Lua callstack!
Stack traceback:
    module.c:48: in function 'some_oblivious_c_function'
    module.c:92: in function 'module_fn_2'
    main.lua:9: in function 'lua_callee_1'
    module.c:61: in function 'module_fn_1'
    main.lua:13: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:16: in <main>
    C: in function '<?>'
]])
end)

it("Singular", function()
    assert_example("singular", [[
Runtime error: main.lua:9: Life's !good
Stack traceback:
    module.c:49: in function 'lifes_good_fn'
    module.c:59: in function 'singular_fn'
    main.lua:9: in function 'some_lua_fn'
    main.lua:13: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:16: in <main>
    C: in function '<?>'
]])
end)


it("Multi-module", function()
    assert_example("multimod", [[
Runtime error: main.lua:10: Error from another module!
Stack traceback:
    module_b.c:16: in function 'another_mod_fn'
    main.lua:10: in function 'some_lua_fn'
    module_a.c:18: in function 'some_mod_fn'
    main.lua:14: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:17: in <main>
    C: in function '<?>'
]])
end)

it("Depth recursion", function()
    assert_example("depth_recursion", [[
Runtime error: main.lua:10: Depth reached 0!
Stack traceback:
    C: in function 'error'
    main.lua:10: in function 'lua_fn'
    module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    module.c:56: in function 'module_fn'
    main.lua:13: in function 'lua_fn'
    main.lua:19: in function 'wrapper'
    C: in function 'xpcall'
    main.lua:22: in <main>
    C: in function '<?>'
]])
end)

it("Anonymous Lua Fn", function()
    assert_example("anon_lua", [[
Runtime error: main.lua:9: Error from a C function, which has no trace in Lua callstack!
Stack traceback:
    module.c:48: in function 'some_oblivious_c_function'
    module.c:92: in function 'module_fn_2'
    main.lua:9: in function '<?>'
    module.c:61: in function 'module_fn_1'
    main.lua:13: in function '<?>'
    C: in function 'xpcall'
    main.lua:16: in <main>
    C: in function '<?>'
]])
end)
