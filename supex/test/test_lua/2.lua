
require "1"

compile [[
|#include "3.lua"|
abc(">>>>")
|#include "4.lua"|
xyz("<<<<")
]]
print("22222222")

