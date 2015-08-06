local socket = require('socket')

local data = 'GET / HTTP/1.0\r\nUser-Agent: curl/7.32.0\r\nHost: www.baidu.com\r\nAccept: */*\r\n\r\n'


function http(srv)
	local tcp = socket.tcp()
	local ok = tcp:connect(srv["host"], srv["port"])
	if not ok then error(string.format('Fail to connect to %s:%s', srv["host"], srv["port"]))  return end

	local now = socket.gettime()
	print(now)
	
	tcp:send( data )
	res = tcp:receive('*a')
	print(res)
	tcp:close()
end


http({host= "www.baidu.com", port =80})
