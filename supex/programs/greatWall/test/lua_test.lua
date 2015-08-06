
package.cpath = "../../../open/lib/?.so;" .. package.cpath
local socket = require('socket')
local data = '*3\r\n$6\r\nLPUSHX\r\n$1\r\n0\r\n$77\r\nPOST /driviewApply.json HTTP/1.0\r\nHost: 127.0.0.1:8888\r\nConnection: close\r\n\r\n\r\n'

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
        res, err, partial = tcp:receive('*l')
	print(res, err, partial)
   	tcp:close()
	return res
end

for i=1,10000 do
	local ok, ret = pcall(luatcp, {host="127.0.0.1", port=4210}, data)
end
