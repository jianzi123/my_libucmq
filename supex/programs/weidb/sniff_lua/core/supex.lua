local APP_POOL = require('pool')
local scan = require('scan')
local supex_http = _G.supex_http
local supex_say = _G.app_lua_add_send_data

module("supex", package.seeall)

function http(host, port, data, size)
	return supex_http(APP_POOL["_TASKER_SCHEME"], host, port, data, size)
end

function spill(data)
	return supex_say(APP_POOL["_SOCKET_HANDLE"], data)
end

function rgs( status )
	local afp = { }
	setmetatable(afp, { __index = _M })
	afp.status = status

	afp.fsize = 0
	afp.fdata = { }

	return afp
end

function say(afp, data)
	table.insert(afp.fdata, data)
	afp.fsize = afp.fsize + data:len()
end

function over(afp)
	--> update body
	local body = table.concat(afp.fdata)
	--> update head
	local head = string.format('HTTP/1.1 %s OK\r\nServer: supex/1.0.0\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n', afp.status, afp.fsize)
	--> flush data
	local data = head .. body
	return supex_say(APP_POOL["_SOCKET_HANDLE"], data)
end


function split_http_data(msg)
	local str = string.match(msg,"(%a+) /")
	if not str then
		return nil
	end

	local method = string.upper(str)
	local head = nil
	local body = nil
	if method == "GET" then
		head = msg
	elseif method == "POST" then
		local head_end = string.find(msg,"\r\n\r\n")
		if head_end then
			head = string.sub(msg,1,head_end)
			body = string.sub(msg,head_end + 4 , #msg)
		end
	end

	local uri = string.match(msg," /(%w.+) HTTP")
	local find_path = string.find(uri,"%?")
	local path = ""
	local uri_arg = nil
	if uri then
		if find_path then
			path = string.sub(uri,1, find_path - 1 )
			uri_arg = string.sub(uri,find_path + 1 , #uri)
		else
			path = uri
			uri_arg = nil
		end
	end
	only.log('D',string.format("method:%s \r\npath:%s \r\nuri_arg:%s \r\nhead:[%s] \r\nbody:[%s]" , method, path , uri_arg, head , body ))
	return method , head , body , path , uri_arg
end



local __G_MEM = {}
setmetatable(__G_MEM, {__mode = "k"})

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


function mem_function_record (tag, fun, max)
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
