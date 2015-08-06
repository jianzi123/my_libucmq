local only = require("only")
local app_lua_make_plan = _G.app_lua_make_plan

module("plan", package.seeall)

PLAN_LIST = { }
PIDX = 0

--[[
function make( args, func, time, live )
	if not func then return false end

	PIDX = PIDX + 1
	if PLAN_LIST[ tostring(PIDX) ] then
		only.log('E', string.format("PLAN %d is exist!", PIDX))
		return nil
	else
		PLAN_LIST[ tostring(PIDX) ] = { ARGS = args, FUNC = func, TIME = time, LIVE = live }
	end
	return PIDX
end

function mount( pidx )
	local plan = PLAN_LIST[ tostring(pidx) ]
	local addr = app_lua_make_plan(tonumber(pidx), plan["TIME"], plan["LIVE"])--TODO: fix time when loop
	if not addr then
		only.log('E', string.format("PLAN %s make fail!", tostring(pidx)))
	end
	return addr and true or false
end
function init( )
	for pidx in pairs(PLAN_LIST) do
		mount( pidx )
	end
end

function call( idx )
	local plan = PLAN_LIST[ tostring(idx) ]
	plan["FUNC"]( plan["ARGS"] )
end
]]--
