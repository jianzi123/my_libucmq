local socket = require('socket')
local data = '*3\r\n$3\r\nSET\r\n$8\r\npersonal\r\n$271\r\n{"appKey":1491067261,"multimediaURL":"http://127.0.0.1/productList","endTime":1530477505,"sourceID":"werwyetrtweywye","commentID":"sdsadsaa","messageType":2,"senderType":1,"content":"hello word!","level":45,"receiverAccountID":"kxl1QuHKCD","senderAccountID":"aaaaaaaaaa"}\r\n'


function luatcp(cfg, data)
        local tcp = socket.tcp()

        if tcp == nil then
                error('load tcp failed')
                return
        end
	tcp:settimeout(10)
        local ret = tcp:connect(cfg["host"], cfg["port"])
        if ret == nil then error(string.format('Fail to connect to %s:%s', cfg["host"], cfg["port"]))  return end
	
        ret = tcp:send(data)
        --res, err, partial = tcp:receive('*l')
	--print(res, err, partial)
   	tcp:close()
	return res
end

for i=1,10000 do
	local ok, ret = pcall(luatcp, {host="127.0.0.1", port=6000}, data)
end
