local libconhash = require('libconhash')
local ok,err = pcall(libconhash.init)
if not ok then
	print(err)
end
local ok,err = pcall(libconhash.set, "titanic", 32)
if not ok then
	print(err)
end
local ok,err = pcall(libconhash.set, "terminator2018", 24)
if not ok then
	print(err)
end
local ok,err = pcall(libconhash.set, "Xenomorph", 25)
if not ok then
	print(err)
end
------------------------
local ok,err = pcall(libconhash.lookup, "baoxue1")
print(err)
local ok,err = pcall(libconhash.lookup, "baoxue2139")
print(err)
local ok,err = pcall(libconhash.lookup, "jskkkkkk2139")
print(err)
local ok,err = pcall(libconhash.lookup, "jskkkkkk2139")
print(err)
local ok,err = pcall(libconhash.lookup, "11111111111111111111")
print(err)
local ok,err = pcall(libconhash.lookup, "11111sdddddd11111xxxxxxxx111")
print(err)
local ok,err = pcall(libconhash.lookup, "baoxue21aasda39")
print(err)
local ok,err = pcall(libconhash.lookup, "baoxue21afsas39")
print(err)
------------------------
local ok,err = pcall(libconhash.destory)
if not ok then
	print(err)
end
