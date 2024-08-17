local component = require "component"

local nodes = 12
local graph = {
    { 2 }, -- 1st node
    { 1, 5 }, -- 2nd node
    { 5 }, -- 3rd node
    { 5 },
    { 2, 3, 4 },
    { 7, 8 },
    { 6 },
    { 6 },
    { 10, 12 },
    { 9, 11, 12 },
    { 10, 12 },
    { 9, 10, 11 } -- 12th node
};

local total_connected_components = component.find_connected_components(graph, nodes);
print(total_connected_components)  -- expect: 3
