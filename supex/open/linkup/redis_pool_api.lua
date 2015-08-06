local socket = require('socket')
local redis = require('redis')
local only = require('only')
local scan = require('scan')
local conhash = require('conhash')
local link = require('link')
local APP_LINK_REDIS_LIST = link["OWN_POOL"]["redis"]


module('redis_pool_api', package.seeall)

local MAX_RECNT = 3
local MAX_DELAY = 20
local OWN_REDIS_POOLS = {}

local function new_connect(memb)
	local nb = 0
	memb["rcnt"] = (memb["rcnt"] or 0)
	repeat
		nb = nb + 1
		ok,memb["sock"] = pcall(redis.connect, memb["host"], memb["port"])
		if not ok then
			memb["rcnt"] = memb["rcnt"] + 1
			socket.select(nil, nil, 0.05 * ((memb["rcnt"] >= MAX_DELAY) and MAX_DELAY or memb["rcnt"]))
			only.log("E", memb["sock"])
			only.log("I", string.format('REDIS: %s:%d | RECNT: %d |---> Tcp:connect: FAILED!', memb["host"], memb["port"], memb["rcnt"]))
			memb["sock"] = nil
		end
		if nb >= MAX_RECNT then
			return false
		end
	until memb["sock"]
	only.log("I", string.format('REDIS: %s:%d | RECNT: %d |---> Tcp:connect: SUCCESS!', memb["host"], memb["port"], memb["rcnt"]))
	memb["rcnt"] = 0
	return true
end


local function fetch_pool(redname, hashkey)
	local list = OWN_REDIS_POOLS[ redname ]
	if not list then
		only.log("E", "NO redis named <--> " .. redname)
		return nil
	end
	if #list == 0 then
		only.log("E", "Empty redis named <--> " .. redname)
		return nil
	end
	if not hashkey then
		only.log("E", "hashkey is not ok!")
		return nil
	end
	local i = 1
	if #list > 1 then
		local node = conhash.lookup(list["root"], hashkey)
		only.log("D", string.format("REDNAME %s HASHKEY %s CONHASH TO %s", redname, hashkey, node))
		i = list[node]
	end
	local memb = list[i]
	if not memb["sock"] then
		if not new_connect( memb ) then
			return nil
		end
	end
	return memb
end

local function flush_pool(memb)
	if memb["sock"] then
		memb["sock"].network.socket:close()
		memb["sock"] = nil
	end
	return new_connect(memb)
end


local function redis_cmd(memb, cmds, ...)
	----------------------------
	-- start
	----------------------------
	cmds = string.lower(cmds)
	----------------------------
	-- API
	----------------------------
	local stat,ret
	local cnt = true
	local index = 0
	repeat
		if cnt then
			stat,ret = pcall(memb["sock"][ cmds ], memb["sock"], ...)
		end
		if not stat then
			local l = string.format("%s |--->FAILED! %s . . .", cmds, ret)
			only.log("E", l)
			cnt = flush_pool(memb)
		end
		index = index + 1
		if index >= MAX_RECNT then
			local l = string.format("do %s rounds |--->FAILED! this request failed!", MAX_RECNT)
			only.log("E", l)
                        assert(false, l)
		end
	until stat
	----------------------------
	-- end
	----------------------------
	return ret
end

function init( )
	for name in pairs( APP_LINK_REDIS_LIST ) do
		local save = {}
		OWN_REDIS_POOLS[ name ] = save
		local list = APP_LINK_REDIS_LIST[name]
		if #list == 0 then
			local memb = {
				mode = "M",
				host = list["host"],
				port = list["port"],
				sock = nil,
				rcnt = 0,
                                vnode = 0
			}
			local ok = new_connect( memb )
			print( string.format("|-------%s redis pool 1/1 init-------> %s", name, (ok and "OK!" or "FAIL!")) )
			table.insert(save, memb)
		else
			local root = conhash.init()
			save["root"] = root
			for i, info in ipairs(list) do
				local memb = {
					mode = info[1],
					host = info[2],
					port = info[3],
                                        vnode = info[4],
					sock = nil,
					rcnt = 0
				}
				local ok = new_connect( memb )
				print( string.format("|-------%s redis pool %d/%d init-------> %s", name, i, #list, (ok and "OK!" or "FAIL!")) )
				table.insert(save, memb)
				local node = string.format("%s:%d", memb["host"], memb["port"])
				conhash.set(root, node, memb["vnode"])
				save[node] = i
			end
		end
	end
        only.log("S", string.format('[OWN_REDIS_POOLS:%s]', scan.dump(OWN_REDIS_POOLS)))
end

-->|	local args = {...}
-->|	local cmd, kv1, kv2 = unpack(args)
function cmd(redname, hashkey, ...)
	-->> redname, hashkey, cmd, keyvalue1, keyvalue2, ...
	-->> redname, hashkey, {{cmd, keyvalue1, keyvalue2, ...}, {...}, ...}

	local memb = fetch_pool(redname, hashkey)
	if not memb then
		return false,nil
	end
	
	local stat,ret,err
	if type(...) == 'table' then
		ret = {}
		for i=1,#... do
			if type((...)[i]) ~= 'table' then
				only.log("E", "error args to call redis_api.cmd(...)")
				break
			end

			stat,ret[i] = pcall(redis_cmd, memb, unpack((...)[i]))

			if not stat then err = ret[i] break end
		end
	else
		stat,ret = pcall(redis_cmd, memb, ...)

		if not stat then err = ret end
	end

	if not stat then
		only.log("E", "failed in redis_cmd " .. tostring(err))
		return false,nil
	end
	return true,ret
end
