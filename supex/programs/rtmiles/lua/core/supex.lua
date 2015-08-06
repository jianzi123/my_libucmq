local APP_POOL = require('pool')
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
