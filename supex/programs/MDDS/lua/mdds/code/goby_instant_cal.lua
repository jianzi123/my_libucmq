local only                 = require('only')
local gosay                = require('gosay')
local msg                  = require('msg')
local safe                 = require('safe')
local utils                = require('utils')
local json                 = require('cjson')
local map                  = require('map')
local scan                 = require('scan')
local socket               = require('socket')
local pool                 = require('pool')
local http_short_api       = require('http_short_api')
local redis_pool_api       = require('redis_pool_api')

local fun_point_match_road = require('fun_point_match_road')

local app_lua_lru_cache_set_value = _G.app_lua_lru_cache_set_value
local app_lua_lru_cache_get_value = _G.app_lua_lru_cache_get_value
local app_lua_lru_cache_remove_value = _G.app_lua_lru_cache_remove_value

module('goby_instant_cal',package.seeall)



-->>key is accountID:instantDriveInfo 
local function get_last_record(key) 
    	local res, ok 
	if not key then 
	    	only.log('E',"get_last_record's key is nil ")
	else
	    	ok, res = app_lua_lru_cache_get_value(key)
	end 

	if not res or not ok then 
	    	only.log('E',"the result of get_last_record is nil ")
		return nil 
	else 
	    	only.log('D',string.format("the result of get_last_record is %s",res))
		return res 
	end
	 
end 

-->>value is store with json format : drive_time stop_time  last_sum_mile  last_speed  start_position end_position tokencode  time  longitude latitude 
local function set_last_record(key,value,val_len)
    	local ok 
	if not key or not value then 
	    	only.log('E',"set_last_record's key or value is nil")
		return false 
	else 
	    	ok,_ = app_lua_lru_cache_set_value(key,value,val_len)
		--the persistence of each calculation result (where ?) 
	end

	if not ok then 
	    	only.log('E',"set_last_record fail")
	    	return false 
	else 
	    	only.log('D',"set_last_record sucess")
	    	return true 
	end 
end 

local function remove_last_record(key)
    	local ok 
	if not key then 
	    	only.log('E',"remove_last_record's key is nil")
		return false 
	else 
	    	ok, _ = app_lua_lru_cache_remove_value(key)
	end 

	if not ok then 
	    	only.log('E',"remove_last_record fail")
	    	return false 
	else 
	    	only.log('D',"remove_last_record sucess")
	    	return true 
	end 

end 

local function store_most_recent_record_to_redis(accountID, key,value_tab)
    -- only store 
    -->>value is store with json format : drive_time stop_time max_speed average_speed last_sum_mile TODO-->> start_position end_position 
    	if not key or not value_tab then 
		only.log('E',"store_most_recent_record_to_redis's key or value_tab is nil")
    	else       				 
	    	local ok ,_ = redis_pool_api.cmd("mapGPSData", accountID or "", 'HMSET',key,'driveTime',value_tab['drive_time'],
									  'stopTime',value_tab['stop_time'],
									  'maxSpeed',value_tab['max_speed'],
									  'averageSpeed',value_tab['average_speed'],
									  'lastSumMile',value_tab['last_sum_mile'],
									  'timeStamp',value_tab['time'],
									  'path',value_tab['path'])
		only.log('D',"store_most_recent_record_to_redis sucess")
            	if not ok then 
			only.log('E',"store_most_recent_record_to_redis fail")
	    	end 
	end  
end


--TODO  
local function persist_each_calculation_result(key,value)
    	
end


function gaptime_of_two_data_packet(accountID,current_packet_time)
    	if not accountID or not current_packet_time then 
	    	only.log('E',"cal gaptime of two packet 's accountID or current_packet_time is nil ")
	end 
	local res = get_last_record(accountID .. ":instantDriveInfo")
	only.log('D',string.format('get_last_record:%s',res))
	if  res then 
	    	local _,j_res = pcall(json.decode,res) 
	    	return tonumber(current_packet_time) - tonumber(j_res['time']) ,j_res 
	else 
	    	return 0 , {} 
	end 
end
function is_poweroff(last_record,current_tokencode)
    	if not next(last_record) or not current_tokencode then 
		only.log('E',"cal user is poweroff 's last_record or current_tokencode is nil ")
	end
	if not last_record['tokencode'] then 
	    	return false
	end 
	if last_record['tokencode']==current_tokencode then 
		return false 
	else 
		--save other place (mongodb)
		
		app_lua_lru_cache_remove_value(accountID .. ":instantDriveInfo")
		return true 
	end 
	    
end 

function get_location (direction ,longitude ,latitude)
    	if direction and longitude and latitude then 
    		local ok,res = fun_point_match_road.entry(direction,longitude,latitude)
		if not ok or not res then 
		    	only.log('E',"get location failed ")
		else 
		    	return res['roadID']
		end 
	else 
	    	only.log('E',"input location parameter error")
		return nil 
	end 
end

function calculate_path (last_record ,current_packet)
    	if not last_record or not next(last_record) then 
	    	last_record_path = ""
	else 
	    	last_record_path = last_record['path']
	end 

    	local location_point = get_location(current_packet['direction'][3],current_packet['longitude'][3],current_packet['latitude'][3])
	if location_point then 
	    	return last_record_path .. location_point .."|"	 
	else   
	    	return last_record_path 
	end 
end

function calculate_speed (data)
        local averageSpeed =0 
        local sumSpeed =0
        local maxSpeed = 0
        for i=1,#data do 
            	sumSpeed= sumSpeed + data[i]
             	if data[i] >maxSpeed then 
                	 maxSpeed=data[i]
             	end 
         end
         averageSpeed = sumSpeed/5 
         return averageSpeed ,maxSpeed
end

function calculate_milage (last_record, current_packet, gap_time)
	local last_lon ,last_lat ,last_sum_mile ,last_speed 
    	if not last_record or not next(last_record) then 
		last_lon ,last_lat ,last_sum_mile ,last_speed = current_packet['longitude'][1],current_packet['latitude'][1],0,0
	else 
        	last_lon ,last_lat ,last_sum_mile ,last_speed = last_record['longitude'],last_record['latitude'],last_record['last_sum_mile'],last_record['last_speed']
	end 
        local data = current_packet['speed']
	local cur_dist =  (last_speed/2+data[1]+data[2]+data[3]+data[4] + data[5]/2)/3.6
        if gap_time ==0 or gap_time <6  then 
            	last_sum_mile = last_sum_mile + cur_dist 
        end 
        if gap_time >=6 and gap_time< 11 then                   
                last_sum_mile =last_sum_mile + ((last_speed+data[1]/2)*gap_time )/3.6 + cur_dist
        else 
	    	if gap_time >=11 then 
                	last_sum_mile = last_sum_mile + map.get_two_point_dist(last_lat,last_lon,current_packet['latitude'][1],current_packet['longitude'][1])

                end 
        end 
                                                                                                               
        return last_sum_mile 
end
     
function vehicle_stop_time (last_record, current_packet, gap_time)
	if not last_record or not next(last_record) then 
	    	stop_time ,drive_time = 0,0
	else 
    		stop_time ,drive_time = last_record['stop_time'] ,last_record['drive_time']
	end 
	local current_dir = current_packet['direction']


	if gap_time >=6 then 
	    	if current_dir[1] ~= -1 then 
		    	drive_time = drive_time + gap_time
		else 
		    	stop_time = stop_time + gap_time 
		end 
	end 	    	
	for i=1, #current_dir do 
	    	if current_dir[i]==-1 then 
		    	stop_time = stop_time+1
		else 
		    	drive_time = drive_time +1 
		end
	end
	    	    	
	return stop_time ,drive_time 

end 

function handle()  
    	local req_body = pool['OUR_BODY_TABLE']
    	local args = {} 
    	args['accountID'] = req_body['accountID']
    	args['longitude'] = req_body['longitude']
    	args['latitude'] = req_body['latitude']
    	args['direction'] = req_body['direction']
    	args['GPSTime'] = req_body['GPSTime']
    	args['speed'] = req_body['speed']
    	args['tokencode'] = req_body['tokencode']
	only.log('D',string.format("args is %s",scan.dump(args)))

	local stex_time = socket.gettime()

	-->> get gaptime_of_two_data_packet 
	local gap_time ,last_record = gaptime_of_two_data_packet(args['accountID'],args['GPSTime'][1])
	-->> check user packet right
	if gap_time < 0 then 
	    	only.log('E'," gap time is negative" )
		return 
	end 

	-->> check user poweroff 
	local flags= false  
	for i=1,#args['tokencode'] do 
		flags = is_poweroff(last_record,args['tokencode'][i])
		if flags == true then 
		    	break 
		end 
	end 
	-->> calculate current user drive info 
	local res_table ={}
	if not flags then 
		-->>cal speed 
	        res_table['average_speed'] ,res_table['max_speed'] = calculate_speed(args['speed'])

		-->>cal mileage 
		res_table['last_sum_mile'] = calculate_milage(last_record,args,gap_time)

		-->>cal time 
		res_table['stop_time'] ,res_table['drive_time'] = vehicle_stop_time(last_record,args,gap_time)
		-->>cal path 
		res_table['path'] = calculate_path(last_record,args)
		-->>add other info 
		res_table['last_speed'] = args['speed'][5]
		res_table['tokencode'] = args['tokencode'][5]
		res_table['time'] = args['GPSTime'][5]
		res_table['longitude'] = args['longitude'][5] 
		res_table['latitude'] = args['latitude'][5]

		-->> store_to_cache
		only.log('D',string.format('result is %s',scan.dump(res_table)))
		local ok , retv =pcall( json.encode,res_table) 
		if not ok or not retv then 
		    	only.log('E',"encode table that store to cache fail ")
		else
			store_most_recent_record_to_redis(args['accountID'], args['accountID']..":instantDriveInfo",res_table)
		    	set_last_record(args['accountID']..":instantDriveInfo",retv,#retv)
		end 
	end 

	only.log("D",string.format("excuate time is %.6f",socket.gettime()-stex_time))

end 
