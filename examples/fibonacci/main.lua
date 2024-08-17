-- Copyright (c) 2024, The Pallene Developers
-- Pallene Tracer is licensed under the MIT license.
-- Please refer to the LICENSE and AUTHORS files for details
-- SPDX-License-Identifier: MIT

local N = tonumber(arg[1]) or 40

local fibonacci = require "fibonacci"
print(fibonacci.fib(N))
