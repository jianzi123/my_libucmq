local only = require('only')
local pool = require('pool')
local supex = require('supex')

module('weidb_test', package.seeall)


function handle()
	only.log("E", "??????")
	local data = 'GET / HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: www.baidu.com\r\n' ..
	'Accept: */*\r\n\r\n'

	local ok, info = supex.http("www.baidu.com", 80, data, #data)
	print(ok, info)
	--print(ok)
	--os.execute("sleep 2")
	--print(pool["OUR_INFO_DATA"])
end

