package.path = "../../open/lib/?.lua;" .. package.path
package.cpath = "../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local redis = require('redis')
local sha = require('sha1')

local host = "127.0.0.1"
local port = 2222
local serv = "spxapi"

local function http(cfg, data)
	local tcp = socket.tcp()
	if tcp == nil then
		error('load tcp failed')
		return false
	end
	tcp:settimeout(10000)
	local ret = tcp:connect(cfg["host"], cfg["port"])
	if ret == nil then
		error("connect failed!")
		return false
	end

	tcp:send(data)
	local result = tcp:receive("*a")
	tcp:close()
	return result
end

local function gen_url(T)
	local url
	for k,v in pairs(T or {}) do
		if url then
			url = string.format([[%s&%s=%s]], url, k, v)
		else
			url = k .. '=' .. v
		end
	end
	return url or ""
end

local function gen_sign(T, secret)
	if not secret then
		local ok,conv = pcall(redis.connect, "192.168.1.11", 6379)
		secret = conv:get(T['appKey'] .. ':secret')
		conv:quit()
		print(T['appKey'], secret)
	end

	local kv_table = {}
	for k,v in pairs(T) do
		if type(v) ~= "table" then
			if k ~= "sign" then
				table.insert(kv_table, k)
			end
		end
	end
	table.insert(kv_table, "secret")
	table.sort(kv_table)
	local sign_string = kv_table[1] .. T[kv_table[1]]
	for i = 2, #kv_table do
		if kv_table[i] ~= 'secret' then
			sign_string = sign_string .. kv_table[i] .. T[kv_table[i]]
		else
			sign_string = sign_string .. kv_table[i] .. secret
		end
	end
	--local ngx = require('ngx')
	--local raw_sign_str = ngx.escape_uri(sign_string)


	--local result = sha.sha1(raw_sign_str)
	local result = sha.sha1(sign_string)
	local sign_result = string.upper(result)

	return sign_result
end

local function gen_data( path, body )
	return "POST /" .. path .. " HTTP/1.0\r\n" ..
	"User-Agent: curl/7.33.0\r\n" ..
	string.format("Host: %s:%d\r\n", host, port) ..
	--"Content-Type: application/json; charset=utf-8\r\n" ..
	--"Content-Type: application/x-www-form-urlencoded\r\n" ..
	"Connection: close\r\n" ..
	"Content-Length:" .. #body .. "\r\n" ..
	"Accept: */*\r\n\r\n" ..
	body
end


local body = {
	spx_txt_to_voice = {
		appKey = "864537339",
		accountID = "8888888888",
		text = "hello world",
	},
}

for key,val in pairs(body) do
	val['sign'] = gen_sign(val)
	local data = gen_data( key, gen_url(val) )
	print(data)
	local info = http({host = host, port = port}, data)
	print(info)
end
