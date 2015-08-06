local APP_POOL = require('pool')
local crzpt_http = _G.crzpt_http

module("crzpt", package.seeall)

function http(host, port, data, size)
	return crzpt_http(APP_POOL["_TASKER_SCHEME"], host, port, data, size)
end
