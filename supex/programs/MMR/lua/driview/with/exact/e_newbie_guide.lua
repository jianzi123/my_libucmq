-- auth: baoxue
-- time: 2014.04.28

local utils = require 'utils'
local only = require('only')
local APP_POOL = require('pool')
local redis_api = require('redis_pool_api')
local link = require('link')
local cjson = require('cjson')
local weibo = require('weibo')


module('e_newbie_guide', package.seeall)


function bind()
	return '["powerOn", "accountID", "tokenCode", "model"]'
end

function match()
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	if not weibo.check_driview_subscribed_msg(accountID, weibo.DRI_APP_NEWBIE_GUIDE) then
		only.log('D','新手教程，被客户禁止!')
        return false;
    end
	return true
end

function work()
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local ok,newbie = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':newbieGuide')
	only.log('I','>>>>>>>>')
	if not ok then
		only.log('E','private redis failed!')
		return
	end
	if not newbie then
		only.log('I','<<<<<<<<')
		redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', accountID .. ':newbieGuide', 1)
		local send_list = {
			{"http://127.0.0.1/productList/newbieGuide/1.amr", 15},
			{"http://127.0.0.1/productList/newbieGuide/2.amr", 16},
			{"http://127.0.0.1/productList/newbieGuide/3.amr", 17},
			{"http://127.0.0.1/productList/newbieGuide/4.amr", 18},
			{"http://127.0.0.1/productList/newbieGuide/5.amr", 19},
			{"http://127.0.0.1/productList/newbieGuide/6.amr", 20},
			{"http://127.0.0.1/productList/newbieGuide/7.amr", 21},
			--{"http://127.0.0.1/productList/newbieGuide/8.amr", 22},
		}
		local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
		for _,v in ipairs( send_list ) do
			local wb = {
				multimediaURL = v[1],
				receiverAccountID = accountID,
				level = v[2],
				interval = 60*10,
				senderType = 2,
			}
			local ok,err = weibo.send_weibo( server, "personal", wb, "2875909808", "2A59FFD121CCC2ECEBB2F1272B5A521E6A938635" )
			if not ok then
				only.log("E", "send weibo failed : " .. err)
			end
		end
	end
end
