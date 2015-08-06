local only = require('only')
local pool = require('pool')
local supex = require('supex')

module('w_xxx', package.seeall)

--[[
function handle()
	local name = "w_xxx"
	local mode = "push"
	local ok = supex.diffuse("/" .. name, "{}", (mode == "push") and true or false)
	if not ok then
		only.log("E", "forward msmq msg failed!")
	end
end
--]]
