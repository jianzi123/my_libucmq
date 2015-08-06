local pool          = require('pool')
local APP_POOL      = require('pool')
local only          = require('only')
local supex         = require('supex')
local utils         = require('utils')
local link          = require('link')
local cjson         = require('cjson')
local weibo_api     = require('weibo')
local scan          = require('scan')
--local redis_api = require('redis_short_api')
local redis_api = require('redis_pool_api')

module('w_xxx', package.seeall)


function test ()
	only.log("E", "??????")
	local data = 'GET /2 HTTP/1.0\r\n' ..
	'User-Agent: curl/7.32.0\r\n' ..
	'Host: 127.0.0.1\r\n' ..
	'Accept: */*\r\n\r\n'
	local ok, info = supex.http("127.0.0.1", 8088, data, #data)
	print(ok, info)
end

function handle()
	local fileurl = "http://127.0.0.1"
	local accountID = "13212xsdfd"
	local content = "hello lua"
	local wb = {
		multimediaURL = fileurl,
		receiverAccountID = accountID,
		interval = 40,
		level = 30,
		content = content,
		senderType = 2,
	}
	wb['senderLongitude'] = 123.923
	wb['senderLatitude'] = 23.23
	wb["senderDirection"] = 23
	wb["senderSpeed"] =  2
	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok, ret = weibo_api.send_weibo( server, "personal", wb, "1854678079", "439581000FF13D23F4A23D7DD61C2144B10AFA64" )
	if ok then
		only.log('D', scan.dump(ret))
	else
		only.log('D', "send weibo error")
	end
end

