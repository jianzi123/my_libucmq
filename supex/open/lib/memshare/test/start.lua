

	local memshare = require('memshare')
	local ok,err = pcall(memshare.new, "ngx_logs", "ngx_logs", 1)
	if not ok then	
		print(err)
	end

