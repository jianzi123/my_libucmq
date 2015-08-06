local http_api = require("http_short_api")
local APP_CONFIG_LIST = require('CONFIG_LIST1')
local common_cfg = require("cfg1")
local mysql_api = require("mysql_pool_api")
local redis_api = require("redis_pool_api")
local cjson = require("cjson")
local APP_POOL = require("pool1")
local only = require('only')
local link = require("link")
local utils = require("utils")
local supex = require("supex1")
local judge = require("judge1")
local weibo = require('weibo1')


module("WORK_FUNC_LIST1", package.seeall)

-->> public
OWN_JOIN = {
	cause ={
		trigger_type = "every_time,one_day,power_on,fixed_time",
	}
}
OWN_MUST = {
	cause = {
		trigger_type = "every_time",
		fix_num = 1,
		delay = 0
	},
}

-->> private
OWN_HINT = {
	-->> exact local whole
	app_task_forward = {},
	-->> alone
	full_url_send_weibo = {},--{fullURL = "[nickname], [content]"},

	unknow_url_send_weibo = {redis_key = "arrivePoiFileUrl", idx_func = "arrive_poi,get_grid_content,get_off_site_content"},

	half_url_random_idx_send_weibo = {
		halfURL = string.gsub([[http://127.0.0.1/productList/????/%s.amr,
		http://127.0.0.1/productList/????/%s.amr]],
		"[\n\t]*", "")
	},

	half_url_incr_idx_send_weibo = {
		idx_key = "driveOnlineHoursPoint,driveOnlineMileagePoint,highwayTrafficMileagePoint,urbanTrafficMileagePoint",
		halfURL = string.gsub([[http://127.0.0.1/productList/drivingMileage/%s.amr,
		http://127.0.0.1/productList/fatigueDriving/%s.amr]],
		"[\n\t]*", "")
	},

	half_url_mult_idx_send_weibo = {
		idx_key = "4MilesAheadPositionTypeSet",
		halfURL = 'http://127.0.0.1/productList/POIRemind/%s.amr'
	},
}

OWN_ARGS = {
	-->> exact local whole
	app_task_forward = {
		app_uri = "a_d_xxxxxxxxxxx",
	},

	-->> alone
	full_url_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		fullURL = 'http://127.0.0.1/productList/xxxxx/xxxx.amr',
		carry_key = false,
	},

	unknow_url_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		redis_key = "arrivePoiFileUrl",
		idx_func = "arrive_poi",
		carry_key = false,
	},
	half_url_random_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_base = "30000",
		idx_max = 1,
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
		carry_key = false,
	},
	half_url_incr_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_base = "30000",
		idx_max = 1,
		idx_key = "driveOnlineMileagePoint",
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
		carry_key = false,
	},
	half_url_mult_idx_send_weibo = {
		app_key = '123456',
		secret = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa',
		interval = 300,
		level = 99,
		idx_key = "4MilesAheadPositionTypeSet",
		halfURL = 'http://127.0.0.1/productList/xxxxx/%s.amr',
		carry_key = "xxx",
	},
}

function app_task_forward( app_name ,accountid,info_table)
	local accountID = accountid
	local ok = judge.freq_filter(app_name, accountID)
	if not ok then
		only.log("D", "freq_filter false")
		return
	end

	local app_uri = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["app_task_forward"]["app_uri"]
    local path = app_uri
	local path = string.gsub(app_uri, "?.*", "")
	local app_srv = link["OWN_DIED"]["http"][ path ]
	local data = utils.compose_http_json_request(app_srv, app_uri, nil, info_table)
	http_api.http(app_srv, data, false)
end

--[[
local function replace_keyword( key )
local random_title = {'亲耐的', '可爱的', '热爱分享的', '最爱的', '牛逼轰轰的', '亲爱滴', '哒玲', '开车得心应手的'}
local change = {
title = function()
local num = #random_title
local random_num = utils.random_among(1, num)
return random_title[random_num]
end,

name = function()
local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
local ok, name_txt = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':nickname')
local ok, mirrtalk_number = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':mirrtalkNumber')
name_txt = name_txt or string.sub(mirrtalk_number, -4, -1)
name_txt = '道客' .. name_txt

return  name_txt
end,
}
return change[ key ]()
end

--  发送个人weibo
local function send_personal_weibo(wb)
local weibo_server = link["OWN_DIED"]["weibo"]["weibo_server"]
local post_body = utils.format_http_form_data(wb, 'abctoint')
local post_head = 'POST /sendPersonalWeibo HTTP/1.0\r\n' ..
'HOST:' .. weibo_server['host'] .. ":" .. tostring(weibo_server['port']) .. '\r\n' ..
'Content-Length:' .. tostring(#post_body) .. '\r\n' ..
'Content-Type:content-type:multipart/form-data;boundary=abctoint\r\n\r\n'

local post_data = post_head .. post_body
only.log('D',"post data : " .. post_data)
local ok, ret = supex.http(weibo_server['host'], weibo_server['port'], post_data, #post_data)
only.log('D', ret)
local body = string.match(ret, '{.*}')
local status, jo = pcall(cjson.decode, body)
if not status or tonumber(jo['ERRORCODE'])~=0 then
return false
end
return true
end

-- 发送集团weibo
local function send_group_weibo(wb)
local weibo_server = link["OWN_DIED"]["weibo"]["weibo_server"]
local post_body = utils.format_http_form_data(wb, 'abctoint')
local post_head = 'POST /sendGroupWeibo HTTP/1.0\r\n' ..
'HOST:' .. weibo_server['host'] .. ":" .. tostring(weibo_server['port']) .. '\r\n' ..
'Content-Length:' .. tostring(#post_body) .. '\r\n' ..
'Content-Type:content-type:multipart/form-data;boundary=abctoint\r\n\r\n'

local post_data = post_head .. post_body
only.log('D',"post data : " .. post_data)
local ok, ret = supex.http(weibo_server['host'], weibo_server['port'], post_data, #post_data)
only.log('D', ret)
local body = string.match(ret, '{.*}')
local status, jo = pcall(cjson.decode, body)
if not status or tonumber(jo['ERRORCODE'])~=0 then
return false
end

return true
end

-- 获取某类型的文本
local function get_redis_txt(code, position_type, tmp_key)
local position_id, interval, content
local key = string.format('%s:positionInfo', code)
local ok, position_info = redis_api.cmd('roadmap',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', key, position_type)
if not ok or position_info == nil then
return
end
only.log("D", position_info)
ok, info_tab = pcall(cjson.decode, position_info)

local now_time = os.time()
for i = 1, #info_tab do
position_id = info_tab[i]['positionID']
interval = info_tab[i]['interval']

if now_time > tonumber(interval) then
key = string.format('%d:details', position_id)
only.log("D", key)
ok, content = redis_api.cmd('roadmap',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
ok, content = pcall(cjson.decode, content)

if ok then
content['positionID'] = string.format('%d',position_id)
ok, content = pcall(cjson.encode, content)
local i, j = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', tmp_key, content)
end
end
end
end

-- 获取某时间段的adtalk
local function get_validity_time_adtlak(code, position_type, tmp_key, now_time, up_line)
local position_id, content
local key = string.format('%s:positionInfo', code)
local ok, position_info = redis_api.cmd('poi',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', key, position_type)
if not ok or position_info == nil then
return
end
only.log("D", position_info)
ok, info_tab = pcall(cjson.decode, position_info)

local flags,num_count = 0, 0
local now_date = os.date("%Y-%m-%d")
for i = 1, #info_tab do
position_id = info_tab[i]['positionID']

ad_key = position_id  .. ':applyInfo'
only.log("D", ad_key)
-- 判断该adtalk是否在有效期内
local ok, valid_info = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', ad_key, 'validDay')
only.log("D", valid_info)
ok, valid_day = pcall(cjson.decode, valid_info)
if not ok then
only.log("D", #valid_day)
end
if valid_day[1] > now_date or valid_day[2] < now_date then
break
end

-- 判断该adtalk是否在投放时段
ok, valid_info = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', ad_key, 'validTime')
if valid_info then
ok , valid_time = pcall(cjson.decode, valid_info)
for i = 1, #valid_time do
if valid_time[i][1] <  now_time and valid_time[i][2] > now_time then
flags = 1
break
end
end
else
flags = 1
end

local text_tab = {}
if flags == 1 and num_count <= up_line then
ok, content = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', ad_key, 'text')
local ok, adtalk_id = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', ad_key, 'adtalkID')
local str = string.format('[adtalkID:%s]', tostring(adtalk_id))
content = content .. str
text_tab['content'] = content

if ok then
text_tab['positionID'] = string.format('%d',position_id)
if string.match(code, '&') then
text_tab['geometryType'] = 1
else
text_tab['geometryType'] = 3
end
ok, text_tab = pcall(cjson.encode, text_tab)
only.log("D", text_tab)
local i, j = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', tmp_key, text_tab)
num_count = num_count + 1
end
end
end
end

-- 获取某类型的文本
local function get_redis_txt_info(code, position_type, tmp_key)
local position_id, interval, content
local key = string.format('%s:positionID', code)
local ok, position_info = redis_api.cmd('roadmap',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', key, position_type)
if not ok or position_info == nil then
return
end
only.log("D", position_info)
ok, info_tab = pcall(cjson.decode, position_info)

local now_time = os.time()
for i = 1, #info_tab do
position_id = info_tab[i]['positionID']
interval = info_tab[i]['validTime']

if now_time > tonumber(interval) then
key = string.format('%d:positionInfo', position_id)
only.log("D", key)
ok, content = redis_api.cmd('roadmap',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', key, position_type)
ok, content = pcall(cjson.decode, content)

if ok then
content['positionID'] = string.format('%d',position_id)
ok, content = pcall(cjson.encode, content)
local i, j = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', tmp_key, content)
end
end
end
end

-- 获取格网中某类型的文本
local function get_grid_content(accountID, lon, lat, position_type)
only.log("D", "position_type is :" .. position_type)

local key = string.format("%s:oldGrid", accountID)
local ok, old_grid = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
old_grid = old_grid or ''

local grid = string.format('%d&%d',tonumber(lon) * 100, tonumber(lat) * 100)

-- 判断用户在该格网提醒菜单是否已经初始化
local accountID 
if old_grid ~= grid then
only.log("D", "initialization menu")

-- 建一个临时集合
local tmp_key = string.format('tmpFile:%d:%s', position_type, accountID)

-- 获取某个格网内文本
get_redis_txt_info(grid, position_type, tmp_key)
local ok, new_tab = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'smembers',tmp_key)
only.log("D",'new_tab:' .. #new_tab)

-- 当用户没有accountID时，直接将所用文本都给他 
key = string.format("%s:%d:set", accountID, position_type)
if not ok or accountID == nil then

for i = 1, #new_tab do 
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, new_tab[i])
end
else
-- 对比总菜单和已读菜单,获取用户没有读取的文本
local read_key = string.format("%s:%d:readSet", accountID, position_type)
local ok, read_tab = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lrange', read_key, 0, -1)
ok, read_tab = pcall(cjson.encode, read_tab)
for i = 1, #new_tab do
if not string.match(read_tab,new_tab[i]) then
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, new_tab[i])
only.log("D", "+1")
end
end
end
-- 删除临时总菜单集合
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', tmp_key)

-- 更新触发的格网
key = string.format("%s:oldGrid", accountID)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', key, grid) 
end
-- 从未读列表中取一条文本
key = string.format("%s:%d:set", accountID, position_type)
local ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'spop', key)

if accountID == nil then
return text
end

-- 如果没有取到文本,就从已读列表中去一条,然后清空已读列表和格网
local read_key = string.format("%s:%d:readSet", accountID, position_type)
if text == nil then
only.log("D", "menu is nil")
ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'rpop', read_key)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', read_key)

return text
end

-- 如果已读文本不超过30，直接存入,如果已读文本有30个那么，将最早的文本移除，将最新的存入
local ok, read_len = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'llen', read_key)
only.log("D", "readSet :" .. read_len)
if ok and tonumber(read_len) ~= 30 then
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lpush', read_key, text)
else
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'rpop', read_key)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lpush', read_key, text)
end

text = string.gsub(text, '\n', '') -- delete \n

return text
end

-- 获取某个行政区内文本
local function get_surface_adtalk_txt(accountID, lon, lat, position_type, min_time_unit, up_line)
only.log("D", "position_type is :" .. position_type)

local key = string.format("%s:%d:countyCode", accountID, position_type)
local ok, old_county_code = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
old_county_code = old_county_code or ''
only.log("D", "old_county_code :" .. old_county_code)

local grid = string.format('%d&%d',tonumber(lon) * 100, tonumber(lat) * 100)
local ok, code_tab = redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hmget', grid, 'countyCode', 'cityCode', 'provinceCode')
county_code = code_tab[1]
city_code = code_tab[2]
province_code = code_tab[3]

local now_time = os.time() % (24 * 60 * 60)
only.log("D", "now_time :" .. now_time)
local ok, new_update_time = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get',position_type .. accountID .. ':newUpdateTime')
new_update_time = new_update_time or 0
only.log("D", "new_update_time :" .. new_update_time)

-- 判断用户在该地区的提醒菜单是否已经初始化
if (tonumber(county_code) ~= tonumber(old_county_code)) or (tonumber(now_time) > tonumber(new_update_time)) then
only.log("D", "initialization menu")

-- 建一个临时集合
local tmp_key = string.format('tmpFile:%d:%s', position_type, accountID)

-- 获取县级菜单
get_validity_time_adtlak(county_code, position_type, tmp_key, now_time, up_line)

-- 获取市级菜单
if city_code ~= nil and city_code ~= county_code then
get_validity_time_adtlak(city_code, position_type, tmp_key, now_time, up_line)
end

-- 获取省级菜单
if province_code ~= nil and province_code ~= city_code then
get_validity_time_adtlak(province_code, position_type, tmp_key, now_time, up_line)
end

-- 对比总菜单和已读菜单,获取用户没有读取的文本
-- 当用户没有accountID时，直接将所用文本都给他
key = string.format("%s:%d:set", accountID, position_type)
if not ok or accountID == nil then
for i = 1, #new_tab do
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, new_tab[i])
end
else
-- 对比总菜单和已读菜单,获取用户没有读取的文本
local ok, new_tab = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'smembers',tmp_key)
only.log("D",'new_tab:' .. #new_tab)

local read_key = string.format("%s:%d:readSet", accountID, position_type)
local ok, read_tab = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lrange', read_key, 0, -1)
ok, read_tab = pcall(cjson.encode, read_tab)

for i = 1, #new_tab do
if not string.match(read_tab,new_tab[i]) then
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, new_tab[i])
only.log("D", "+1")
end
end
end

-- 删除临时总菜单集合
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', tmp_key)

-- 更新异地提醒的行政区编号
key = string.format("%s:%d:countyCode", usrid, position_type)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', key, county_code)

-- 更新下次update时间
local new_update_time = now_time -  (now_time % min_time_unit)  + min_time_unit
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', position_type .. accountID .. ':newUpdateTime', new_update_time)
end
-- 从未读列表中取一条文本
key = string.format("%s:%d:set", accountID, position_type)
local ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'spop', key)
-- 如果没有取到文本,就从已读列表中去一条,然后清空已读列表和行政区编号
if accountID == nil then
return text
end

local read_key = string.format("%s:%d:readSet", accountID, position_type)
if text == nil then
only.log("D", "menu is nil")
ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'rpop', read_key)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', read_key)

key = string.format("%s:%d:countyCode", accountID, position_type)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', key)

return text
end

-- 如果已读文本不超过30，直接存入,如果已读文本有30个那么，将最早的文本移除，将最新的存入
local ok, read_len = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'llen', read_key)
only.log("D", "readSet :" .. read_len)
if ok and tonumber(read_len) ~= 30 then
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lpush', read_key, text)
else
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'rpop', read_key)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lpush', read_key, text)
end

text = string.gsub(text, '\n', '') -- delete \n

return text
end

local function get_grid_adtalk_txt(accountID, lon, lat, position_type, min_time_unit, up_line)
only.log("D", "position_type is :" .. position_type)

local key = string.format("%s:oldGrid", accountID)
local ok, old_grid = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
old_grid = old_grid or ''
only.log("D", "old_grid :" .. old_grid)

local grid = string.format('%d&%d',tonumber(lon) * 100, tonumber(lat) * 100)
only.log("D", "grid :" .. grid)
local now_time = os.time() % (24 * 60 * 60)
only.log("D", "now_time :" .. now_time)
local ok, new_update_time = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get',position_type .. accountID .. ':newUpdateTime')
new_update_time = new_update_time or 0
only.log("D", "new_update_time :" .. new_update_time)

-- 判断用户在该格网提醒菜单是否已经初始化
local accountID
if (old_grid ~= gridi) or (new_time > new_update_time) then
--if (old_grid == grid) or (new_time ~= new_update_time) then
only.log("D", "initialization menu")

-- 建一个临时集合
local tmp_key = string.format('tmpFile:%d:%s', position_type, accountID)

-- 获取某个格网内文本
get_validity_time_adtlak(grid, position_type, tmp_key, now_time, up_line)
local ok, new_tab = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'smembers',tmp_key)
only.log("D",'new_tab:' .. #new_tab)

-- 当用户没有accountID时，直接将所用文本都给他
key = string.format("%s:%d:set", accountID, position_type)
if not ok or accountID == nil then
for i = 1, #new_tab do 
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, new_tab[i])
end
else 
-- 对比总菜单和已读菜单,获取用户没有读取的文本
local read_key = string.format("%s:%d:readSet", accountID, position_type)
local ok, read_tab = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lrange', read_key, 0, -1)
ok, read_tab = pcall(cjson.encode, read_tab)

for i = 1, #new_tab do
if not string.match(read_tab,new_tab[i]) then
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, new_tab[i])
only.log("D", "+1")
end
end
end

-- 删除临时总菜单集合
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', tmp_key)

-- 更新触发的格网
key = string.format("%s:oldGrid", accountID)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', key, grid)

-- 更新下次update时间
local new_update_time = now_time -  (now_time % min_time_unit)  + min_time_unit
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', position_type .. accountID .. ':newUpdateTime', new_update_time)
end
-- 从未读列表中取一条文本
key = string.format("%s:%d:set", accountID, position_type)
local ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'spop', key)

-- 如果没有取到文本,就从已读列表中去一条,然后清空已读列表和格网
if accountID == nil then
return text
end

local read_key = string.format("%s:%d:readSet", accountID, position_type)
if text == nil then
only.log("D", "menu is nil")
ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'rpop', read_key)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', read_key)

return text
end

-- 如果已读文本不超过30，直接存入,如果已读文本有30个那么，将最早的文本移除，将最新的存入
local ok, read_len = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'llen', read_key)
only.log("D", "readSet :" .. read_len)
if ok and tonumber(read_len) ~= 30 then
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lpush', read_key, text)
else
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'rpop', read_key)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'lpush', read_key, text)
end

text = string.gsub(text, '\n', '') -- delete \n

return text
end

-- 获取某个行政区内文本
local function get_off_site_content(accountID, lon, lat, position_type)
only.log("D", "position_type is :" .. position_type)

local key = string.format("%s:%d:countyCode", accountID, position_type)
local ok, old_county_code = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', key)
old_county_code = old_county_code or ''
only.log("D", "old_county_code :" .. old_county_code)

local grid = string.format('%d&%d',tonumber(lon) * 100, tonumber(lat) * 100)
local ok, code_tab = redis_api.cmd('mapGridOnePercent',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hmget', grid, 'countyCode', 'cityCode', 'provinceCode')
county_code = code_tab[1]
city_code = code_tab[2]
province_code = code_tab[3]

-- 判断用户在该地区的提醒菜单是否已经初始化
if county_code ~= old_county_code then
only.log("D", "initialization menu")

-- 建一个临时集合
local tmp_key = string.format('tmpFile:%d:%s', position_type, accountID)

-- 获取县级菜单
get_redis_txt(county_code, position_type, tmp_key)

-- 获取市级菜单
if city_code ~= nil and city_code ~= county_code then
get_redis_txt(city_code, position_type, tmp_key)
end

-- 获取省级菜单
if province_code ~= nil and province_code ~= city_code then
get_redis_txt(province_code, position_type, tmp_key) 
end

-- 对比总菜单和已读菜单,获取用户没有读取的文本
key = string.format("%s:%d:set", accountID, position_type)
local read_key = string.format("%s:%d:readSet", accountID, position_type)
local ok, tab_num = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sdiffstore', key, tmp_key, read_key)
only.log("D", "the personal menu is :" .. tab_num)

-- 删除临时总菜单集合
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', tmp_key)

-- 更新异地提醒的行政区编号
key = string.format("%s:%d:countyCode", accountID, position_type)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'set', key, county_code)
end
-- 从未读列表中取一条文本
key = string.format("%s:%d:set", accountID, position_type)
local ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'spop', key)
-- 如果没有取到文本,就从已读列表中去一条,然后清空已读列表和行政区编号
if text == nil then
only.log("D", "menu is nil")
local read_key = string.format("%s:%d:readSet", accountID, position_type)
ok, text = redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'spop', read_key)
redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', read_key)

key = string.format("%s:%d:countyCode", accountID, position_type)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'del', key)
return text
end

-- 将取出的content加入到已读的列表中
key = string.format("%s:%d:readSet", accountID, position_type)
redis_api.cmd('app',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'sadd', key, text)

text = string.gsub(text, '\n', '') -- delete \n

return text
end

-- 根据选择的函数名，调用相应的函数
local function get_content( key, accountID, position_type, lon, lat, min_time_unit, up_line)
local get_function = {
get_off_site_content = function()
local text = get_off_site_content(accountID, lon, lat, position_type)
return text
end,
get_grid_content = function()
local text = get_grid_content(accountID, lon, lat, position_type)
return  text
end,
get_grid_adtalk_txt = function()
local text = get_grid_adtalk_txt(accountID, lon, lat, position_type, min_time_unit, up_line)
return text
end,
get_surface_adtalk_txt = function()
local text = get_surface_adtalk_txt(accountID, lon, lat, position_type, min_time_unit, up_line)
return text
end,
}

return get_function[key]()
end
-- 从缓存中获取文本weibo
function unknow_url_send_weibo( app_name )
only.log("D", "unknow_url_send_weibo work")

local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
local lon = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
local lat = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
local position_type = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["position_type"]  
local interval = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["interval"]
local allow_yes = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["yes"]
local level = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["level"]
local app_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["app_key"]
local fun_type = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["fun_type"]
local text_fmt = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["text"]
local min_time_unit = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["min_time_unit"]
local up_line = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["unknow_url_send_weibo"]["up_line"]


-- 检查播放的频次
local ok = judge.freq_filter(app_name, accountID)
if not ok then
only.log("D", "freq_filter false")
return
end

local text = get_content(fun_type, accountID, position_type, lon, lat, min_time_unit, up_line) 
if text == nil then
judge.freq_regain( app_name, accountID)
only.log("D", "text in redis is nil")
return
end

ok, text = pcall(cjson.decode, text)
txt = string.format(text_fmt, text['content'])
local wb = {
appKey = app_key,
text = txt,
UID = accountID,
IDType = 1,
interval = interval,
allowYes = allow_yes,
level = level,
}
if carry_key then
local ok, info = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':' .. carry_key)
if ok and info then
local ok,jo = pcall(cjson.decode, info)
if ok and jo then
for k,v in pairs(jo) do
wb[ k ] = v
end
end
end
end
if text['geometryType'] then
wb['geometryType'] = text['geometryType']
wb['positionID'] = text['positionID']
end
local ok, secret = redis_api.cmd('public',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', wb['appKey'] .. ':appKeyInfo', 'secret')
wb['sign'] = utils.gen_sign(wb, secret)
wb['text'] = utils.url_encode(wb['text'])

ok = send_personal_weibo(wb)
if not ok then
judge.freq_regain( app_name, accountID )
only.log("D", "send weibo failed")
end
end
]]--


function full_url_send_weibo( app_name )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local lon       = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat       = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	local speed     = APP_POOL["OUR_BODY_TABLE"]["speed"][1]
	local direction = APP_POOL["OUR_BODY_TABLE"]["direction"][1]
	local altitude  = APP_POOL["OUR_BODY_TABLE"]["altitude"][1]
	local interval = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["full_url_send_weibo"]["interval"]
	local level = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["full_url_send_weibo"]["level"]
	local app_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["full_url_send_weibo"]["app_key"]
	local secret = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["full_url_send_weibo"]["secret"]
	local fileURL = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["full_url_send_weibo"]["fullURL"]
	local carry_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["full_url_send_weibo"]["carry_key"]

	local ok = judge.freq_filter(app_name, accountID)
	if not ok then
		only.log("D", "freq_filter false")
		return
	end
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
	if carry_key then
		local ok, info = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':' .. carry_key)
		if ok and info then
			local ok,jo = pcall(cjson.decode, info)
			if ok and jo then
				for k,v in pairs(jo) do
					wb[ k ] = v
				end
			end
		end
	end

	if not wb["senderDirection"] then
		wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	end
	if not wb["senderSpeed"] then
		wb["senderSpeed"]  =  string.format('[%s]',speed and math.ceil(speed) or 0)
	end
	if not wb["senderAltitude"]  and  altitude then
		wb["senderAltitude"]  =  altitude
	end
	if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
		wb["senderLongitude"] = lon
		wb["senderLatitude"] = lat
	end

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		judge.freq_regain( app_name, accountID )
		only.log("E", "send weibo failed : " .. err)
	end
end

function half_url_random_idx_send_weibo( app_name )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local app_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["app_key"]
	local secret = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["secret"]
	local interval = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["interval"]
	local level = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["level"]

	local idx_base = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["idx_base"]
	local idx_max = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["idx_max"]
	local halfURL = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["halfURL"]
	local carry_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_random_idx_send_weibo"]["carry_key"]

	local idx = utils.random_among(idx_base + 1, idx_base + idx_max)
	local fileURL = string.format(halfURL, idx)

	local ok = judge.freq_filter(app_name, accountID)
	if not ok then
		only.log("D", "freq_filter false")
		return
	end
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
	if carry_key then
		local ok, info = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':' .. carry_key)
		if ok and info then
			local ok,jo = pcall(cjson.decode, info)
			if ok and jo then
				for k,v in pairs(jo) do
					wb[ k ] = v
				end
			end
		end
	end

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		judge.freq_regain( app_name, accountID )
		only.log("E", "send weibo failed : " .. err)
	end
end

function half_url_incr_idx_send_weibo( app_name )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local lon       = APP_POOL["OUR_BODY_TABLE"]["longitude"][1]
	local lat       = APP_POOL["OUR_BODY_TABLE"]["latitude"][1]
	local speed     = APP_POOL["OUR_BODY_TABLE"]["speed"][1]
	local direction = APP_POOL["OUR_BODY_TABLE"]["direction"][1]
	local altitude  = APP_POOL["OUR_BODY_TABLE"]["altitude"][1]
	local app_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["app_key"]
	local secret = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["secret"]
	local interval = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["interval"]
	local level = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["level"]

	local idx_base = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["idx_base"]
	local idx_max = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["idx_max"]
	local idx_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["idx_key"]
	local halfURL = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["halfURL"]
	local carry_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_incr_idx_send_weibo"]["carry_key"]

	local ok, idx = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', string.format("%s:%s", accountID, idx_key))
	if (not ok) or (not idx) then
		return
	end
	idx = tonumber(idx)
	if idx > idx_max then
		idx = idx_max
	end
	idx = idx_base + idx
	local fileURL = string.format(halfURL, idx)

	local ok = judge.freq_filter(app_name, accountID)
	if not ok then
		only.log("D", "freq_filter false")
		return
	end
	local wb = {
		multimediaURL = fileURL,
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
	if carry_key then
		local ok, info = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'get', accountID .. ':' .. carry_key)
		if ok and info then
			local ok,jo = pcall(cjson.decode, info)
			if ok and jo then
				for k,v in pairs(jo) do
					wb[ k ] = v
				end
			end
		end
	end

	if not wb["senderDirection"] then
		wb["senderDirection"] = string.format('[%s]', direction and math.ceil(direction) or -1)
	end
	if not wb["senderSpeed"] then
		wb["senderSpeed"]  =  string.format('[%s]', speed and math.ceil(speed) or 0)
	end
	if not wb["senderAltitude"]  and  altitude then
		wb["senderAltitude"]  =  altitude
	end
	if lon and lat and not wb["senderLongitude"] and not wb["senderLatitude"] then
		wb["senderLongitude"] = lon
		wb["senderLatitude"] = lat
	end

	local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
	local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
	if not ok then
		judge.freq_regain( app_name, accountID )
		only.log("E", "send weibo failed : " .. err)
	end
end

function half_url_mult_idx_send_weibo( app_name )
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local interval = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["interval"]
	local level = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["level"]
	local app_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["app_key"]
	local secret = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["secret"]

	local idx_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["idx_key"]
	local halfURL = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["halfURL"]
	local carry_key = APP_CONFIG_LIST["OWN_LIST"][app_name]["work"]["half_url_mult_idx_send_weibo"]["carry_key"]

	local ok, array = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'smembers', accountID .. ':' .. idx_key)
	if (not ok) or (not array) then
		return
	end
	local wb = {
		receiverAccountID = accountID,
		level = level,
		interval = interval,
		senderType = 2,
	}
	for _,idx in pairs(array or {}) do
		wb["multimediaURL"] = string.format(halfURL, idx)
		if carry_key then
			local ok, info = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', accountID .. ':' .. carry_key, tostring(idx))
			if ok and info then
				local ok,jo = pcall(cjson.decode, info)
				if ok and jo then
					for k,v in pairs(jo) do
						wb[ k ] = v
					end
				end
			end
		end

		local ok = judge.freq_filter(app_name, accountID)
		if not ok then
			only.log("D", "freq_filter false")
			return
		end

		local server = link["OWN_DIED"]["http"]["weiboapi/v2/sendMultimediaPersonalWeibo"]
		--for i, v in pairs(wb or {}) do
		--    only.log("D", string.format("[%s:%s]", i, v))
		--end
		local ok,err = weibo.send_weibo( server, "personal", wb, app_key, secret )
		if not ok then
			judge.freq_regain( app_name, accountID )
			only.log("E", "send weibo failed : " .. err)
		end
	end
end

