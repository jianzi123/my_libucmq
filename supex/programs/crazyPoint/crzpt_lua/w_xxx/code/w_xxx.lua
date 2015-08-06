local only = require('only')
local APP_POOL = require('pool')
local plan = require('plan')
local scan = require('scan')

local plan = require('plan')
local scan = require('scan')
local weather = require('l_f_weather_forcast')
local utils = require('utils')
module('w_xxx', package.seeall)

--[[
local function main_entry( ... )
print("hello world!")
end
]]--


function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end

function origin()
	only.log("D", "init start ... .. .")-- how to mount task once?
end


function lookup( jo )
	print("xxxx")
end

function accept( jo )
	--[[
	--> make lua
	local delay = 240
	local time = (os.time() + 8*60*60 + delay) % (24*60*60)
	local pidx = plan.make( nil, main_entry, time, true )
	--> mount C
	if pidx and APP_POOL["_FINAL_STAGE"] then
	local ok = plan.mount( pidx )
	only.log("I", string.format("mount pidx %s %s", pidx, ok))
	end
	]]--

end


function handle( jo )

	print( scan.dump(jo) )
	local accountID = jo["ARGS"]["data"]["accountID"]
	local info_table = jo["ARGS"]["data"]
	if not info_table then
		only.log('E',"info_table is nil")
	end
	weather.work(accountID,info_table)
	print( scan.dump(jo) )
	print("jizhong")
end

