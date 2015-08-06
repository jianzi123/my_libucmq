local LIB_CJSON		= require('cjson')
local APP_POOL		= require('pool')
local APP_LOGS		= require('logs')
local APP_SCAN		= require('scan')
local only		= require('only')
local SERV_NAME		= _G.app_lua_get_serv_name()

module("apply", package.seeall)


local OWN_APP_HOME_PATH = "./lua/" .. SERV_NAME
local OWN_APP_LIST_PATH = OWN_APP_HOME_PATH .. "/list/"


local function save_one_list( t, f, c )
	if not APP_POOL["_FINAL_STAGE"] then return end

	only.log("D", string.format("save json data to file:%s", LIB_CJSON.encode( t )))
	local info = APP_SCAN.dump(t)
	info = string.gsub(info, '%[%d+%][\t% ]*=[\t% ]*', '')
	if c then
		--[[--->
		info = string.gsub(info, '%{[\n\t% ]*([^\n\t,]+),[\n\t% ]*([^\n\t,]+),?[\n\t% ]*%}', '{%1,%2}')
		--[[--->
		info = string.gsub(info, '{[\n\t% ]*([^{\n\t% ]+)', '{%1')
		info = string.gsub(info, ',[\n\t% ]*([^{]+)', ',%1')
		info = string.gsub(info, ',[\n\t% ]*(},)', '%1')
		--]]--->
		info = string.gsub(info, '",([\n\t% ]*)"', '","')
		info = string.gsub(info, '%{[\n\t% ]*("[^\n\t]+"),?[\n\t% ]*%}', '{%1}')
		--]]--->
	end

	local full = OWN_APP_LIST_PATH .. f
	local fd = io.open( full .. ".tmp", "w+")
	fd:write( string.format('module("%s")\n\n\nOWN_LIST = ', f) )
	fd:write( info )
	fd:close()
	os.execute(string.format("mv %s %s", full .. ".tmp", full .. ".lua"))
end

--<[===============================CONFIG DATA===============================]>--
OWN_CONFIG_LIST_INFO = require('CONFIG_LIST').OWN_LIST

function push_config( name, cfgs )
	OWN_CONFIG_LIST_INFO[ name ] = cfgs
	save_one_list( OWN_CONFIG_LIST_INFO, "CONFIG_LIST", false )
end

function pull_config( name )
	--return LIB_CJSON.encode( OWN_CONFIG_LIST_INFO[ name ] or {} )
	local info = LIB_CJSON.encode( OWN_CONFIG_LIST_INFO[ name ] or {} )
	local data = string.format('{"appname":"%s","config":%s}', name, string.gsub(info, '\\/', '/'))
	return data
end
--<[===============================STATUS DATA===============================]>--
OWN_STATUS_LIST_INFO = require('STATUS_LIST').OWN_LIST
-->+<-- for init
local OWN_APPLY_HAVE_POOL = {}
local OWN_APPLY_INDX_POOL = {}
-->+<-- for load
local OWN_APPLY_FUNC_POOL = {}
local OWN_APPLY_IFON_POOL = {}

local opt_list = {
	open = function( name )
		OWN_APPLY_IFON_POOL[ name ] = true
	end,
	close = function( name )
		OWN_APPLY_IFON_POOL[ name ] = false
	end,
	null = function( name )
		OWN_APPLY_IFON_POOL[ name ] = nil
	end,
}

function apply_control( name, cntl )
	OWN_APPLY_HAVE_POOL[ name ] = cntl
	opt_list[ cntl ]( name )
	local indx = OWN_APPLY_INDX_POOL[ name ]
	OWN_STATUS_LIST_INFO[ indx ][2] = cntl
end

function push_status( name, cntl )
	apply_control( name, cntl )
	save_one_list( OWN_STATUS_LIST_INFO, "STATUS_LIST", true )
end

function pull_status( )
	return string.format('[{"name":"精准模式","value":{}},{"name":"部分模式","value":{}},{"name":"广播模式","value":%s},{"name":"点播模式","value":{}}]',
		LIB_CJSON.encode(OWN_APPLY_HAVE_POOL))
end
--<[===============================APPLY FUNC===============================]>--

function apply_init( )
	only.log("I", "start init app module ... .. .")
	for indx, info in pairs(OWN_STATUS_LIST_INFO) do
		local name = info[1]
		local cntl = info[2]
		OWN_APPLY_HAVE_POOL[ name ] = cntl
		OWN_APPLY_INDX_POOL[ name ] = indx
		opt_list[ cntl ]( name )
	end
end

function apply_check( name )
	return OWN_APPLY_HAVE_POOL[ name ] and true or false
end

-->we can also use loadstring() instead of require()
--[[
function load_mod( name )
	local fd = io.open( name )
	local cmd = fd:read('*a')
	fd:close()
	local fun = loadstring( cmd )
	return fun
end
]]--
local function one_app_job( name, ifon, func )
	only.log("D", string.format("[%s]==1== doing ...", name))
	if ifon then
		only.log("D", string.format("[%s]==2== doing ...", name))
		--> set app log
		APP_POOL["OWN_APP_NAME"] = name
		--> do task
		local ok,result = pcall(func.handle)
		if not ok then
			only.log("E", result)
		end
		--> reset app log
		APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_DEFAULT_NAME"]
	end
end

function apply_insmod( name )
	if OWN_APPLY_FUNC_POOL[ name ] then
		only.log("W", "app " .. name .. " has installed!")
	else
		APP_LOGS.open_log( name )
		only.log("I", "install app " .. name)
	end
	OWN_APPLY_FUNC_POOL[ name ] = require(name)
	package.loaded[ name ] = nil
end

function apply_load()
	only.log("I", "start load app module ... .. .")
	for name, cntl in pairs(OWN_APPLY_HAVE_POOL) do
		if cntl == "open" or cntl == "close" then
			apply_insmod( name )
		end
	end
end

function apply_rmmod( name )
	if OWN_APPLY_FUNC_POOL[ name ] then
		APP_LOGS.free_log( name )

		OWN_APPLY_FUNC_POOL[ name ] = nil
		package.loaded[ name ] = nil
		only.log("I", "remove app " .. name)
	else
		only.log("W", "app " .. name .. "don't exist!")
	end
end

function apply_execute( name )
	if OWN_APPLY_IFON_POOL[ name ] == nil then
		only.log("W", string.format("API [%s] is not insmod!!!", name))
	end
	if OWN_APPLY_IFON_POOL[ name ] == false then
		only.log("W", string.format("API [%s] is not open!!!", name))
	end
	one_app_job( name, OWN_APPLY_IFON_POOL[ name ], OWN_APPLY_FUNC_POOL[ name ] )
end

function apply_runmods( )
	for _, info in pairs(OWN_STATUS_LIST_INFO) do
		local name = info[1]
		--local cntl = info[2]
		one_app_job( name, OWN_APPLY_IFON_POOL[ name ], OWN_APPLY_FUNC_POOL[ name ] )
	end
end
