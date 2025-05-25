--
-- Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
--
-- SPDX-License-Identifier: GPL-2.0-or-later
--

---
--- Describes the testing suite for the Mupen64 Lua API.
---

dofile(debug.getinfo(1).source:sub(2):gsub("[^\\]+$", "") .. 'prelude.lua')

lust.describe('mupen64', function()
    lust.describe('shims', function()
        lust.describe('table', function()
            lust.it('get_n_works', function()
                lust.expect(table.getn({ 1, 2, 3 })).to.equal(3)
            end)
        end)
    end)

    lust.describe('movie', function()
        lust.describe('play', function()
            lust.it('returns_ok_result_with_non_nil_path', function()
                lust.expect(movie.play("i_dont_exist_but_whatever.m64")).to.equal(Mupen.result.res_ok)
            end)
            lust.it('returns_bad_file_result_with_nil_path', function()
                lust.expect(movie.play(nil)).to.equal(Mupen.result.vcr_bad_file)
            end)
        end)
    end)
end)
