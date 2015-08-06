local scan = require('scan')


local abc = {
	"xxx",
	"0000",
	xx = "1111",
	yy = "yy",
	ax = "ax",
	zz = "zzz"
}
local x = scan.dump(abc)
print(x)
local x = scan.dump(abc)
print(x)
do return end

function mem_record (tag, fun, max)
	local mem = {}
	setmetatable(mem, {__mode = "kv"})
	local map = {}
	map["MAX"] = max
	map["IDX"] = 0

	if not __G_MEM then
		__G_MEM = {}
		setmetatable(__G_MEM, {__mode = "k"})
	end

	__G_MEM[tag] = function (...)
		local x = scan.dump({...})
		print(x)
		local r = mem[x]
		if r == nil then
			local idx = map["IDX"] + 1
			if idx > map["MAX"] then
				idx = 1
			end
			map["IDX"] = idx
			local old = map[idx]
			if old then
				mem[ old ] = nil
			end
			map[idx] = x

			r = { fun(...) }
			mem[x] = r
		end
		return unpack(r)
	end
	return __G_MEM[tag]
end

---------------------------------------------------------------
function test(a, b)
	os.execute("sleep 2")
	return true, ((a + 1 + 1 + 1 + 1 + 1 + 1) - 6)/1*8/8*8/8-3/6
end

---------------------------------------------------------------
--[[
collectgarbage("collect")
print("\n\n")
print( collectgarbage("count") )
local test1 = mem_record( "xx", test, 5 )

collectgarbage("collect")

print("\n\n")
print( collectgarbage("count") )
print( test1(5) )
print( collectgarbage("count") )
print( test1(6) )
print( collectgarbage("count") )
print( test1(5) )
print( collectgarbage("count") )
print("\n\n")

collectgarbage("collect")
print( collectgarbage("count") )
]]--

local test1 = mem_record( "xx", test, 5 )
--[[
print( test1(1) )
print( test1(2) )
print( test1(3) )
print( test1(4) )
print( test1(5) )
--print( test1(6) )
print( test1(1) )
]]--
print( test1(1, 1) )
print( test1(1, 1) )
