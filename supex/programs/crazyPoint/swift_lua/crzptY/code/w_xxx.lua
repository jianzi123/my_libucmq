local only = require('only')
local supex = require('supex')

module('w_xxx', package.seeall)


function handle()
	only.log("E", "??????")
	local data = 'GET /2 HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: 127.0.0.1\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("127.0.0.1", 8088, data, #data)
	--print(ok, info)
end


