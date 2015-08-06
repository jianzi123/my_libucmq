local only	= require("only")
local redis_api = require("redis_pool_api")
local socket	= require("socket")
local ALIAS_LIST= require("ALIAS_LIST")

module("monitor", package.seeall)

--初始化
local MONITOR_POOL = {
	PERIOD_TIMESTAMP	= 0,			--周期开始时间
	MODULE_TIMESTAMP	= 0,			--模块执行起始时间
	MONITOR_DETAIL_LIST	= {}
}

--定义补充模块中文名称
local ALIAS_LOCAL = {
	['e_power_on']		='开机',
	['e_power_on_status']	='开机状态',
	['e_power_off_status']	='关机状态',
	['e_power_off']		='关机',
	['e_power_on_weibo']	='开机消息提醒',
	['e_newbie_guide']	='新手教程',
	['l_f_road_traffic']	='路况',
	['l_collect_gps']	='里程计数',
	['l_collect']		='在线时长',
}



--获取时间戳(精确到ms)
local function get_time_stamp()
	return socket.gettime()
end

--监控初始化
function mon_init()
	MONITOR_POOL["PERIOD_TIMESTAMP"] = get_time_stamp()
	only.log("D", "MONITOR:初始化全局table")
end

local function refresh(name)
	MONITOR_POOL["MONITOR_DETAIL_LIST"][name] = {
			count 		= 0,
			match_grow 	= 0,
			work_grow	= 0,
			match_avge	= 0,
			work_avge	= 0,
			match_peak	= 0,
			work_peak	= 0,
			match_vally	= 0,
			work_vally	= 0,
	}
end


--初始化record table
function mon_start(name, flag)
	only.log("D", "MONITOR:进入START")
	local monitor_record = MONITOR_POOL["MONITOR_DETAIL_LIST"]
	--如果子表为空或者不存在,刷新子表
--	if table.maxn((monitor_record[name] or {})) == 0 then
	if not monitor_record[name] or not monitor_record[name]["count"] then
		only.log("D", "MONITOR:REFRESH")
		refresh(name)
	end
	monitor_record[name]["count"] = monitor_record[name]["count"] + 1
	only.log("D", "MONITOR:COUNT +1 |" .. tostring(monitor_record[name]["count"]))
end




--统计match次数
function mon_bef_match(name, flag)
	only.log("D", "进入MATCH")
	if flag then
		local model_monitor = MONITOR_POOL["MONITOR_DETAIL_LIST"][name]
		model_monitor["match_grow"] = model_monitor["match_grow"] + 1
		only.log("D","MONITOR:MATCH +1 |" .. tostring(model_monitor["match_grow"]))

		MONITOR_POOL["MODULE_TIMESTAMP"] = get_time_stamp()
	end
end

--为match记时
function mon_end_match(name,flag)
	if flag then
		local model_monitor = MONITOR_POOL["MONITOR_DETAIL_LIST"][name]
		local now = get_time_stamp()
		local last = now - MONITOR_POOL["MODULE_TIMESTAMP"]
		--only.log("D","MONITOR_MATCH1:" .."avge"..tostring(model_monitor["match_avge"]) .. "peak"..tostring(model_monitor["match_peak"]).."vally"..tostring(model_monitor["match_vally"]) .. "last" ..tostring(last)
		--	.. tostring(model_monitor["match_vally"]))
		model_monitor["match_avge"] = model_monitor["match_avge"] + last
		model_monitor["match_peak"] = (model_monitor["match_peak"] > last) and model_monitor["match_peak"] or last
		model_monitor["match_vally"] = (model_monitor["match_vally"] < last) and (model_monitor["match_vally"] ~= 0) and model_monitor["match_vally"] or last
		--only.log("D","MONITOR_MATCH2:" .."avge"..tostring(model_monitor["match_avge"]) .. "peak"..tostring(model_monitor["match_peak"]).."vally"..tostring(model_monitor["match_vally"]))
	end
end


--统计work次数
function mon_bef_work(name,flag)
	only.log("D", "进入WORK")
	if flag then
		local model_monitor = MONITOR_POOL["MONITOR_DETAIL_LIST"][name]
		model_monitor["work_grow"] = model_monitor["work_grow"] + 1
		only.log("D","MONITOR:WORK +1 |" .. tostring(model_monitor["work_grow"]))

		MONITOR_POOL["MODULE_TIMESTAMP"] = get_time_stamp()
	end
end

--为WORK记时
function mon_end_work(name,flag)
	if flag then
		local model_monitor = MONITOR_POOL["MONITOR_DETAIL_LIST"][name]
		local now = get_time_stamp()
		local last = now - MONITOR_POOL["MODULE_TIMESTAMP"]
	--	only.log("D","MONITOR_WORK1:" .."avge"..tostring(model_monitor["work_avge"]) .. "peak"..tostring(model_monitor["work_peak"]).."vally"..tostring(model_monitor["work_vally"]) .. "last" ..tostring(last))
		model_monitor["work_avge"] = model_monitor["work_avge"] + last
		model_monitor["work_peak"] = (model_monitor["work_peak"] > last) and model_monitor["work_peak"] or last
		model_monitor["work_vally"] = (model_monitor["work_vally"] < last) and (model_monitor["work_vally"] ~= 0) and model_monitor["work_vally"] or last
	--	only.log("D","MONITOR_WORK2:" .."avge"..tostring(model_monitor["work_avge"]) .. "peak"..tostring(model_monitor["work_peak"]).."vally"..tostring(model_monitor["work_vally"]) .. "last" ..tostring(last))
		
	end
end


--TODO
function mon_finish(name)
end




local function calculate( idx )
	local percent_store = {}
	local avgtime_store = {}
	local peak_vally_store = {}


	--是空表直接跳过(空表只会发生在启动后还没有客户接入的情况下)
	for k,v in pairs(MONITOR_POOL["MONITOR_DETAIL_LIST"]) do
		only.log("D","MONITOR:calculate:" .. k);
		--计算百分比
	--	if v["count"] and v["count"] ~= 0 then
		table.insert(
			percent_store,
			string.format("%s=%0.2f%%|%0.2f%%",
                       		(ALIAS_LIST["OWN_LIST"][k] or ALIAS_LOCAL[k] or k),
				(v["match_grow"] or 0)/((not v["count"] or v["count"] == 0) and 10000 or v["count"]) * 100,
				(v["work_grow"] or 0)/((not v["count"] or v["count"] == 0) and 10000 or v["count"]) * 100 )
		)

		--计算平均执行时间
		table.insert(
			avgtime_store, 
			string.format("%s=%0.3f|%0.3f",
                       		(ALIAS_LIST["OWN_LIST"][k] or ALIAS_LOCAL[k] or k),
				(v["match_avge"] or 0)/
					((not v["match_grow"] or v["match_grow"] == 0) and 10000 or v["match_grow"]),
				(v["work_avge"] or 0)/
					((not v["work_grow"] or v["work_grow"] == 0) and 10000 or v["work_grow"]) )
		)

		--峰值
		table.insert(peak_vally_store,
			string.format("%s=%0.3f|%0.3f|%0.3f|%0.3f",
                       	(ALIAS_LIST["OWN_LIST"][k] or ALIAS_LOCAL[k] or k),
			(v["match_peak"] or 0)*1000, (v["match_vally"] or 0)*1000,
			(v["work_peak"] or 0)*1000, (v["work_vally"] or 0)*1000 )
		)

		do
			if v["work_grow"] and v["work_grow"] > 0 then
				local redis_key = string.format("%s:%s:DRIVIEW:SCENE:COUNTS", os.date("%Y%m%d"),
                       		(ALIAS_LIST["OWN_LIST"][k] or ALIAS_LOCAL[k] or k) )
				redis_api.cmd("public", "", "incrby", redis_key, v["work_grow"])
			end
		end
	
		only.log("D","MONITOR:重置:" .. k)
		--重置子表
		refresh(k)
	end

	local time_string = string.format("%s----%s",
		os.date("%Y%m%d%H%M%S", MONITOR_POOL["PERIOD_TIMESTAMP"]),
		os.date("%Y%m%d%H%M%S", os.time()))

	local census_percent = string.format("%s==>\n%s", time_string, table.concat(percent_store, "\n"))
	local census_avgtime = string.format("%s==>\n%s", time_string, table.concat(avgtime_store, "\n"))
	local census_peak_vally = string.format("%s==>\n%s", time_string, table.concat(peak_vally_store, "\n"))
	return { PERCENT = census_percent , AVGTIME = census_avgtime , PEAK_VALLY = census_peak_vally }
end

local function callback( idx )
	--计算百分比
	only.log("D", "MONITOR:开始统计数据")
	local census = calculate(idx)
	only.log("I", census["PERCENT"])
	only.log("I", census["AVGTIME"])

	only.log("D", "MONITOR:开始写redis")
	redis_api.cmd("public", "", "mset",
		idx .. ":DRIVIEW:PERCENT:EXECUTE", census["PERCENT"],
		idx .. ":DRIVIEW:AVGTIME:EXECUTE", census["AVGTIME"],
		idx .. ":DRIVIEW:PEAK_VALLY:EXECUTE", census["PEAK_VALLY"])
	--TODO
	--redis_api.cmd("public", "", "set", "DRIVIEW:PERCENT:TRIGGER", census)

	MONITOR_POOL["PERIOD_TIMESTAMP"] = get_time_stamp()
	only.log("D", "MONITOR:统计数据结束")
end

--回调
function mon_clear( idx )
	local ok,ret = pcall(callback, idx)
	if not ok then
		only.log("E", "MONITOR:回调执行失败" .. tostring(ret))
	end
end
