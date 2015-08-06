local LIB_CJSON		= require('cjson')
local APP_PLAN		= require('plan')
local APP_POOL		= require('pool')
local APP_LOGS		= require('logs')
local APP_UTILS		= require('utils')
local APP_REDIS_API	= require('redis_pool_api')
local only		= require('only')

local app_lua_gain_plan = app_lua_gain_plan
local SERV_NAME		= app_lua_get_serv_name()
local APP_APPLY		= require( SERV_NAME )

function app_init( )
	--> init first
	APP_LOGS.init_log( {APP_POOL["OWN_DEFAULT_NAME"], APP_POOL["OWN_COMMAND_NAME"]} )
	--> init redis
	APP_REDIS_API.init( )
	--> init apply
	APP_APPLY.origin( )
end

function app_exit( )
	--> free final
	APP_LOGS.exit_log( )
end

function app_load( )
	--APP_PLAN.init( )
end


function app_rfsh( )
	only.log("S", 'rfsh logs ... .. .')
	APP_LOGS.rfsh_log( )
end



function app_pull( top, msg )
	APP_POOL["_FINAL_STAGE"] = top
	--> come into manage model
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_COMMAND_NAME"]
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	local data = msg
	only.log("I", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		local ok,result = pcall(APP_APPLY.lookup, jo)
		if not ok then
			only.log("E", result)
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_DEFAULT_NAME"]
end

function app_push( top, msg )
	APP_POOL["_FINAL_STAGE"] = top
	--> come into manage model
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_COMMAND_NAME"]
	only.log("D", '_________________________________START_________________________________________')
	--> get data
	local data = msg
	only.log("S", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		local ok,result = pcall(APP_APPLY.accept, jo)
		if not ok then
			only.log("E", result)
		end
	end
	::DO_NOTHING::
	only.log("D", '_________________________________OVER_________________________________________\n\n')
	--> reset to main
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_DEFAULT_NAME"]
end

function app_call( top, idx )
	APP_POOL["_FINAL_STAGE"] = top
	only.log("D", '_________________________________START_________________________________________')
	--> set app log
	APP_POOL["OWN_APP_NAME"] = SERV_NAME
	only.log("I", string.format("call plan ID : %d", idx))
	local data = app_lua_gain_plan(idx)
	only.log("D", data)
	if data then
		local ok,jo = pcall(LIB_CJSON.decode, data)
		if not ok then
			only.log('E', "error json body <--> " .. data)
			goto DO_NOTHING
		end
		--> run call
		local ok,result = pcall(APP_APPLY.handle, jo)
		if not ok then
			only.log("E", result)
		end
	end
	::DO_NOTHING::
	--> reset app log
	APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_DEFAULT_NAME"]
	--> reset data
	only.log("D", '_________________________________OVER_________________________________________\n\n')
end
