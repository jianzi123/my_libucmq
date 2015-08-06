local only = require('only')
local pool = require('pool')
local supex = require('supex')

module('sniff', package.seeall)


function handle()
	if pool["OUR_BODY_TABLE"] and pool["OUR_BODY_TABLE"]["APPLY"] and pool["OUR_BODY_TABLE"]["TIME"] then
		local mode = pool["OUR_BODY_TABLE"]["MODE"]
		local mode_list = {
			["push"] = 1,	--for lua VM
			["pull"] = 2,	--for lua VM

			["insert"] = 3,	--for task list
			["update"] = 4,	--for task list
			["select"] = 5,	--for task list
		}
		print("---------------")
		print(mode, mode_list[ mode ])

		if mode_list[ mode ] then
		print("+++++++++++++++++")
			local ok = supex.diffuse("/" .. pool["OUR_BODY_TABLE"]["APPLY"], pool["OUR_BODY_DATA"], pool["OUR_BODY_TABLE"]["TIME"], mode_list[ mode ])
			if not ok then
				only.log("E", "forward msmq msg failed!")
			end
		end
	end
end

