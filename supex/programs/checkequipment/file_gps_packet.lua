--
--版权声明：无
--文件名称：file_gps_packet.lua
--创建者：王张彦
--创建日期：2015.5.9
--文件描述：读取一个文件数据，拼接成一个GPS数据包
--历史记录：无
module('file_gps_packet', package.seeall)


--名称：str_split
--功能：把一个字符串拆成多个字符串
--参数：s一个字符串，c拆分字符串的分割符
--返回值：字符串表
--修改：无,从其他的文件复制的函数
local function str_split(s, c)
	if not s then 
		return nil
	end
  
	local m = string.format("([^%s]+)", c)
		local t = {}
		local k = 1
	for v in string.gmatch(s, m) do
		t[k] = v
		k = k + 1
	end
	return t
end


--名称：file_gps
--功能：读取一个文件数据，拼接成一个GPS数据包
--参数：无
--返回值：GPS数据包组成的表
--修改：新生成函数，王张彦，2015.5.9
function file_gps()

	local tab 	= {}
	--存储文件每行的表
	local packet	={}
	--存储GPS数据包的表
	
	 -- local fileName= "119224671565764.txt"		
	 -- local fileName= "127915525718505.txt"	
	 -- local fileName= "167962734303548.txt"		
	 -- local fileName= "189831250336261.txt"		
	 -- local fileName= "355877419150000.txt"		
	 -- local fileName= "550787004986523.txt"	
	 -- local fileName= "699781914382669.txt"	
	 -- local fileName= "911987302423591.txt"	
	 -- local fileName= "973864360846127.txt"	
	 -- local fileName= "974205759074252.txt"
      local fileName= "123456789012345.txt"		
	--  local fileName= "1111111111.txt"		
	 -- local fileName= "911.txt"		
	 -- local fileName= "699.txt"		
	 -- local fileName= "333.txt"		
	 -- local fileName= "167.txt"		
	 -- local fileName= "6999999.txt"		
	 -- local fileName= "6999.txt"		
	 --local fileName= "3559.txt"		
	--读取一个文件
	local f = assert(io.open(fileName,'r'))
		for line in f:lines() do
			table.insert(tab,line)
		end

  		f:close() 

	for key_line = 6 ,#tab ,5 do
			
		local longitude = {}
		local altitud	= {}
		local latitude	= {}
		local GPSTime	= {}
		local speed	= {}
		local direction = {}	
		local arry	= {}
		for key = key_line ,   key_line -4 , -1 do
			if key > #tab then
				break
			end
			--每行字符串一空格为间隔，拆分成一个表
			arry = str_split(tab[key],' ')
		
			--拆分的数据表按照固定的顺序插入的相应的表中			
			table.insert(longitude,arry[6])
       			table.insert(altitud,arry[8])
			table.insert(latitude,arry[7])
			table.insert(GPSTime,arry[1])
			table.insert(speed,arry[10])
			table.insert(direction,arry[9])
		
		end
	
		local imei	= arry[4]	
 		local accountID	= arry[5]
		local tokenCode	= arry[11]
		if tokenCode == nil  then
			break
		end
		if imei == nil  then
			break
		end
		if accountID == nil  then
			break
		end
		local length	= string.len(tokenCode)		
		local token	= string.sub(tokenCode, 1, length -1)		
		
		if  #longitude < 5 then
	     		break
		end
		if  #speed < 5 then
	     		break
		end
		if  #altitud < 5 then
	     		break
		end
		if  #latitude < 5 then
	     		break
		end
		if  #direction < 5 then
	     		break
		end
		if  #GPSTime < 5 then
	     		break
		end
		--[[
		{"longitude":[118.4285932,118.4285948,118.4285993,118.4286088,118.4286278],
		 "latitude":[35.1127742,35.112832,35.1128803,35.1129217,35.112959],
		  "IMEI":"480284076177897","model":"SG900","collect":true,
		  "speed":[23,20,17,16,14],
		 "tokenCode":"s4xlmlC34X",
		"accountID":"JsFvG4plXp",
		"GPSTime":[1430380073,1430380072,1430380071,1430380070,1430380069],
		"altitude":[74,75,74,74,74],
		"direction":[182,184,190,200,213]}
		--]]
		--把表中的数据拼接成一个为字符串
		local im  = '{'..'\"'.."IMEI"..'\"'..":"..'\"'..imei..'\"'..","..'\"'.."accountID"..'\"'..":"..'\"'..accountID..'\"'..","
		local tok = '\"'.."tokenCode"..'\"'..":"..'\"'..token..'\"'.."," 
		local lon = '\"'.."longitude"..'\"'..":"..'['..longitude[1]..","..longitude[2]..','..longitude[3]..','..longitude[4]..','..longitude[5]..']'..','
		local alt = '\"'.."altitude"..'\"'..":"..'['..altitud[1]..","..altitud[2]..','..altitud[3]..','..altitud[4]..','..altitud[5]..']'..','
		local lat = '\"'.."latitude"..'\"'..":"..'['..latitude[1]..","..latitude[2]..','..latitude[3]..','..latitude[4]..','..latitude[5]..']'..','
		local gst = '\"'.."GPSTime"..'\"'..":"..'['..GPSTime[1]..","..GPSTime[2]..','..GPSTime[3]..','..GPSTime[4]..','..GPSTime[5]..']'..','
		local spd = '\"'.."speed"..'\"'..":"..'['..speed[1]..","..speed[2]..','..speed[3]..','..speed[4]..','..speed[5]..']'..','
		local dir = '\"'.."direction"..'\"'..":"..'['..direction[1]..","..direction[2]..','..direction[3]..','..direction[4]..','..direction[5]..']'..'}'

		--把所有的短串拼接成一个GPS数据包的字符串
		local str = im..tok..lon..alt..lat..gst..spd..dir
	
		table.insert(packet,str)
	end

	--for i = 1, #packet do	
	--	print(packet[i])
	--end
	return packet
end




