

	local memshare = require('memshare')
	--local ok = memshare.push( "ngx_logs", nil)
	--if ok then print(ok) end
	print("----------")
	local ok,err = pcall(memshare.push,"ngx_logs", nil)
	if not ok then print(err) end

