local only = require('only')
local supex = require('supex')
local redis_api = require('redis_pool_api')

module('w_xxx', package.seeall)


function handle()
	only.log("E", "??????")
	--[[
	local data = 'GET /4 HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: 127.0.0.1\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("127.0.0.1", 80, data, #data)
	print(ok, info)
	]]--
	local ok,ret = redis_api.cmd("private", "accountIDxxx", "set", "testkey", "123456")
	print(ok, ret)
	local ok,ret = redis_api.cmd("private", "accountIDyyy", "set", "testkey", "654321")
	print(ok, ret)
	local ok,ret = redis_api.cmd("private", "accountIDzzz", "set", "testkey2", "111111")
	print(ok, ret)
end

