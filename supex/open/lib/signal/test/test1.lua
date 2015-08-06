local luasignal = require('luasignal')
local socket = require('socket')



local mm = 'print("xxxxxxxxxxx\\n")'
local nn = 'print("123456789\\n")'
luasignal.install(mm, 200)
--luasignal.install(nn)
repeat
	print("1111111111111111\n")
	socket.sleep(20)
until false
