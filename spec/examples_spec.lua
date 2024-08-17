-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

local util = require "spec.util"

local function assert_example(example, expected_content)
    local cdir  = util.shell_quote("examples/"..example)
    local ok, err, output_content, _ =
        util.outputs_of_execute(string.format("cd %s && make --quiet&& ../../pt-lua main.lua", cdir))
    assert(ok, err)
    assert.are.same(expected_content, output_content)

    -- With Pallene Tracer tracebacks enabled
    local ok, err, output_content, _ =
        util.outputs_of_execute(string.format([[
            cd %s
            make clean --quiet
            make debug --quiet
            ../../pt-lua main.lua
        ]], cdir))
    assert(ok, err)
    assert.are.same(expected_content, output_content)
end

it("Fibonacci", function()
    assert_example("fibonacci", "102334155\n")
end)

it("Find Connected Components", function()
    assert_example("connected-components", "3\n")
end)
