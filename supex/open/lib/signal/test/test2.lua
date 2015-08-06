local luasignal = require('luasignal')
local socket = require('socket')



local mm = 'print("xxxxxxxxxxx\\n")'
local nn = 'print("123456789\\n")'
luasignal.install(nn, 300)
--luasignal.install(nn)
repeat
	print("222222222222222222222222\n")
	socket.sleep(20)
until false
