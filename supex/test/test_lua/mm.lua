function mem_record (tag, fun)
	local mem = {}
	setmetatable(mem, {__mode = "kv"})
	if not __G_MEM then
		__G_MEM = {}
		setmetatable(__G_MEM, {__mode = "k"})
	end

	__G_MEM[tag] = function (...)
		local x = ...
		if type(...) == 'table' then
			for k,v in pairs(...) do
				if type(k) ~= 'number' or type(v) ~= 'string' then
					return fun(...)
				end
			end
			x = table.concat(..., "|")
		end
		local r = mem[x]
		if r == nil then
			r = { fun(...) }
			mem[x] = r
		end
		return unpack(r)
	end
	return __G_MEM[tag]
end

function mem_obtain (tag, ...)
	return __G_MEM[tag](...)
end

function mem_forget (tag)
	__G_MEM[tag] = nil
	collectgarbage("collect")
end
---------------------------------------------------------------
function test(a)
	os.execute("sleep 1")
	return true, ((a + 1 + 1 + 1 + 1 + 1 + 1) - 6)/1*8/8*8/8-3/6
end
---------------------------------------------------------------

collectgarbage("collect")
print( collectgarbage("count") )
mem_record( "xx", test )
mem_record( "yy", test )

collectgarbage("collect")
print( collectgarbage("count") )
print( mem_obtain("xx", 5) )
print( mem_obtain("xx", 6) )

print( collectgarbage("count") )
mem_forget ("xx")
print( collectgarbage("count") )



print( mem_obtain("yy", 5) )
mem_forget ("yy")
print( collectgarbage("count") )
---------------------------------------------------------------
collectgarbage("collect")
print("\n\n")
print( collectgarbage("count") )
local test1 = mem_record( "xx", test )
local test2 = mem_record( "yy", test )

collectgarbage("collect")
print( collectgarbage("count") )
print( test1(5) )
print( test1(6) )

print( collectgarbage("count") )
test1 = nil
collectgarbage("collect")
print( collectgarbage("count") )



print( test2(5) )
test2 = nil
collectgarbage("collect")
print( collectgarbage("count") )
