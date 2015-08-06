function mem_record (key, val)
	if not __G_MEM then
		__G_MEM = {}
		setmetatable(__G_MEM, {__mode = "k"})
	end

	__G_MEM["KEY"] = key
	__G_MEM["VAL"] = val
end

function mem_obtain (key)
	if not __G_MEM then
		return nil
	end
	if key ~= __G_MEM["KEY"] then
		return nil
	else
		return __G_MEM["VAL"]
	end
end
--[[
function mem_record (key, val)
	if not __G_MEM then
		__G_MEM = {}
		setmetatable(__G_MEM, {__mode = "k"})
	end

	__G_MEM["KEY"] = key
	__G_MEM["TAG"] = {key}
	__G_MEM[ __G_MEM["TAG"] ] = val
end

function mem_obtain (key)
	if not __G_MEM then
		return nil
	end
	if key ~= __G_MEM["KEY"] then
		return nil
	else
		return __G_MEM[ __G_MEM["TAG"] ]
	end
end
]]--

local function test()
	local key1 = "12341"
	local key2 = "abcwd"
	local val1 = "xxxxxxxxxxx"
	local val2 = "yyyyyyyyyyy"

	collectgarbage("collect")
	print( collectgarbage("count") )

	mem_record (key1, val1)
	print(mem_obtain("12341"))
	collectgarbage("collect")
	print( collectgarbage("count") )
	print(mem_obtain("12341"))
	key1 = key2
	print(mem_obtain("12341"))
	mem_record (key1, val2)
	collectgarbage("collect")
	print( collectgarbage("count") )
	print(mem_obtain("12341"))
	print(mem_obtain("abcwd"))
end

test()
collectgarbage("collect")
print("===================");
for k,v in pairs(__G_MEM) do
	print(k, v)
end

--[[
local key1 = {"12341"}
local key2 = {"abcwd"}
local val1 = "xxxxxxxxxxx"
local val2 = "yyyyyyyyyyy"

local mem = {}
setmetatable(mem, {__mode = "k"})

collectgarbage("collect")
print( collectgarbage("count") )
mem[ key1 ] = val1
collectgarbage("collect")
print( collectgarbage("count") )
key1 = key2
mem[ key1 ] = val2
collectgarbage("collect")
print( collectgarbage("count") )
for k,v in pairs(mem) do
	print(k, v)
end
print(mem[ {"abcwd"} ])
]]--



--------------------------------------------------------------
function mem_classify_record (own, key, val)
	if not __G_MEM then
		__G_MEM = {}
		setmetatable(__G_MEM, {__mode = "k"})
	end
	if not __G_MEM[own] then
		__G_MEM[own] = {}
		setmetatable(__G_MEM[own], {__mode = "kv"})
	end

	__G_MEM[own]["KEY"] = key
	__G_MEM[own]["VAL"] = val
end

function mem_classify_obtain (own, key)
	if not __G_MEM or not __G_MEM[own] then
		return nil
	end
	if key ~= __G_MEM[own]["KEY"] then
		return nil
	else
		return __G_MEM[own]["VAL"]
	end
end


local function test()
	local key1 = "12341"
	local key2 = "abcwd"
	local val1 = "xxxxxxxxxxx"
	local val2 = "yyyyyyyyyyy"

	collectgarbage("collect")
	print( collectgarbage("count") )

	mem_classify_record ("abc", key1, val1)
	print(mem_classify_obtain("abc", "12341"))
	collectgarbage("collect")
	print( collectgarbage("count") )
	print(mem_classify_obtain("abc", "12341"))
	key1 = key2
	print(mem_classify_obtain("abc", "12341"))
	mem_classify_record ("abc", key1, val2)
	collectgarbage("collect")
	print( collectgarbage("count") )
	print(mem_classify_obtain("abc", "12341"))
	print(mem_classify_obtain("abc", "abcwd"))
end

test()
collectgarbage("collect")
print("===================");
for k,v in pairs(__G_MEM["abc"]) do
	print(k, v)
end

