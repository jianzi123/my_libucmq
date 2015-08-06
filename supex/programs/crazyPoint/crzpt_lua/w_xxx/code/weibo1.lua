local utils     = require('utils')
local cjson     = require('cjson')
local http_api  = require('supex1').http
local redis_api = require('redis_pool_api')
local init_data = require('init_data1')
local APP_POOL  = require('pool1')
module('weibo1', package.seeall)

local weibo_api_list = {
	["online"]   = "weiboapi/v2/sendMultimediaOnlineWeibo",
	["personal"] = "weiboapi/v2/sendMultimediaPersonalWeibo",
	["group"]    = "weiboapi/v2/sendMultimediaGroupWeibo",
	["city"]     = "weiboapi/v2/sendMultimediaOnlineCityWeibo",
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



-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
----------------------------jiang z.s. ---------------------------------


----大类序号	大类名称
SYS_WB_ALL_MSG      = 1 	----1	总开关
SYS_WB_AROUND_VOICE = 2 	----2	获取周围吐槽
SYS_WB_AD           = 3 		----3	广告
SYS_WB_DRIVIEW      = 4 	----4	Driview
SYS_WB_CRAZY        = 5 	----5	正点时钟
SYS_WB_CHANNEL      = 6     ----6 频道,DJ 


--1~32   	32位系统使用
--33~xxx  N位,给所有业务使用


---- check_system_subscribed_msg
---- return 
---- true  	可以发送微博
---- false	屏蔽微博
---- WEME 判断用户是否开启以下功能
function check_system_subscribed_msg( accountid , weibo_type )
	if not weibo_type then return true end
	local ok_status,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':userMsgSubscribed')
	if not ok_status or not ok_status then return  true end
	if not ok_val or #ok_val < weibo_type then return true end
	if weibo_type == WB_TYPE_ALL_MSG and tostring(string.sub(ok_val,1,1)) == "1" then
		return true
	elseif weibo_type ~= WB_TYPE_ALL_MSG then
		if tostring(string.sub(ok_val,1,1)) == "1"  and tostring(string.sub(ok_val,tonumber(weibo_type),tonumber(weibo_type))) == "1"  then
			return true
		end
	end
	return false
end

--------------------------------------------------------------------------------------------------------------------------
------------- 以下内容,请严格按照当前方式,定义常量   注意添加备注,日期等 ---------------------------------------------
-------------  同时需要更新表`app_weibo`.`subscribedTypeInfo`, 注意subscribedType -------------------------
------------------------------[CrazyAPI index:(34  ~  96) ]-----------------------------------------------------------

-- 34		分享驾驶信息
-- 35		汤姆猫自动回复
-- 36		最近道客
-- 37		当前驾驶方向与状态
-- 38		PM2.5
-- 39		新闻
-- 40		在线用户提醒
-- 41		夜驾提醒

CRAZY_APP_SHARE_DRI_INFO       = 34 
CRAZY_APP_AUTO_REPLY_BY_TOMCAT = 35 
CRAZY_APP_VICINITY_DK          = 36 
CRAZY_APP_CURRENT_DRI_INFO     = 37
CRAZY_APP_PM_2_5               = 38 
CRAZY_APP_NEWS                 = 39 
CRAZY_APP_REPORT_ONLINEUSER    = 40 
CRAZY_APP_DRI_AT_NINGHT        = 41




--========================================================================================================================

------------------------------[Driview index:(97  ~  160) ]-----------------------------------------------------------
DRI_APP_HOME_OFFSET     = 98        ----异地触发
DRI_APP_TRAFFIC_REMIND  = 99        ----路况
DRI_APP_ONLINE_REPORT	= 100 	    ----用户上线提醒
--DRI_APP_DAOKE_SPEED     = 102 	    ----道客时速
DRI_APP_SOLARCALENDER	= 103		----青年小阳历
DRI_APP_DRIVE_MILEAGE	= 104		----在线(连续)行驶历程
DRI_APP_FATIGUE_DRIVING = 105		----疲劳驾驶
DRI_APP_NEWBIE_GUIDE    = 106		----新手教程

DRI_APP_MON_DRIVE_MILEAGE= 142         ----当月驾驶里程是否达标提醒
DRI_APP_HIGHWAY_OVER_SPEED = 143         ---- 高速超速提醒
DRI_APP_URBAN_OVER_SPEED = 144         ---- 城区道路超速提醒
DRI_APP_DRIVING_PATTERN = 145         ---- 驾驶模式识别
DRI_APP_LIMIT_SPEED_REMIND = 146         ---- 道路限速提醒

---4_MILES---
DRI_APP_4_MILES_FARMER_MARKET                               = 107 --农贸市场
DRI_APP_4_MILES_ZOO                                         = 108 --动物园
DRI_APP_4_MILES_ARBORETUM                                   = 109 --植物园
DRI_APP_4_MILES_COMMUNITY                                   = 110 --小区
DRI_APP_4_MILES_MIDDLE_SCHOOL                               = 111 --中学
DRI_APP_4_MILES_PRIMARY_SCHOOL                              = 112 --小学
DRI_APP_4_MILES_NURSERY_SCHOOL                              = 113 --幼儿园
DRI_APP_4_MILES_TUNNEL                                      = 114 --隧道
DRI_APP_4_MILES_INTERCHANGE                                 = 115 --立交桥
DRI_APP_4_MILES_ROUNDABOUT                                  = 116 --环岛
DRI_APP_4_MILES_HIGHWAY_TOLL_STATION                        = 117 --高速收费站
DRI_APP_4_MILES_GAS_STATION                                 = 118 --加油站
DRI_APP_4_MILES_SERVICE_AREA                                = 119 --服务区
DRI_APP_4_MILES_GREAT_BRIDGE                                = 120 --大桥
DRI_APP_4_MILES_TOLL_STATION                                = 121 --非高速收费 
DRI_APP_4_MILES_MAXIMUM_SPEED_LIMIT                         = 122 --最大限速
DRI_APP_4_MILES_MINIMUM_SPEED                               = 123 --最低限速
DRI_APP_4_MILES_FALLING_ROCKS_AHEAD                         = 124 --注意落石
DRI_APP_4_MILES_NECK_BREAKING_SECTION_AHEAD                 = 125 --事故易发地 
DRI_APP_4_MILES_EASY_GLIDE                                  = 126 --易滑
DRI_APP_4_MILES_VILLAGE                                     = 127 --村庄
DRI_APP_4_MILES_OVERPASS                                    = 128 --过街天桥
DRI_APP_4_MILES_SOMEONE_TO_LOOK_AFTER_THE_RAILWAY_CROSSING  = 129 --有人看管的铁路道口
DRI_APP_4_MILES_UNATTENDED_RAILWAY_CROSSING                 = 130 --无人看管的铁路道口
DRI_APP_4_MILES_NO_OVERTAKING                               = 131 --禁止超车
DRI_APP_4_MILES_PARKING_LANE                                = 132 --停车让行
DRI_APP_4_MILES_DECELERATION_LANE                           = 133 --减速让行
DRI_APP_4_MILES_PASSING_LINES                               = 134 --会车让行
DRI_APP_4_MILES_ELECTRONIC_EYE                              = 135 --电子眼
DRI_APP_4_MILES_DIRECTION_OF_KANBAN                         = 136 --方向看板
DRI_APP_4_MILES_EXHIBITION_HALL                             = 137 --展览馆
DRI_APP_4_MILES_DRIVING_SCHOOL                              = 138 --驾校
DRI_APP_4_MILES_CONSECUTIVE_CURVE                           = 139 --连续弯路
DRI_APP_4_MILES_RAILWAY_CROSSING                            = 140 --铁路道口
DRI_APP_4_MILES_REVERSE_CURVE                               = 141 --反向弯路   
DRI_APP_4_MILES_SPEED_CAMERA                                = 147 --限速摄像头
DRI_APP_4_MILES_ILLEGAL_CAMERA                              = 148 --违章摄像头
DRI_APP_4_MILES_GYMNASIUM                                   = 149 --体育馆
DRI_APP_4_MILES_TOURIST_SPOT                                = 150 --旅游景点
DRI_APP_4_MILES_CARRIAGEWAY_NARROWS                         = 151 --道路变窄
DRI_APP_4_MILES_SHARP_TO_THE_LEFT                           = 152 --向左急弯路
DRI_APP_4_MILES_SHARP_TO_THE_RIGHT                          = 153 --向右急弯路
DRI_APP_4_MILES_CONFLUENCE_OF_LEFT                          = 154 --左侧合流
DRI_APP_4_MILES_CONFLUENCE_OF_RIGHT                         = 155 --右侧合流

DRI_APP_POWER_ON_WEIBO                                      = 156 --开机微博
DRI_APP_4_MILES_HIGHWAY_EXIT                                = 157 --高速出口
DRI_APP_WEATHER_FORCAST                                     = 158 --天气预报
DRI_APP_4_MILES_RIGHT_TURN_RED_LIGHT                        = 159 --右轉紅燈
DRI_APP_4_MILES_RIGHT_TURN_AVOID_PEDESTRIANS				= 160 --右转避让行人
--[[
DRI_APP_4_MILES_FIRST_CLASS_HOSPITAL_AT_GRADE_3             = 158 --三甲医院
--]]

DRI_APP_4_MILES  = {
	[1118102] = {txt = "动物园",                no = DRI_APP_4_MILES_ZOO},
	[1118103] = {txt = "植物园",                no = DRI_APP_4_MILES_ARBORETUM },
	[1122106] = {txt = "驾校",                  no = DRI_APP_4_MILES_DRIVING_SCHOOL},
	[1114101] = {txt = "农贸市场",              no = DRI_APP_4_MILES_FARMER_MARKET},
	[1115101] = {txt = "体育馆",                no = DRI_APP_4_MILES_GYMNASIUM},
	[1122105] = {txt = "展览馆",                no = DRI_APP_4_MILES_EXHIBITION_HALL},
	[1118101] = {txt = "旅游景点",              no = DRI_APP_4_MILES_TOURIST_SPOT},
	[1123105] = {txt = "加油站",                no = DRI_APP_4_MILES_GAS_STATION},

	[1126116] = {txt = "有人看管的铁路道口",    no = DRI_APP_4_MILES_SOMEONE_TO_LOOK_AFTER_THE_RAILWAY_CROSSING},
	[1126117] = {txt = "无人看管的铁路道口",    no = DRI_APP_4_MILES_UNATTENDED_RAILWAY_CROSSING},
	[1126110] = {txt = "注意落石",              no = DRI_APP_4_MILES_FALLING_ROCKS_AHEAD},
	[1126111] = {txt = "事故易发地段",          no = DRI_APP_4_MILES_NECK_BREAKING_SECTION_AHEAD},
	[1126112] = {txt = "易滑",                  no = DRI_APP_4_MILES_EASY_GLIDE},
	[1126113] = {txt = "村庄",                  no = DRI_APP_4_MILES_VILLAGE},
	[1126118] = {txt = "道路变窄",              no = DRI_APP_4_MILES_CARRIAGEWAY_NARROWS},
	[1126119] = {txt = "向左急弯路",            no = DRI_APP_4_MILES_SHARP_TO_THE_LEFT},
	[1126120] = {txt = "向右急弯路",            no = DRI_APP_4_MILES_SHARP_TO_THE_RIGHT},
	[1126122] = {txt = "连续弯路",              no = DRI_APP_4_MILES_CONSECUTIVE_CURVE},
	[1126123] = {txt = "左侧合流",              no = DRI_APP_4_MILES_CONFLUENCE_OF_LEFT},
	[1126124] = {txt = "右侧合流",              no = DRI_APP_4_MILES_CONFLUENCE_OF_RIGHT},

	--[1123109] = {txt = "电子眼",                  no = DRI_APP_4_MILES_ELECTRONIC_EYE},
	[1122102] = {txt = "中学",                  no = DRI_APP_4_MILES_MIDDLE_SCHOOL},
	[1122103] = {txt = "小学",                  no = DRI_APP_4_MILES_PRIMARY_SCHOOL},
	[1122104] = {txt = "幼儿园",                no = DRI_APP_4_MILES_NURSERY_SCHOOL},

	[1123101] = {txt = "隧道",                  no = DRI_APP_4_MILES_TUNNEL},
	[1123107] = {txt = "大桥",                  no = DRI_APP_4_MILES_GREAT_BRIDGE},
	[1123110] = {txt = "限速摄像头",            no = DRI_APP_4_MILES_SPEED_CAMERA},
	[1123111] = {txt = "违章摄像头",            no = DRI_APP_4_MILES_ILLEGAL_CAMERA},

	----高速跨省收费站 -- 1123112
	----高速出口收费站 -- 1123113
	----非高速收费站   -- 1123108
	[1123112] = {txt = "高速跨省收费站",        no = DRI_APP_4_MILES_HIGHWAY_TOLL_STATION}, --117
	[1123113] = {txt = "高速出口收费站",        no = DRI_APP_4_MILES_HIGHWAY_TOLL_STATION}, --117
	[1123108] = {txt = "非高速收费站",          no = DRI_APP_4_MILES_HIGHWAY_TOLL_STATION}, --117

	--[[
	DRI_APP_4_MILES_FIRST_CLASS_HOSPITAL_AT_GRADE_3             = 157 --三甲医院
	DRI_APP_4_MILES_HIGHWAY_EXIT                                = 158 --高速出口
	--]]
	--[[
	[1116101] = {txt = "三甲医院", no = DRI_APP_4_MILES_FIRST_CLASS_HOSPITAL_AT_GRADE_3 },
	--]]
	[1126157] = {txt = "高速出口", no = DRI_APP_4_MILES_HIGHWAY_EXIT },
	[1123106] = {txt = "服务区", no = DRI_APP_4_MILES_SERVICE_AREA},

	[1126159] = {txt = "右转红灯", no = DRI_APP_4_MILES_RIGHT_TURN_RED_LIGHT },
	[1126160] = {txt = "右转避让行人", no = DRI_APP_4_MILES_RIGHT_TURN_AVOID_PEDESTRIANS},

}

DRI_APP_LIST = {
	l_f_home_offsite = { no = DRI_APP_HOME_OFFSET     , text = "异地触发，被用户禁止!"},
	l_f_highway_mileage_traffic = { no = DRI_APP_TRAFFIC_REMIND  , text = "高德路况，被用户禁止!"},
	l_f_urban_mileage_traffic = { no = DRI_APP_TRAFFIC_REMIND  , text = "高德路况，被用户禁止!"},
	l_f_road_change_traffic = { no = DRI_APP_TRAFFIC_REMIND  , text = "高德路况，被用户禁止!"},
	l_f_road_traffic = { no = DRI_APP_TRAFFIC_REMIND  , text = "roadRank路况，被用户禁止!"},
	---用户上线提醒
	---青年小阳历
	l_f_driving_mileage = { no = DRI_APP_DRIVE_MILEAGE	, text = "在线行驶历程提醒，被用户禁止!"},
	l_f_continuous_driving_mileage = { no = DRI_APP_DRIVE_MILEAGE	, text = "在线行驶历程提醒，被用户禁止!"},
	l_f_fatigue_driving = { no = DRI_APP_FATIGUE_DRIVING , text = "疲劳驾驶，被用户禁止!"},
	---新手教程，手写

	e_f_power_on_mon_driving_mileage = { no = DRI_APP_MON_DRIVE_MILEAGE, text = "当月驾驶里程是否达标提醒，被用户禁止!"},
	l_f_mon_driving_mileage = { no = DRI_APP_MON_DRIVE_MILEAGE, text = "当月驾驶里程是否达标提醒，被用户禁止!"},

	l_f_highway_over_speed = { no = DRI_APP_HIGHWAY_OVER_SPEED , text = "高速超速提醒，被用户禁止!"},
	l_f_urban_over_speed = { no = DRI_APP_URBAN_OVER_SPEED , text = "城区超速提醒，被用户禁止!"},
	l_f_over_speed = { no = DRI_APP_URBAN_OVER_SPEED , text = "超速提醒，被用户禁止!"},

	l_f_driving_pattern = { no = DRI_APP_DRIVING_PATTERN , text = "驾驶模式识别，被用户禁止!"},
	l_f_limit_speed_remind = { no = DRI_APP_LIMIT_SPEED_REMIND , text = "道路限速提醒，被用户禁止!"},
	l_f_weather_forcast = { no = DRI_APP_WEATHER_FORCAST    , text = "天气预报提醒，被用户禁止!"},

}

-- 98	异地提醒	
-- 99	高德路况	
-- 100	上线提醒	
-- 101	灾害天气	
-- 102	道客时速	
-- 103	青年小阳历	
-- 104	在线行驶里程	
-- 105	疲劳驾驶	
-- 106	新手教程	
-- 107	前方4公里(POI提醒)	农贸市场
-- 108	前方4公里(POI提醒)	动物园
-- 109	前方4公里(POI提醒)	植物园
-- 110	前方4公里(POI提醒)	小区
-- 111	前方4公里(POI提醒)	中学
-- 112	前方4公里(POI提醒)	小学
-- 113	前方4公里(POI提醒)	幼儿园
-- 114	前方4公里(POI提醒)	隧道
-- 115	前方4公里(POI提醒)	立交桥
-- 116	前方4公里(POI提醒)	环岛
-- 117	前方4公里(POI提醒)	高速收费站
-- 118	前方4公里(POI提醒)	加油站
-- 119	前方4公里(POI提醒)	服务区
-- 120	前方4公里(POI提醒)	大桥
-- 121	前方4公里(POI提醒)	非高速收费站
-- 122	前方4公里(POI提醒)	最大限速
-- 123	前方4公里(POI提醒)	最低限速
-- 124	前方4公里(POI提醒)	注意落石
-- 125	前方4公里(POI提醒)	事故易发地段
-- 126	前方4公里(POI提醒)	易滑
-- 127	前方4公里(POI提醒)	村庄
-- 128	前方4公里(POI提醒)	过街天桥
-- 129	前方4公里(POI提醒)	有人看管的铁路道口
-- 130	前方4公里(POI提醒)	无人看管的铁路道口
-- 131	前方4公里(POI提醒)	禁止超车
-- 132	前方4公里(POI提醒)	停车让行
-- 133	前方4公里(POI提醒)	减速让行
-- 134	前方4公里(POI提醒)	会车让行
-- 135	前方4公里(POI提醒)	电子眼
-- 136	前方4公里(POI提醒)	方向看板
-- 137	前方4公里(POI提醒)	展览馆
-- 138	前方4公里(POI提醒)	驾校
-- 139	前方4公里(POI提醒)	连续弯路
-- 140	前方4公里(POI提醒)	铁路道口
-- 141	前方4公里(POI提醒)	反向弯路 	
-- 142  当月驾驶里程是否达标提醒
-- 147	前方4公里(POI提醒)	限速摄像头
-- 148	前方4公里(POI提醒)	违章摄像头



------------- 以上内容,请严格按照当前方式,定义常量   注意添加备注,日期等 ---------------------------------------------
--------------------------------------------------------------------------------------------------------------------------


---- [正点时钟]判断用户是否开启以下功能
function check_crazy_subscribed_msg( accountid, app_type )
	if not app_type then return true end
	if not accountid or #accountid ~= 10 then return true end
	local ok_status,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':userMsgSubscribed')
	if not ok_status or not ok_val then return  true end
	if not ok_val or  #ok_val < app_type then return true end
	if app_type < 32 then 
		error( string.format("[weibo]weibo_type is error:%s ", app_type ) )
		return false 
	end

	if tostring(string.sub(ok_val,1,1)) == "1" 							---- 系统总开关
		and tostring(string.sub(ok_val,5,5)) == "1" 						---- [系统]正点时钟
		and tostring(string.sub(ok_val,33,33)) == "1" 					---- [用户订阅]总开关
		and tostring(string.sub(ok_val,tonumber(app_type),tonumber(app_type))) == "1"  then
		return true
	end
	return false
end
---- [appcenter]判断用户是否开启以下功能
function check_appcenter_subscribed_msg( accountid, app_type )
	if not app_type then return true end
	if not accountid or #accountid ~= 10 then return true end
	local ok_status,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':userMsgSubscribed')
	if not ok_val or #ok_val < app_type then 
		return  true 
	end
	if app_type < 32 then 
		error( string.format("[weibo]weibo_type is error:%s ", app_type ) )
		return false 
	end

	if tostring(string.sub(ok_val,1,1)) == "1" 							---- 系统总开关
		and tostring(string.sub(ok_val,4,4)) == "1" 						---- [系统]driview
		and tostring(string.sub(ok_val,33,33)) == "1" 					---- [用户订阅]总开关
		and tostring(string.sub(ok_val,tonumber(app_type),tonumber(app_type))) == "1"  then
		return true
	end
	return false
end


---- [driview]判断用户是否开启以下功能
function check_driview_subscribed_msg( accountid, app_type )
    local accountID = accountid
	if not app_type then return true end
	if not accountid or #accountid ~= 10 then return true end
	--local ok_status,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':userMsgSubscribed')
	local ok_val = init_data.get_user_msg_subscribed (accountID)
	if not ok_val or #ok_val < app_type then 
		return  true 
	end
	if app_type < 32 then 
		error( string.format("[weibo]weibo_type is error:%s ", app_type ) )
		return false 
	end

	if tostring(string.sub(ok_val,1,1)) == "1" 							---- 系统总开关
		and tostring(string.sub(ok_val,4,4)) == "1" 						---- [系统]driview
		and tostring(string.sub(ok_val,33,33)) == "1" 					---- [用户订阅]总开关
		and tostring(string.sub(ok_val,tonumber(app_type),tonumber(app_type))) == "1"  then
		return true
	end
	return false
end


local appkey_tab = {
	appkey1209071138 = 163,
}


local function  get_apptype_by_appkey(app_key)
	return appkey_tab["appkey" .. tostring(app_key) ]
end



function check_thirdapp_subscribed_msg( accountid, app_key )
	if not app_key or not accountid or #accountid ~= 10 then return true end

	local app_type = get_apptype_by_appkey(app_key)
	if not app_type then return true end

	local ok_status,ok_val = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', accountid .. ':userMsgSubscribed')
	if not ok_status or not ok_val or #ok_val < app_type then return  true end
	if app_type < 32 then 
		error( string.format("[weibo]weibo_type is error:%s ", app_type ) )
		return false 
	end

	if tostring(string.sub(ok_val,1,1)) == "1" 									---- 系统总开关
		and tostring(string.sub(ok_val,33,33)) == "1" 							---- 用户总开关
		and tostring(string.sub(ok_val,tonumber(app_type),tonumber(app_type))) == "1"  then
		return true
	end
	return false
end

-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
