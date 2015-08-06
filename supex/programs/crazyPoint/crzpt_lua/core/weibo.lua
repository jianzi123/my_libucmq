local utils = require('utils')
local cjson = require('cjson')
local http_api = require('crzpt').http

module('weibo', package.seeall)

local weibo_api_list = {
	["online"]	= "weiboapi/v2/sendMultimediaOnlineWeibo",
	["personal"]	= "weiboapi/v2/sendMultimediaPersonalWeibo",
	["group"]	= "weiboapi/v2/sendMultimediaGroupWeibo",
	["city"]	= "weiboapi/v2/sendMultimediaOnlineCityWeibo",
}

function send_weibo( server, wb_type, wb_args, appkey, secret )
	local path = weibo_api_list[ wb_type ]
	if not path then
		return false, string.format("No this weibo api : {%s}", wb_type)
	end
	
	wb_args["appKey"] = appkey
	wb_args['sign'] = utils.gen_sign(wb_args, secret)
	local data = utils.compose_http_form_request(server, path, nil, wb_args, nil, nil)
	
	only.log('D',"http request data :\n" .. data)
	local ok, ret = http_api(server['host'], server['port'], data, #data)
	only.log('D', ret)
	local body = string.match(ret, '{.*}')
	local ok, jo = pcall(cjson.decode, body)
	if not ok then
		return false,jo
	end
	if tonumber(jo["ERRORCODE"]) ~= 0 then
		return false,jo["RESULT"]
	end
	return true,jo["RESULT"]
end


-->[多媒体个人微博]<--
--[[----------------->
* multimediaURL,multimediaFile:	`前者为多媒体文件的URL地址,后者为多媒体文件流,此二者必选其一`
* receiverAccountID:		`接收者的IMEI或accountID`
* interval:			`微博有效时间长度,单位为秒`
* level:			`微博的优先级    0-99.0:优先级最高，99:优先级最低`
* content:			`微博的文本内容`
* senderType:			`添加发送类型区分微博来源 1:WEME    2:system    3:other`
--]]----------------->

-->[多媒体集团微博]<--
--[[----------------->
* multimediaURL,multimediaFile:	`前者为多媒体文件的URL地址,后者为多媒体文件流,此二者必选其一`
* groupID:			`发送的集团编号`
* interval:			`微博有效时间长度,单位为秒`
* level:			`微博的优先级    0-99.0:优先级最高，99:优先级最低`
* content:			`微博的文本内容`
* senderType:			`添加发送类型区分微博来源 1:WEME    2:system    3:other`
--]]----------------->

-->[多媒体城市微博]<--
--[[----------------->
* multimediaURL,multimediaFile:	`前者为多媒体文件的URL地址,后者为多媒体文件流,此二者必选其一`
* regionCode:			`城市编号`
* interval:			`微博有效时间长度,单位为秒`
* level:			`微博的优先级    0-99.0:优先级最高，99:优先级最低`
* content:			`微博的文本内容`
* senderType:			`添加发送类型区分微博来源 1:WEME    2:system    3:other`
--]]----------------->

-->[多媒体在线用户微博]<--
--[[----------------->
* multimediaURL,multimediaFile:	`前者为多媒体文件的URL地址,后者为多媒体文件流,此二者必选其一`
* interval:			`微博有效时间长度,单位为秒`
* level:			`微博的优先级    0-99.0:优先级最高，99:优先级最低`
* content:			`微博的文本内容`
* senderType:			`添加发送类型区分微博来源 1:WEME    2:system    3:other`
--]]----------------->
