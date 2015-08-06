local APP_UTILS = require('utils')
local APP_CONFIG_LIST = require('CONFIG_LIST')

module("BOOL_FUNC_LIST", package.seeall)

--不能有空格
OWN_HINT = {
	-->> poistion
	judge_position = {
		pos_type = "highway,urban"
	},
	--[[
	mileage_is_more_than = {
	time_type = "ontime,total"
	},
	]]--
	drive_mileage_point = {
		idx_key = "driveOnlineMileagePoint,highwayTrafficMileagePoint,urbanTrafficMileagePoint"
	},
	drive_online_point = {
		idx_key = "driveOnlineHoursPoint"
	},
	mon_drive_mileage_point = {
		idx_key = "monDriveOnlineMileagePoint,monHighwayTrafficMileagePoint,monUrbanTrafficMileagePoint"
	},

	---power_on for mon_drive mileage
	power_on_mon_drive_mileage_point = {
		idx_key = "monDriveOnlineMileagePoint,monHighwayTrafficMileagePoint,monUrbanTrafficMileagePoint"
	},
	---- limit speed
	is_limit_speed_remind = {
		idx_key = "isLimitSpeedRemind"
	},

	-- continuous driving 
	is_continuous_driving_mileage_point = {
		idx_key = "continuousDrivingMileage"
	},
	is_over_speed = {
		speed = 125,
	},
}

OWN_ARGS = {
	-->> poistion
	judge_position = {
		pos_type = 'highway',
	},
	--[[
	mileage_is_more_than = {
	mileage = 100000,
	time_type = 'ontime',
	},
	]]--
	-->> time
	drive_online_point = {
		increase = 60*60,
		idx_key = "driveOnlineHoursPoint"
	},
	drive_online_time_period = {
		start_time = 2*60*60,
		stop_time = 3*60*60,
	},
	check_time_is_between_in = {
		time_start = 23*60*60,
		time_end = 4*60*60
	},
	driving_pattern_recognition = {
	},
	-->> speed
	check_speed_is_less_than = {speed = 60},
	check_speed_is_more_than = {speed = 130},
	-->> direction
	drive_mileage_point = {
		increase = 30,
		idx_key = "driveOnlineMileagePoint"
	},
	mon_drive_mileage_point = {
		base_mileage = 300,
		idx_key = "monDriveOnlineMileagePoint"
	},
	-----power_on for mon_drive_mileage
	power_on_mon_drive_mileage_point = {
		base_mileage = 300,
		idx_key = "monDriveOnlineMileagePoint"
	},
	is_limit_speed_remind = {
		idx_key = "isLimitSpeedRemind"
	},

	-- continueous driving
	is_continuous_driving_mileage_point = {
		increase = 10,
		idx_key = "continuousDrivingMileage"
	},
	is_over_speed = {
		speed = 125,
	},

	-- weather_forcast
}

OWN_LIST = {
	-->> poistion
	judge_position = function( app_name, key )
		return string.format('APP_JUDGE.judge_position("%s")', app_name)
	end,
	is_road_change = function( app_name, key )
		return string.format('APP_JUDGE.is_road_change("%s")', app_name)
	end,
	is_off_site = function( app_name, key )
		return string.format('APP_JUDGE.is_off_site("%s")', app_name)
	end,
	--[[
	is_at_poi = function( app_name, key )
	return string.format('APP_JUDGE.is_at_poi("%s")', app_name)
	end,
	mileage_is_more_than = function( app_name, key )
	return string.format('APP_JUDGE.mileage_is_more_than("%s")', app_name)
	end,
	]]--

	-->> time
	drive_online_point = function( app_name, key )
		return string.format('APP_JUDGE.drive_online_point("%s")', app_name)
	end,
	drive_online_time_period = function( app_name, key )
		return string.format('APP_JUDGE.drive_online_time_period("%s")', app_name)
	end,
	check_time_is_between_in = function (  app_name, key  )
		return string.format('APP_JUDGE.check_time_is_between_in("%s")', app_name)
	end,
	driving_pattern_recognition  = function (  app_name, key  )
		return string.format('APP_JUDGE.driving_pattern_recognition  ("%s")', app_name)
	end,
	-->> speed
	check_speed_is_less_than = function( app_name, key )
		return  string.format('APP_JUDGE.check_speed_is_less_than("%s")', app_name)
	end,
	check_speed_is_more_than = function( app_name, key )
		return  string.format('APP_JUDGE.check_speed_is_more_than("%s")', app_name)
	end,

	-->> direction
	drive_mileage_point = function( app_name, key )
		return string.format('APP_JUDGE.drive_mileage_point("%s")', app_name)
	end,
	mon_drive_mileage_point = function( app_name, key )
		return string.format('APP_JUDGE.mon_drive_mileage_point("%s")', app_name)
	end,
	power_on_mon_drive_mileage_point = function( app_name, key )
		return string.format('APP_JUDGE.power_on_mon_drive_mileage_point("%s")', app_name)
	end,
	is_limit_speed_remind = function( app_name, key )
		return string.format('APP_JUDGE.is_limit_speed_remind ("%s")', app_name)
	end,
	is_continuous_driving_mileage_point = function( app_name, key )
		return string.format('APP_JUDGE.is_continuous_driving_mileage_point("%s")', app_name)
	end,
	is_over_speed = function( app_name, key )
		return string.format('APP_JUDGE.is_over_speed("%s")', app_name)
	end,

}
