local LIB_CJSON = require('cjson')
local APP_REDIS_API = require('redis_pool_api')
local APP_LOGS = require('logs')
local APP_GOSAY = require('gosay')
local APP_POOL = require('pool')
local APP_POOL_INIT_MAP = APP_POOL.init_map
local APP_POOL_SET_MAP = APP_POOL.set_map
local APP_POOL_RESET_MAP = APP_POOL.reset_map
local APP_ROUTE = require('route')
local APP_ROUTE_CHECK = APP_ROUTE.check_status
local APP_ROUTE_LOAD = APP_ROUTE.load_all_app
local APP_ROUTE_MAKE_APP = APP_ROUTE.make_new_app
local APP_ROUTE_MAKE_CFG = APP_ROUTE.make_new_cfg
local APP_ROUTE_MAKE_EXP = APP_ROUTE.make_new_exp
local APP_ROUTE_MAKE_WAS = APP_ROUTE.make_new_was
local APP_ROUTE_MAKE_IDX = APP_ROUTE.make_new_idx
local APP_ROUTE_CLEAN_APP = APP_ROUTE.clean_old_app
local APP_ROUTE_CLEAN_CFG = APP_ROUTE.clean_old_cfg
local APP_ROUTE_CLEAN_EXP = APP_ROUTE.clean_old_exp
local APP_ROUTE_CLEAN_WAS = APP_ROUTE.clean_old_was
local APP_ROUTE_CLEAN_IDX = APP_ROUTE.clean_old_idx
local APP_ROUTE_PUSH_TEMPLET = APP_ROUTE.push_templet
local APP_ROUTE_PUSH_STATUS = APP_ROUTE.push_status
local APP_ROUTE_PUSH_CONFIG = APP_ROUTE.push_config
local APP_ROUTE_PULL_SEARCH = APP_ROUTE.pull_search
local APP_ROUTE_PULL_TEMPLET = APP_ROUTE.pull_templet
local APP_ROUTE_PULL_STATUS = APP_ROUTE.pull_status
local APP_ROUTE_PULL_CONFIG = APP_ROUTE.pull_config
local APP_ROUTE_PULL_EXPLAIN = APP_ROUTE.pull_explain
local APP_ROUTE_PULL_ALIAS = APP_ROUTE.pull_alias
local APP_MODEL = require('model')
local APP_UTILS = require('utils')
local APP_KV_INFO_LIST = require('KV_INFO_LIST')
local only = require('only')
local init_data = require("init_data")

local OWN_EXACT_MODE = 1
local OWN_LOCAL_MODE = 2
local OWN_WHOLE_MODE = 3
local OWN_ALONE_MODE = 4
local OWN_MODE_INDEX = {
	["exact"] = OWN_EXACT_MODE,
	["local"] = OWN_LOCAL_MODE,
	["whole"] = OWN_WHOLE_MODE,
	["alone"] = OWN_ALONE_MODE,
}

local OWN_MAIN_INIT = {
	[OWN_EXACT_MODE] = APP_MODEL.exact_init,
	[OWN_LOCAL_MODE] = APP_MODEL.local_init,
	[OWN_WHOLE_MODE] = APP_MODEL.whole_init,
	[OWN_ALONE_MODE] = APP_MODEL.alone_init,
}
local OWN_MAIN_CONTROL = {
	[OWN_EXACT_MODE] = APP_MODEL.exact_control,
	[OWN_LOCAL_MODE] = APP_MODEL.local_control,
	[OWN_WHOLE_MODE] = APP_MODEL.whole_control,
	[OWN_ALONE_MODE] = APP_MODEL.alone_control,
}
local OWN_MAIN_INSMOD = {
	[OWN_EXACT_MODE] = APP_MODEL.exact_insmod,
	[OWN_LOCAL_MODE] = APP_MODEL.local_insmod,
	[OWN_WHOLE_MODE] = APP_MODEL.whole_insmod,
	[OWN_ALONE_MODE] = APP_MODEL.alone_insmod,
}
local OWN_MAIN_RMMOD = {
	[OWN_EXACT_MODE] = APP_MODEL.exact_rmmod,
	[OWN_LOCAL_MODE] = APP_MODEL.local_rmmod,
	[OWN_WHOLE_MODE] = APP_MODEL.whole_rmmod,
	[OWN_ALONE_MODE] = APP_MODEL.alone_rmmod,
}
local OWN_MAIN_RUNMODS = {
	[OWN_EXACT_MODE] = APP_MODEL.exact_runmods,
	[OWN_LOCAL_MODE] = APP_MODEL.local_runmods,
	[OWN_WHOLE_MODE] = APP_MODEL.whole_runmods,
	[OWN_ALONE_MODE] = APP_MODEL.alone_runmods,
}


function app_init()
	--> init first
	APP_LOGS.init_log( {APP_POOL["OWN_APPLY_NAME"], APP_POOL["OWN_FETCH_NAME"], APP_POOL["OWN_MERGE_NAME"]} )
	--> init redis pool
	APP_REDIS_API.init()
	--> init maps
	APP_POOL_INIT_MAP( )
	OWN_MAIN_INIT[ OWN_EXACT_MODE ]()
	OWN_MAIN_INIT[ OWN_LOCAL_MODE ]()
	OWN_MAIN_INIT[ OWN_WHOLE_MODE ]()
	OWN_MAIN_INIT[ OWN_ALONE_MODE ]()
	--> init model
	APP_ROUTE_LOAD( OWN_EXACT_MODE, OWN_MAIN_INSMOD[ OWN_EXACT_MODE ] )
	APP_ROUTE_LOAD( OWN_LOCAL_MODE, OWN_MAIN_INSMOD[ OWN_LOCAL_MODE ] )
	APP_ROUTE_LOAD( OWN_WHOLE_MODE, OWN_MAIN_INSMOD[ OWN_WHOLE_MODE ] )
	APP_ROUTE_LOAD( OWN_ALONE_MODE, OWN_MAIN_INSMOD[ OWN_ALONE_MODE ] )
end

function app_exit()
	--> free final
	APP_LOGS.exit_log( )
end



function app_rfsh( top, sfd, sch )
	APP_POOL["_FINAL_STAGE"] = top
	only.log("S", 'rfsh logs ... .. .')
	APP_LOGS.rfsh_log( )
end

function app_cntl( top, name, cmds, mode )
	APP_POOL["_FINAL_STAGE"] = top
	only.log("I", string.format("【%s】 ------> |model:%s|name:%s", cmds, mode, name))
	local ctrl_cmd_list = {
		open = function( name, mode )
			if APP_ROUTE_CHECK( mode, name ) then
				OWN_MAIN_CONTROL[ mode ]( name, true )
				APP_ROUTE_PUSH_STATUS( mode, name, "open" )
			end
		end,
		close = function( name, mode )
			if APP_ROUTE_CHECK( mode, name ) then
				OWN_MAIN_CONTROL[ mode ]( name, false )
				APP_ROUTE_PUSH_STATUS( mode, name, "close" )
			end
		end,
		insmod = function( name, mode )
			OWN_MAIN_INSMOD[ mode ]( name, true )
			APP_ROUTE_PUSH_STATUS( mode, name, "open" )
		end,
		rmmod = function( name, mode )
			OWN_MAIN_RMMOD[ mode ]( name )
			APP_ROUTE_PUSH_STATUS( mode, name, "null" )
		end,
		delete = function( name, mode )
			OWN_MAIN_RMMOD[ mode ]( name )
			APP_ROUTE_PUSH_STATUS( mode, name, nil )
			local mode_list = {
				[1] = "exact",
				[2] = "local",
				[3] = "whole",
				[4] = "alone",
			}
			APP_ROUTE_CLEAN_APP( mode_list[ mode ], name )
			APP_ROUTE_CLEAN_CFG( name )
			APP_ROUTE_CLEAN_EXP( name )
			APP_ROUTE_CLEAN_WAS( name )
			APP_ROUTE_CLEAN_IDX( mode_list[ mode ], name )
		end,
	}
	ctrl_cmd_list[cmds]( name, mode )
end


function app_pull( top, sfd, sch )
	APP_POOL["_FINAL_STAGE"] = top
	APP_POOL["_SOCKET_HANDLE"] = sfd
	APP_POOL["_TASKER_SCHEME"] = sch
	--> come into manage model
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_FETCH_NAME"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_FETCH_NAME"]
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	-- APP_POOL.OUR_BODY_DATA = app_lua_get_body_data(sfd)
	local data = app_lua_get_body_data(sfd)
	only.log("I", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		--> parse data
		local pull_cmd_list = {
			get_all_app = function(  )
				local body = APP_ROUTE_PULL_STATUS()
				APP_GOSAY.resp( 200, body )
			end,
			get_all_was = function(  )
				local body = APP_ROUTE_PULL_ALIAS()
				APP_GOSAY.resp( 200, body )
			end,
			get_tmp_app = function(  )
				local body = APP_ROUTE_PULL_SEARCH( jo["tmpname"], jo["mode"] )
				APP_GOSAY.resp( 200, body )
			end,
			get_all_tmp = function(  )
				local body = APP_ROUTE_PULL_TEMPLET( jo["mode"] )
				APP_GOSAY.resp( 200, body )
			end,
			get_app_cfg = function( )
				local config = APP_ROUTE_PULL_CONFIG( jo["appname"] )
				local data = string.format('{"appname":"%s","config":%s}', jo["appname"], config)
				APP_GOSAY.resp( 200, data )
			end,
			get_app_exp = function( )
				local explain = APP_ROUTE_PULL_EXPLAIN( jo["appname"] )
				local data = string.format('{"appname":"%s","explain":%s}', jo["appname"], explain)
				APP_GOSAY.resp( 200, data )
			end,
			get_tmp_arg = function( )
				local info = {}
				info[ "format" ] = APP_KV_INFO_LIST["OWN_INFO"]["format"]
				info[ "args" ] = {}
				local keys = APP_ROUTE_PULL_TEMPLET( jo["mode"], jo["tmpname"] )
				for i=1, #(keys or {}) do
					table.insert( info[ "args" ], { [tostring( keys[i] )] = APP_KV_INFO_LIST["OWN_INFO"]["keywords"][tostring( keys[i] )] } )
				end
				local data = LIB_CJSON.encode(info)
				only.log("I", data)
				APP_GOSAY.resp( 200, data )
			end,
			get_all_job = function( )
				local info = {}
				info[ "func" ] = APP_KV_INFO_LIST["OWN_INFO"]["workfunc"][ jo["mode"] ] or {}
				local data = LIB_CJSON.encode(info)
				only.log("I", data)
				APP_GOSAY.resp( 200, data )
			end,
			get_all_arg = function( )
				local info = {}
				local keys = APP_ROUTE_PULL_TEMPLET()
				for k,_ in pairs(APP_KV_INFO_LIST["OWN_INFO"]["keywords"] or {}) do
					table.insert( info, k )
				end
				local data = LIB_CJSON.encode(info)
				only.log("I", data)
				APP_GOSAY.resp( 200, data )
			end
		}
		if jo["operate"] then 
			local ok,result = pcall( pull_cmd_list[ jo["operate"] ] )
			if not ok then
				only.log("E", result)
			end
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_APPLY_NAME"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_APPLY_NAME"]
end

function app_push( top, sfd, sch )
	APP_POOL["_FINAL_STAGE"] = top
	APP_POOL["_SOCKET_HANDLE"] = sfd
	APP_POOL["_TASKER_SCHEME"] = sch
	--> come into manage model
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_MERGE_NAME"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_MERGE_NAME"]
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	-- APP_POOL.OUR_BODY_DATA = app_lua_get_body_data(sfd)
	local data = app_lua_get_body_data(sfd)
	only.log("S", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		--> parse data
		local push_cmd_list = {
			ctl_one_app = function( )
				app_cntl( top, jo["appname"], jo["status"], OWN_MODE_INDEX[jo["mode"]] )
			end,
			fix_app_cfg = function( )
				APP_ROUTE_PUSH_CONFIG( jo["appname"], jo["config"] )
			end,
			new_one_tmp = function( )
				APP_ROUTE_PUSH_TEMPLET( jo["mode"], jo["tmpname"], jo["remarks"], jo["args"] )
			end,
			new_one_app = function( )
				APP_ROUTE_MAKE_APP( jo["mode"], jo["appname"], jo["args"], jo["func"])
				APP_ROUTE_MAKE_CFG( jo["appname"], jo["args"], jo["func"] )
				APP_ROUTE_MAKE_EXP( jo["appname"], jo["args"], jo["func"] )
				APP_ROUTE_MAKE_WAS( jo["appname"], jo["nickname"] )
				APP_ROUTE_MAKE_IDX( jo["appname"], jo["tmpname"] )
			end
		}
		if jo["operate"] then 
			local ok,result = pcall( push_cmd_list[ jo["operate"] ] )
			if not ok then
				only.log("E", result)
			end
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_APPLY_NAME"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_APPLY_NAME"]
end

function app_call( top, sfd, sch )
	--only.log("D", '_________________________________START_________________________________________')
	APP_POOL["_FINAL_STAGE"] = top
	APP_POOL["_SOCKET_HANDLE"] = sfd
	APP_POOL["_TASKER_SCHEME"] = sch
	--> get data
	APP_POOL["OUR_BODY_DATA"] = app_lua_get_body_data(sfd)
	only.log("I", "BODY DATA is:" .. tostring(APP_POOL["OUR_BODY_DATA"]))
	APP_POOL["OUR_BODY_TABLE"] = APP_UTILS.parse_http_body_data( app_lua_get_head_data(sfd), APP_POOL["OUR_BODY_DATA"] ) or {}

	--> get args
	APP_POOL["OUR_URI_ARGS"] = app_lua_get_uri_args(sfd)
	only.log("I", "URI DATA is:" .. tostring(APP_POOL["OUR_URI_ARGS"]))
	APP_POOL["OUR_URI_TABLE"] = APP_UTILS.parse_url( APP_POOL["OUR_URI_ARGS"] ) or {}

	--- add @ here for first run before all modules
	APP_POOL["OWN_APP_NAME"] = init_data['NAME']
	APP_POOL["OWN_APP_MODE"] = init_data['NAME']
	local ok, err = pcall(init_data.handle)
	if not ok then
		only.log('E', string.format("pcall init_data.handle error:%s", err))
	end
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_APPLY_NAME"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_APPLY_NAME"]

	--> run call
	local app_name = APP_POOL["OUR_URI_TABLE"]["app_name"]
	local app_mode = APP_POOL["OUR_URI_TABLE"]["app_mode"]
	if (not app_name) and (not app_mode or app_mode == "exact" or app_mode == "local") then
		--> parse map
		for k, v in pairs( APP_POOL["OUR_BODY_TABLE"] or {}) do
			APP_POOL_SET_MAP( k , v )
		end
	end
	if app_mode then
		local log_idx_list = {
			["exact"] = "OWN_EXACT_NAME",
			["local"] = "OWN_LOCAL_NAME",
			["whole"] = "OWN_WHOLE_NAME",
			["alone"] = "OWN_ALONE_NAME",
		}
		APP_POOL["OWN_APP_MODE"] = APP_POOL[ log_idx_list[app_mode] ]
		--only.log("I", string.format("app_mode->[%s] OWN_APP_MODE->[%s] OWN_MODE_INDEX->[%d]", app_mode, APP_POOL["OWN_APP_MODE"], OWN_MODE_INDEX[app_mode]))
		OWN_MAIN_RUNMODS[ OWN_MODE_INDEX[app_mode] ]( app_name, not app_name )
	else
		-->>[alone]
		APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_ALONE_NAME"]
		OWN_MAIN_RUNMODS[ OWN_ALONE_MODE ]( app_name, not app_name )
		-->>[whole]
		APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_WHOLE_NAME"]
		OWN_MAIN_RUNMODS[ OWN_WHOLE_MODE ]( app_name, not app_name  )
		-->>[exact]
		APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_EXACT_NAME"]
		OWN_MAIN_RUNMODS[ OWN_EXACT_MODE ]( app_name, not app_name )
		-->>[local]
		APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_LOCAL_NAME"]
		OWN_MAIN_RUNMODS[ OWN_LOCAL_MODE ]( app_name, not app_name )
	end
	if (not app_name) and (not app_mode or app_mode == "exact" or app_mode == "local") then
		--> reset map
		APP_POOL_RESET_MAP( )
	end

	--> reset to main
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_APPLY_NAME"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_APPLY_NAME"]

	--only.log("D", '_________________________________OVER_________________________________________\n\n')
end
