local only                 = require('only')
local gosay                = require('gosay')
local msg                  = require('msg')
local safe                 = require('safe')
--local app_utils            = require('app_utils')
local utils                = require('utils')
local json                 = require('cjson')
--local ngx                  = require('ngx')
local map                  = require('map')
local scan                 = require('scan')
local socket               = require('socket')
local pool                 = require('pool')
--local map_utils            = require('map_utils')
local http_short_api       = require('http_short_api')
local redis_pool_api       = require('redis_pool_api')

local fun_point_match_road = require('fun_point_match_road')
local cal_poi_by_line      = _G.cal_poi_by_line
local cal_front_road_by_road = _G.cal_front_road_by_road

local MAX_ANGLE              = 30     --最大误差角
local G_line_id_set          = {}

module('goby',package.seeall)

function sub_direction(ang1, ang2)
    	local sub_angle = math.abs(tonumber(ang1)-tonumber(ang2))
    	return  sub_angle < 180 and sub_angle or 360-sub_angle
end

function check_parameter(args)
	-->>check accountID
	if not args['accountID']  then 
		only.log('E' , "accountID is nil")
		return false 	
	else
		local id_len = string.len(args['accountID'])
		if id_len  ~= 10 and id_len ~= 15 then 
			return false 
		end
	end
	-->>check longitude 
	local longitude = tonumber(args['longitude'])
	if (not longitude) or (longitude > 137.8347 or longitude < 72.004) then
		only.log('E',"longitude data error")
		return false 
	end
	-->>check latitude 
	local latitude = tonumber(args['latitude'])
	if (not latitude) or (latitude > 55.8271 or longitude < 0.8293) then
		only.log('E',"latitude data error")
		return false 
	end
	-->>check direction
	local direction = tonumber(args['direction'])
	if not direction then
		args['direction'] = -1
	end
	return  true 
end

function get_location(args)
	-->>get user's location(line_id,road_id)
	local ok , result = fun_point_match_road.entry(args['direction'],args['longitude'],args['latitude'],args['accountID'])
	if not ok then
		only.log('E','get lineid failed')
		return nil ,nil
	else 
		return result['roadID'] , result['lineID']
	end
end
 
function poi_info_store_to_redis (account_id,json_poi_info)
    	if  account_id and json_poi_info then
		local ok , _ = redis_pool_api.cmd('MapPersonalFrontPOI', account_id, 'set',account_id .. ":frontPOI",json_poi_info)
		if not ok then
	    		only.log('E',string.format("set %s:frontPOI value:%s",account_id,json_poi_info))
	    		return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'],'redis')
		else
		    	return gosay.resp_msg(msg['MSG_SUCCESS_WITH_RESULT'],'success store personal poi ')
    	        end
	else
	    	only.log('E',"store personal poi with account_id or poi info is nil")
		
	end
end

function get_standard_poi_id(digital_poi_id)
    	if tonumber(digital_poi_id)==nil then 
		only.log('E',"poi id covert digital to standard failed ")
		return nil
    	end
    	local init_tab={'0000000','000000','00000','0000','000','00','0',''}
    	local len = get_len_of_num(digital_poi_id)
    	return string.format("P%s%d",init_tab[len],digital_poi_id)
end

function get_len_of_num( number)
    	local i=1;
    	while math.floor(number/10) ~= 0 do
		i=i+1
		number = math.floor(number/10)
    	end
    	return i
end
function get_poi(account_id)
    	

    	local poi_set = cal_poi_by_line(G_line_id_set)

    	local tab_poi_set ={}
    	for i=1 ,#poi_set,6 do 
    		local temp = {}
		local poiId = get_standard_poi_id(tonumber(poi_set[i]))
		temp['type']= poi_set[i+1]
		temp['longitude']= poi_set[i+2]/1000000
		temp['latitude']= poi_set[i+3]/1000000
		temp['angle1']= poi_set[i+4]-360
		temp['angle2']= poi_set[i+5]-360
		if poiId then 
			tab_poi_set[tostring(poiId)]= temp
		end
    	end

	local ok,json_poi_info = pcall(json.encode,tab_poi_set)
	if not ok then 
	    	only.log('E',"tab_poi_set encode json error")
		return gosay.resp_msg(msg['MSG_ERROR_REQ_CODE'],'encode json')
	end
	only.log('D',string.format("tab_poi_set json :%s",json_poi_info))
	
	poi_info_store_to_redis(account_id,json_poi_info)


end
-->>process front line info

function decode_front_line_id(str)
	only.log('D','DECODE START')
	
	if not str then 
		return nil  
	end 

	local f_str_tab  = utils.str_split(str,';')

	local res_tab = {}
	for k1,v1 in pairs(f_str_tab) do 
		local s_str_tab = utils.str_split(v1,'$')
		if type(s_str_tab)== "table" and next(s_str_tab) then 
			res_tab[s_str_tab[1]]= s_str_tab[2]
		else 
		  	return nil 
		end
	end
	--only.log('D','FIRST DECODE')

	for k2,v2 in pairs(res_tab) do 
		local tem = utils.str_split(v2,',')
		if type(tem)=='table' and  next(tem) then 
			res_tab[k2]= tem 
		else
			return nil 
		end 
			
	end

	--only.log('D','SECOND DECODE')

	local final_tab ={}
	for k3,v3 in pairs(res_tab)do 
		final_tab[k3]={}
		for k4,v4 in pairs(v3) do 
			local tem = utils.str_split(v4,':')
			if type(tem)=='table' and  next(tem) then 
				table.insert(final_tab[k3],tem)
			else 
				return  nil 
			end 
		end
	end
	--only.log('D','THIRD DECODE')
	return final_tab 
end
-->>add new way to get line id
function get_line_id_set(line_id,direction)
	if not line_id or not tonumber(direction)then 
		only.log('E','get line_id is nil or direction is not number')
		G_line_id_set ={}
		return  
	end
	local front_line_id_info = get_front_line_id_info(line_id)
	if not front_line_id_info then 
		only.log('E','get front_line_id_info is nil')
		G_line_id_set ={}
		return 
	end

	--only.log('D','before start decode ')

	local line_tab = decode_front_line_id(front_line_id_info)
	if not line_tab or not next(line_tab) then 
		only.log('E','get line table is nil ')
		G_line_id_set ={}
		return 
	end
	--only.log('D','add elem G-line_id set')
	local idx=-2
	for k,v in pairs(line_tab) do 
		if sub_direction(tonumber(k),direction)<=MAX_ANGLE then
			idx = k 
		end
	end
	if idx == -2 then  G_line_id_set={} return end 

	for k2, v2 in pairs(line_tab[idx]) do			 
		for i=0,v2[3]-1 do 
			table.insert(G_line_id_set,tostring(v2[1]+i*v2[2]))
		end
	end 
	--only.log('D','finish add G-line-id set')
end 
-->>get front line id from redis
function get_front_line_id_info (key) 
	if not key then 
		only.log('E',"get front line id info 's key is nil")
		return nil
	end
	local ok,res = redis_pool_api.cmd('MapLineInRoad', "", 'get',key .. ':frontIds') 
	if not ok then 
		return gosay.resp_msg(msg['MSG_DO_REDIS_FAILED'],'redis')
	else
		only.log('D',string.format('line_id_info:%s',res))
		return res
	end
end

local function checkparametr(args)
	local longitude =tonumber( args['longitude']) 
	if not longitude then return false  end

	local latitude =tonumber( args['latitude']) 
	if not latitude  then return false end

	local time =tonumber( args['time']) 
	if not time then return false end

	if not args['accountID'] then return false end

	return true 
end

local function entry(args)
	-->>check parameter
	if not checkparametr(args) then
		only.log('E',"get parameter error")
		return 
	end 
	-->> update_grid_account_id_info	
	local lon = args["longitude"]
	local lat = args["latitude"]
	local accountID = args["accountID"]
	local new_gridID = string.format("%d&%d", math.floor(lon*100), math.floor(lat*100))
	args["gridID"] = new_gridID
	-->> set info
	local accountkey = accountID .. ":info"
	local ok,old_gridID = redis_api.cmd("mapGPSData", "HGET", accountkey, "gridID")
	local ok = redis_api.cmd("mapGPSData", 'hmset', accountkey,
			"longitude", lon,
			"latitude", lat,
			"time", args["time"],
			"gridID", new_gridID)
	
	-->> set accountIds
	local new_grid_key = new_gridID .. ":accountIds";
	if not old_gridID then
		redis_api.cmd("mapGPSData", 'SADD', new_grid_key, accountID)
	else
		if old_gridID ~= new_gridID then
			local old_grid_key = old_gridID .. ":accountIds"
			redis_api.cmd("mapGPSData", 'SMOVE', old_grid_key, new_grid_key, accountID)
		end
	end

end
function handle()
	local req_body = pool["OUR_BODY_TABLE"]
	local args = {}
	args['accountID']=req_body['accountID']
	args['longitude']=req_body['longitude'][1]
	args['latitude']=req_body['latitude'][1]
	args['direction']=req_body['direction'][1]
	
	local account_id = args['accountID']
         
	only.log('D',string.format("[args :%s]",scan.dump(args)))
		
	local starttime = socket.gettime()
	-->>check parameter
    	local ok = check_parameter(args)
	if not ok then 
	    	only.log('E','check parameter failed')
		return gosay.resp_msg(msg['MSG_ERROR_REQ_ARG'],'paramter')
	end

	-->>get start position  line_id
	local _ , line_id  = get_location(args)
	only.log('D',string.format('[each get location(%s) time :%f]',line_id ,socket.gettime()-starttime))

	-->>get line_id set 
	get_line_id_set(line_id,args['direction'])
	only.log('D',string.format("line id set :%s",scan.dump(G_line_id_set)))
	local getlineTime = socket.gettime()
	only.log('D',string.format('[get line_id set time :%f]',getlineTime-starttime))

	-->> do poi
	get_poi(account_id)
	only.log('D',string.format('[get poi set time :%f]',socket.gettime()-getlineTime))
	G_line_id_set = {}
	-->>other task: add_grid_info 
	args['time']=req_body['GPSTime'][1]
	entry(args)
	only.log('D',"add_grid_info has been excuted")
end

