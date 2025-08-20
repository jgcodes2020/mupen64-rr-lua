--
-- Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
--
-- SPDX-License-Identifier: GPL-2.0-or-later
--

-- Tries to call APIs protected by the sandbox. Run this script in untrusted mode and verify that all assertions pass.

dofile(debug.getinfo(1).source:sub(2):gsub("[^\\]+$", "") .. 'prelude.lua')

-- Add libsocket to the cpath for the `require` test later.
local libsocket_path = path_root .. "lib\\luasocket\\"
local libsocket_dll_path = libsocket_path .. "socket\\core.dll"
package.cpath = libsocket_path .. "?.dll;" .. package.cpath

local tests = {}
tests[#tests + 1] = function()
    local suc, exitcode, code = os.execute("start calc.exe")
    assert(suc == false)
    assert(exitcode == nil)
    assert(code == nil)
end
tests[#tests + 1] = function()
    local file, err = io.popen("start calc.exe")
    assert(file == nil)
    assert(err == nil)
end
tests[#tests + 1] = function()
    local suc, err = os.remove("a.txt")
    assert(suc == false)
    assert(err == nil)
end
tests[#tests + 1] = function()
    local suc, err = os.rename("a.txt", "b.txt")
    assert(suc == false)
    assert(err == nil)
end
tests[#tests + 1] = function()
    local lib = package.loadlib(libsocket_dll_path, "luaopen_testlib")
    assert(lib == nil)
end
-- tests[#tests + 1] = function()
--     local lib = require("socket.core")

--     assert(lib == nil)
-- end

for key, value in pairs(tests) do
    value()
end
