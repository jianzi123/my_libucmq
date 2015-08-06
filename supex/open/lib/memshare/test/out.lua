
local luasignal = require("luasignal")

repeat
	local memshare = require('memshare')
	local ok,err = pcall(memshare.pop, "ngx_logs")
	if not ok then	
		print(err)
	end

	--luasignal.send( 11611 )
until false
