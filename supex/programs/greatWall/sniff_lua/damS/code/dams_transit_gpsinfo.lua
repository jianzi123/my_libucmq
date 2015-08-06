local only  = require('only')
local pool  = require('pool')
local supex = require('supex')
local utils = require('utils')
local link  = require('link')
local cjson = require('cjson')
local http_api = require('http_short_api')

module('dams_transit_gpsinfo', package.seeall)


local MAX_TIMEOUT = 120 

function handle()

	only.log('D',"==============enter dams_transit_gpsinfo handle=========")

	local data = pool["OUR_BODY_DATA"]

	local ok, tab = pcall(cjson.decode , data)

	if ok and tab then

		if tab["GPSTime"]	 and #tab["GPSTime"] > 0 and  os.time() - tonumber(tab['GPSTime'][1]) < MAX_TIMEOUT then

			local host_info = link['OWN_DIED']['http']['roadRank']
			local request = utils.compose_http_json_request( host_info ,'roadRank', nil , data)
			http_api.http_ex( host_info , request, false,'ROADRANK_SERVER',15)

			local host_info = link['OWN_DIED']['http']['driView']
			local request = utils.compose_http_json_request( host_info ,'driviewApply.json', nil , data)
			http_api.http_ex( host_info, request, false,'DRIVIEW_SERVER',15)

			local host_info = link['OWN_DIED']['http']['MDDS']
			local request = utils.compose_http_json_request( host_info ,'mapapi/v3/MDDS', nil , data)
			http_api.http_ex( host_info, request, false,'MDDS_SERVER',15)
		else
			only.log('E', string.format("==============dams_transit_gpsinfo abandon gps data=========\r\n%s",data) )
		end

	end

	only.log('D',"==============leave dams_transit_gpsinfo handle=========")
end

