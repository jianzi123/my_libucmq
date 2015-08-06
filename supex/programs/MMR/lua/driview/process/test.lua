local only = require('only')

module("test", package.seeall)


function handle()
	print(">>>><<<<<")
	only.log('S', "1111111111111111111")
	only.log('S', "%s", "")
	only.log('S', "%s=%d", nil, 5)
	only.log('S', "100%%")
end
