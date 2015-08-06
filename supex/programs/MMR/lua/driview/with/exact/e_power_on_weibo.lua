-- auth: chengjian
-- time: 2014.02.11

local utils = require('utils')
local only = require('only')
local APP_POOL = require('pool')
local redis_api = require('redis_pool_api')
local weibo = require('weibo')
local link = require('link')


module('e_power_on_weibo', package.seeall)

function bind()
	return '["powerOn", "accountID", "tokenCode", "model"]'
end

function match()
	if not weibo.check_driview_subscribed_msg(accountID, weibo.DRI_APP_POWER_ON_WEIBO) then
		only.log('D','开机微博，被客户禁止!')
		return false
	end
	return true
end

function work()

	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]

	-- 判断发送哪个时间段的文本
	local idx = 1
	local currrent = os.date("%X", os.time())
	local time = {
		{min = '05:00:00', max = '09:30:00'},
		{min = '09:30:01', max = '11:30:00'},
		{min = '11:30:01', max = '14:00:00'},
		{min = '14:00:01', max = '17:30:00'},
		{min = '17:30:01', max = '20:00:00'},
		{min = '20:00:01', max = '23:00:00'},
		{min = '23:00:01', max = '23:59:59'},
		{min = '00:00:00', max = '04:59:59'},
	}
	for i = 1 , #time do
		if currrent > time[i]['min'] and currrent < time[i]['max']  then
			idx = i
			break
		end
	end
	local idx_type = {"one","two","three","four","five","six","seven","seven"}
	local fileURL = "http://127.0.0.1/productList/powerOnGreetings/" .. idx_type[idx]
	local idx_file = {
		{"101.amr","102.amr","103.amr","104.amr","105.amr","106.amr","107.amr","108.amr"},
		{"109.amr","110.amr","111.amr","112.amr"}, 
		{"113.amr","114.amr","115.amr","116.amr"}, 
		{"117.amr","118.amr","119.amr"}, 
		{"120.amr","121.amr","122.amr","123.amr","124.amr"}, 
		{"125.amr","126.amr","127.amr","128.amr"}, 
		{"129.amr","130.amr","131.amr","132.amr","133.amr","134.amr"}, 
		{"129.amr","130.amr","131.amr","132.amr","133.amr","134.amr"}, 
	}
	local id = (os.time() % #idx_file[idx]) + 1
	fileURL = fileURL .. "/" .. idx_file[idx][id]

	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = 10,
		interval = 60*10,
		senderType = 2,
	}

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, "3689953515", "3E351DD73E3FA09A7F0BE01B19601446DDE6939B" )
	if not ok then
		only.log("E", "send weibo failed : " .. err)
	end
end

