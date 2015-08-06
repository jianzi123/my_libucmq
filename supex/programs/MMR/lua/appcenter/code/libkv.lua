local redis_api = require('redis_pool_api')
local only = require('only')

module('libkv',package.seeall)

--函 数:libkv_hash_cache_set
--功 能:数据保存到本地，同时也保存到远处redis中
--参 数:server redis服务器, hash hash关键字
--返回值:失败返回false,成功返回true
--说 明:
function libkv_hash_cache_set(server, hash, key, value)
	if not server then
		only.log('E', string.format("[Function:libkv_hash_cache_server] [server is nil] ,[key: %s], [value: %s]", key or '', value or ''))
                return false
	end
	if not key then
		only.log('E', string.format("[Function:libkv_hash_cache_set] [key is nil],[value: %s]", value or ''))
		return false
	end
	if not value then
		only.log('E', string.format('[Function:libkv_hash_cache_set]  [key:%s], [value is nil]', key))
		return false
	end
	
	

	only.log('D', string.format("[Function:libkv_hash_cache_set] [server:%s][hashkey:%s][key:%s][value:%s]", server or '', hash or '', key or '', value or ''))

	--保存到redis中,失败的话直接返回
	local ok  = redis_api.cmd(server, hash, 'set', key, value)
	if not ok then
		only.log('E',string.format("[Function:libkv_hash_cache_set] save [key:%s] [value:%s] to [redis:%s] failed",key, value, server))
		return false
	end
	--保存到本地缓存中
	local  command = 'set'  
	command= command .. ' '..key .. ' '..tostring(value) 
	ok, ret =libkv_cache_set_value(command, #command)	
	if not ok then	
		only.log('E',string.format("[Function:libkv_hash_cache_set] save [key:%s] [value:%s] to libkv failed, [reason:%s]",key, value, ret)) 
	end
        return true
end
--函 数:libkv_hash_cache_get
--功 能:从本地缓存中获取key的值，如果获取失败，则从server redis服务器上获取
--参 数:server redis服务器, hash hash时的关键字, key 要获取的键 
--返回值: 成功返回true和key的值,失败返回false
--说明:
function libkv_hash_cache_get(server, hash, key)
	if not key then
		only.log('E',string.format("[Function:libkv_hash_cache_get] [server:%s] [hash:%s][key is nil]"))  	
		return false
	end

	local command = 'get'
	command = command .. ' ' .. key
	local ok, ret = libkv_cache_get_value(command, #command)		
	if ok then
		only.log('D', string.format("[Function:libkv_hash_cache_get] get [key:%s]  from libkv success, [value:%s]", key, ret or ''))
		return true, ret
	end
	if not ok then 	
		ok, ret = redis_api.cmd(server, hash,'get', key)
		if ok and ret then
			command = 'set'
			command = command .. ' ' .. key .. ' ' ..tostring(ret)
			ok = libkv_cache_set_value(command, #command)
			if not ok then
				only.log('E',string.format("[Function:libkv_hash_cache_get] save [key:%s]  to libkv failed, [reason:%s]",key, ret or ''))
			end
			only.log('I',string.format("[Function:libkv_hash_cache_get] [key:%s] from redis [value:%s]",key, ret))
			return true, ret
		end
	end
	only.log('E',string.format("[Function:libkv_hash_cache_get] get [key:%s] failed [reaseon:%s]", key, ret or ''))
	return false
end

--函 数:libkv_cache_delete
--功 能:从libkv中删除key
--参 数:
--返回值:失败返回false,成功返回true

function libkv_cache_delete(key)
        if not key then
              only.log('E',string.format("[Function:libkv_cache_delete] [key is nil]"))  	
		return false
        end
        local command = 'del'
        command = command .. ' ' .. key
        local len = #command
        ok,ret = libkv_cache_delete_key(command, len)
        if not ok then
                only.log('E',string.format("[Function:libkv_cache_delete] delte [key:%s] failed,[reason:%s]",key, ret or ''))
                return false;
        end
        return true;
end
