local t = require "lua_thread"

local function g()
   local c = require "chrono"
   for i = 1, 10 do
      print(i)
      c.waitFor(1000000)
   end
end

local xx = {}

for i = 1, 10 do
   xx[i] = t.newThread(g)
end

for _, v in pairs(xx) do
   t.join(v)
end

