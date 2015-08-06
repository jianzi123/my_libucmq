local LIB_CJSON		= require('cjson')
local APP_POOL		= require('pool')
local APP_LOGS		= require('logs')
local APP_GOSAY		= require('gosay')
local APP_APPLY		= require('apply')
local APP_UTILS		= require('utils')
local APP_REDIS_API	= require('redis_pool_api')
local only		 = require('only')
local supex        = require('supex')

function app_init()
	--> init first
	APP_LOGS.init_log( {APP_POOL["OWN_DEFAULT_NAME"], APP_POOL["OWN_COMMAND_NAME"]} )
	--> init redis
	APP_REDIS_API.init( )
	--> init apply
	APP_APPLY.apply_init( )
	--> load apply
	APP_APPLY.apply_load( )
end

function app_exit()
	--> free final
	APP_LOGS.exit_log( )
end



function app_rfsh( top, sfd )
	APP_POOL["_FINAL_STAGE"] = top
	only.log("S", 'rfsh logs ... .. .')
	APP_LOGS.rfsh_log( )
end

function app_cntl( top, name, cmds )
	APP_POOL["_FINAL_STAGE"] = top
	only.log("I", string.format("【%s】 ------> |name:%s", cmds, name))
	local ctrl_cmd_list = {
		open = function( name )
			APP_APPLY.push_status( name, "open" )
		end,
		close = function( name )
			APP_APPLY.push_status( name, "close" )
		end,
		insmod = function( name )
			APP_APPLY.apply_insmod( name )
			APP_APPLY.push_status( name, "open" )
		end,
		rmmod = function( name )
			APP_APPLY.apply_rmmod( name )
			APP_APPLY.push_status( name, "null" )
		end,
		delete = function( name )
		end,
	}
	if APP_APPLY.apply_check( name ) then
		ctrl_cmd_list[cmds]( name )
	end
end


function app_pull( top, sfd )
	APP_POOL["_FINAL_STAGE"] = top
	APP_POOL["_SOCKET_HANDLE"] = sfd
	--> come into manage model
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_COMMAND_NAME"]
	only.log("D", '_________________________________START_________________________________________')
	--> get data
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
			get_all_app = function( )
				local data = APP_APPLY.pull_status()
				APP_GOSAY.resp( 200, data )
			end,
			get_app_cfg = function( )
				local data = APP_APPLY.pull_config( jo["appname"] )
				APP_GOSAY.resp( 200, data )
			end,
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
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_DEFAULT_NAME"]
end

function app_push( top, sfd )
	APP_POOL["_FINAL_STAGE"] = top
	APP_POOL["_SOCKET_HANDLE"] = sfd
	--> come into manage model
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_COMMAND_NAME"]
	only.log("D", '_________________________________START_________________________________________')
	--> get data
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
				app_cntl( top, jo["appname"], jo["status"] )
			end,
			fix_app_cfg = function( )
				APP_APPLY.push_config( jo["appname"], jo["config"] )
			end,
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
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_DEFAULT_NAME"]
end

local function app_call( top, msg, way )
	only.log("D", '_________________________________START_________________________________________')
	APP_POOL["_FINAL_STAGE"] = top
	--> get info
	APP_POOL["OUR_INFO_DATA"] = msg

	local path = nil
	local uri_arg = nil

	------- method , head , body , path , uri_arg
	APP_POOL['OUR_METHOD'],APP_POOL['OUR_HEAD'],APP_POOL['OUR_BODY_DATA'] , path , APP_POOL["OUR_URI_ARGS"] = supex.split_http_data(msg)

	--only.log('D', string.format("--%s \r\n-- %s  \r\n-- %s  \r\n-- %s  \r\n-- %s" ,
	--			APP_POOL['OUR_METHOD'],APP_POOL['OUR_HEAD'],APP_POOL['OUR_BODY_DATA'] , path , APP_POOL["OUR_URI_ARGS"]
	--			) )



	--> get data
	-- APP_POOL["OUR_BODY_DATA"] = nil--app_lua_get_body_data(sfd)

	-- only.log("I", "BODY DATA is:" .. tostring(APP_POOL["OUR_BODY_DATA"]))
	-- APP_POOL["OUR_BODY_TABLE"] = {}--APP_UTILS.parse_http_body_data( app_lua_get_head_data(sfd), APP_POOL["OUR_BODY_DATA"] ) or {}

	--> get args
	-- APP_POOL["OUR_URI_ARGS"] = nil--app_lua_get_uri_args(sfd)

	-- only.log("I", "URI DATA is:" .. tostring(APP_POOL["OUR_URI_ARGS"]))
	-- APP_POOL["OUR_URI_TABLE"] = {}--APP_UTILS.parse_url( APP_POOL["OUR_URI_ARGS"] ) or {}

	--> run call
	only.log("I", 'access : ' .. path)

	--local come_msize = collectgarbage("count")
	if way then
		APP_APPLY.apply_execute( path )
	else
		APP_APPLY.apply_runmods( )
	end
	--[[
	local done_msize = collectgarbage("count")
	collectgarbage("collect")
	local over_msize = collectgarbage("count")
	print( string.format("APPLY CALL COME : memory size \t[%d]KB \t[%d]M", come_msize, come_msize/1024) )
	print( string.format("APPLY CALL DONE : memory size \t[%d]KB \t[%d]M", done_msize, done_msize/1024) )
	print( string.format("APPLY CALL OVER : memory size \t[%d]KB \t[%d]M", over_msize, over_msize/1024) )
	print()
	]]--
	only.log("D", '_________________________________OVER_________________________________________\n\n')
end

function app_call_all( top, msg )
	app_call( top, msg, false )
end

function app_call_one( top, msg )
	app_call( top, msg, true )
end
