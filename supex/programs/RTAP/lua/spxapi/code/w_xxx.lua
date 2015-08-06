local only = require('only')
local supex = require('supex')

module('w_xxx', package.seeall)


function handle()
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.sina.com\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("www.sina.com", 80, data, #data)
	only.log("D", info)
	print(ok, info)
end

