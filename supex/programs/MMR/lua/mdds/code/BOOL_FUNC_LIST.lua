local APP_UTILS = require('utils')
local APP_CONFIG_LIST = require('CONFIG_LIST')

module("BOOL_FUNC_LIST", package.seeall)

--不能有空格
OWN_HINT = {
}

OWN_ARGS = {
	-->> poistion
	is_4_miles_ahead_have_poi = {
		type_delay = 300,
	},
}

OWN_LIST = {
	-->> poistion
	is_4_miles_ahead_have_poi = function( app_name, key )
		return string.format('APP_JUDGE.is_4_miles_ahead_have_poi("%s")', app_name)
	end,
}
