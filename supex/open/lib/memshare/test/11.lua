
local luasignal = require("luasignal")

repeat
	local memshare = require('memshare')
	local ok,err = pcall(memshare.push, "ngx_logs", "ABCDEFGHIJKMNOPQRSTUVWXYZ\n")
	if not ok then	
		print(err)
	end

	--luasignal.send( 11611 )
until false
