-- auth: baoxue
-- time: 2014.04.27

local utils = require('utils')
local only = require('only')
local APP_POOL = require('pool')
local redis_api = require('redis_pool_api')
local mysql_api = require('mysql_pool_api')
local cjson = require('cjson')
local link = require('link')
local init_data = require("init_data")


module('l_collect', package.seeall)

function bind()
	return '["collect", "accountID"]'
end

function match()
	return true
end

function work()
        --[[
	local accountID = APP_POOL["OUR_BODY_TABLE"]["accountID"]
	local timestamp = os.time()
	local key = accountID..':configTimestamp'
	local ok, cfg_time = init_data.lru_hash_cache_get("private",accountID,key)
	if ok and cfg_time then
		local key = accountID..':currentOnlineTime'
		local value = timestamp - cfg_time
		value = tostring(value)
		local value_len = # value
		init_data.lru_hash_cache_set("private_hash",accountID,key, value, value_len)
        end
	local log_string  = "==============l_collect["..accountID.."]=============="
	only.log('D',log_string)
        ]]--

end
