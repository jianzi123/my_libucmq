local APP_POOL = require('pool')
local APP_POOL_EXACT_MAKE = APP_POOL.reckon_exact_match_word
local APP_POOL_LOCAL_INIT = APP_POOL.init_local_match
local APP_POOL_LOCAL_MAKE = APP_POOL.reverse_local_match_word
local APP_POOL_LOCAL_CHECK = APP_POOL.check_local_match
local APP_UTILS = require('utils')
local APP_LOGS = require('logs')
local only = require('only')
local APP_CFG = require('cfg')
local socket = require('socket')
local monitor = require('monitor')

local table = require('table')
local os = require('os')
local io = require('io')
local string = require('string')
local print = print
local pairs = pairs
local pcall = pcall
local assert = assert

local classify = APP_CFG["OWN_INFO"]["OPEN_LOGS_CLASSIFY"]

local BASE = _G

module("model")
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
local function one_app_job( name, insp, ifon, func )
	local t1 = socket.gettime()
	local t2 = nil
	local state = true
	
	monitor.mon_start( name, ifon )
	if ifon then
		--> set app log
		APP_POOL["OWN_APP_NAME"] = name
		--> check if match
		monitor.mon_bef_match( name, insp )
		if insp then
			local ok,result = pcall(func.match)
			if not ok then
				only.log("E", result)
			end
			state = ok and result or false
		end
		monitor.mon_end_match( name, insp )

		--> check if work
		t2 = socket.gettime()
		monitor.mon_bef_work( name, state )
		if state then
			--> do task
			local ok,result = pcall(func.work)
			if not ok then
				only.log("E", result)
			end
		end
		monitor.mon_end_work( name, state )
		--> reset app log
		APP_POOL["OWN_APP_NAME"] = APP_POOL["OWN_APPLY_NAME"]
	end
	monitor.mon_finish( name )
	local t3 = socket.gettime()
	local mode = APP_POOL["OWN_APP_MODE"]
	APP_POOL["OWN_APP_MODE"] = APP_POOL["OWN_APPLY_NAME"]
        if APP_CFG["OWN_INFO"]["SYSLOGLV"] then 
                only.log('S', string.format("MODULE : %s ===> ifon=(%s) insp=(%s) call=(%s) match [%f] | work [%f] | total [%f]",
                        name, ifon, insp, state, ifon and (t2 - t1) or 0, ifon and (t3 - t2) or 0, t3 - t1))
        end
	APP_POOL["OWN_APP_MODE"] = mode
end
----------------------------------------exact--------------------------------------------
local OWN_EXACT_FUNC_POOL = {}
local OWN_EXACT_IFON_POOL = {}
local OWN_EXACT_WORD_POOL = {}
local OWN_EXACT_NAME_POOL = {}

function exact_init()
	only.log("I", "init exact module ...")
end

function exact_control( name, set )
	OWN_EXACT_IFON_POOL[ name ] = set and true or false
end

function exact_insmod( name, set )
	if OWN_EXACT_WORD_POOL[ name ] then
		only.log("W", "exact app " .. name .. " has installed!")
	else
		if classify then
			APP_LOGS.open_log( name )
		end
		only.log("I", "install exact app " .. name)
	end
	local func = BASE.require(name)
	local info = func.bind()
	local word = APP_POOL.create_exact_match_word(info)
	OWN_EXACT_WORD_POOL[ name ] = word
	OWN_EXACT_IFON_POOL[ name ] = set and true or false
	if not OWN_EXACT_NAME_POOL[ word ] then
		OWN_EXACT_NAME_POOL[ word ] = {}
	end
	table.insert(OWN_EXACT_NAME_POOL[ word ], name)
	OWN_EXACT_FUNC_POOL[ name ] = func
	BASE.package.loaded[ name ] = nil
end

function exact_rmmod( name )
	if OWN_EXACT_WORD_POOL[name] then
		if classify then
			APP_LOGS.free_log( name )
		end
		local word = OWN_EXACT_WORD_POOL[name]
		APP_UTILS.tab_remove_ivalue( OWN_EXACT_NAME_POOL[ word ], name )
		if #OWN_EXACT_NAME_POOL[ word ] == 0 then
			OWN_EXACT_NAME_POOL[ word ] = nil
		end
		OWN_EXACT_WORD_POOL[ name ] = nil
		OWN_EXACT_FUNC_POOL[ name ] = nil
		OWN_EXACT_IFON_POOL[ name ] = nil
		BASE.package.loaded[ name ] = nil
		only.log("I", "remove exact app " .. name)
	else
		only.log("W", "exact app " .. name .. "don't exist!")
	end
end

function exact_runmods( name, insp )
	if name then
		one_app_job( name, insp, OWN_EXACT_IFON_POOL[ name ], OWN_EXACT_FUNC_POOL[ name ] )
	else
		--> fetch index
		local word = APP_POOL_EXACT_MAKE( )
		--> get task list
		for _, name in pairs(OWN_EXACT_NAME_POOL[ word ] or {}) do
			one_app_job( name, insp, OWN_EXACT_IFON_POOL[ name ], OWN_EXACT_FUNC_POOL[ name ] )
		end
	end
end

----------------------------------------local--------------------------------------------
local OWN_LOCAL_FUNC_POOL = {}
local OWN_LOCAL_IFON_POOL = {}
local OWN_LOCAL_NAME_POOL = {}
local OWN_LOCAL_WORD_POOL = {}

function local_init()
	only.log("I", "init local module ...")
	APP_POOL_LOCAL_INIT()
end

function local_control( name, set )
	OWN_LOCAL_IFON_POOL[ name ] = set and true or false
end

function local_insmod( name, set )
	if OWN_LOCAL_WORD_POOL[ name ] then
		only.log("W", "local app " .. name .. " has installed!")
	else
		if classify then
			APP_LOGS.open_log( name )
		end
		only.log("I", "install local app " .. name)
	end
	local func = BASE.require(name)
	local info = func.bind()
	local word = APP_POOL.create_local_match_word( info )
	OWN_LOCAL_WORD_POOL[ name ] = word
	OWN_LOCAL_IFON_POOL[ name ] = set and true or false
	if not OWN_LOCAL_NAME_POOL[ word ] then
		OWN_LOCAL_NAME_POOL[ word ] = {}
	end
	table.insert(OWN_LOCAL_NAME_POOL[ word ], name)
	OWN_LOCAL_FUNC_POOL[ name ] = func
	BASE.package.loaded[ name ] = nil
end

function local_rmmod( name )
	if OWN_LOCAL_WORD_POOL[name] then
		if classify then
			APP_LOGS.free_log( name )
		end
		local word = OWN_LOCAL_WORD_POOL[name]
		APP_UTILS.tab_remove_ivalue( OWN_LOCAL_NAME_POOL[ word ], name )
		if #OWN_LOCAL_NAME_POOL[ word ] == 0 then
			OWN_LOCAL_NAME_POOL[ word ] = nil
		end
		OWN_LOCAL_WORD_POOL[ name ] = nil
		OWN_LOCAL_FUNC_POOL[ name ] = nil
		OWN_LOCAL_IFON_POOL[ name ] = nil
		BASE.package.loaded[ name ] = nil
		only.log("I", "remove local app " .. name)
	else
		only.log("W", "local app " .. name .. "don't exist!")
	end
end

function local_runmods( name, insp )
	if name then
		one_app_job( name, insp, OWN_LOCAL_IFON_POOL[ name ], OWN_LOCAL_FUNC_POOL[ name ] )
	else
		--> fetch reverse sequence
		local word = APP_POOL_LOCAL_MAKE( )
		for idx in pairs(OWN_LOCAL_NAME_POOL) do
			--> get task list
			if APP_POOL_LOCAL_CHECK( word, idx ) then
				for _, name in pairs(OWN_LOCAL_NAME_POOL[ idx ]) do
					one_app_job( name, insp, OWN_LOCAL_IFON_POOL[ name ], OWN_LOCAL_FUNC_POOL[ name ] )
				end
			end
		end
	end
end

----------------------------------------whole--------------------------------------------
local OWN_WHOLE_FUNC_POOL = {}
local OWN_WHOLE_IFON_POOL = {}

function whole_init()
	only.log("I", "init whole module ...")
end

function whole_control( name, set )
	OWN_WHOLE_IFON_POOL[ name ] = set and true or false
end

function whole_insmod( name, set )
	if OWN_WHOLE_FUNC_POOL[ name ] then
		only.log("W", "whole app " .. name .. " has installed!")
	else
		if classify then
			APP_LOGS.open_log( name )
		end
		only.log("I", "install whole app " .. name)
	end
	OWN_WHOLE_FUNC_POOL[ name ] = BASE.require(name)
	OWN_WHOLE_IFON_POOL[ name ] = set and true or false
	BASE.package.loaded[ name ] = nil
end

function whole_rmmod( name )
	if OWN_WHOLE_FUNC_POOL[ name ] then
		if classify then
			APP_LOGS.free_log( name )
		end
		OWN_WHOLE_FUNC_POOL[ name ] = nil
		OWN_WHOLE_IFON_POOL[ name ] = nil
		BASE.package.loaded[ name ] = nil
		only.log("I", "remove whole app " .. name)
	else
		only.log("W", "whole app " .. name .. "don't exist!")
	end
end

function whole_runmods( name, insp )
	if name then
		one_app_job( name, insp, OWN_WHOLE_IFON_POOL[ name ], OWN_WHOLE_FUNC_POOL[ name ] )
	else
		for name in pairs(OWN_WHOLE_FUNC_POOL) do
			one_app_job( name, insp, OWN_WHOLE_IFON_POOL[ name ], OWN_WHOLE_FUNC_POOL[ name ] )
		end
	end
end
----------------------------------------alone--------------------------------------------
local OWN_ALONE_FUNC_POOL = {}
local OWN_ALONE_IFON_POOL = {}

function alone_init()
	only.log("I", "init alone module ...")
end

function alone_control( name, set )
	OWN_ALONE_IFON_POOL[ name ] = set and true or false
end

function alone_insmod( name, set )
	if OWN_ALONE_FUNC_POOL[ name ] then
		only.log("W", "alone app " .. name .. " has installed!")
	else
		if classify then
			APP_LOGS.open_log( name )
		end
		only.log("I", "install alone app " .. name)
	end
	OWN_ALONE_FUNC_POOL[ name ] = BASE.require(name)
	OWN_ALONE_IFON_POOL[ name ] = set and true or false
	BASE.package.loaded[ name ] = nil
end

function alone_rmmod( name )
	if OWN_ALONE_FUNC_POOL[ name ] then
		if classify then
			APP_LOGS.free_log( name )
		end
		OWN_ALONE_FUNC_POOL[ name ] = nil
		OWN_ALONE_IFON_POOL[ name ] = nil
		BASE.package.loaded[ name ] = nil
		only.log("I", "remove alone app " .. name)
	else
		only.log("W", "alone app " .. name .. "don't exist!")
	end
end

function alone_runmods( name, insp )
	if name then
		one_app_job( name, insp, OWN_ALONE_IFON_POOL[ name ], OWN_ALONE_FUNC_POOL[ name ] )
	end
end
