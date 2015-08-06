local only  = require('only')
local pool  = require('pool')
local supex = require('supex')
local utils = require('utils')
local link  = require('link')
local cjson = require('cjson')
local http_api = require('http_short_api')

module('dams_transit_poweron', package.seeall)


function handle()
	local req_body = pool["OUR_BODY_DATA"]

	local ok , args = pcall(cjson.decode,req_body)
	if not ok or not args then
		only.log('E',"json_decode failed!")
		return false
	end

	local accountid = args['accountID']
	if not accountid or #tostring(accountid) ~= 10 then
		accountid = args['IMEI']
	end

	local data = string.format('{"powerOn":true,"accountID":"%s","tokenCode":"%s","model":"%s"}',
					accountid,
					args['tokenCode'],
					args['model'])
	
	local host_info = link['OWN_DIED']['http']['driView']
	local request = utils.compose_http_json_request( host_info ,'driviewApply.json', nil , data)
	http_api.http( host_info, request, false)

end

