module("CONFIG_LIST1")


OWN_LIST = {
	["l_f_continuous_driving_mileage"] = {
		["bool"] = {
			["is_continuous_driving_mileage_point"] = {
				["increase"] = 10,
				["idx_key"] = "continuousDrivingMileage",
			},
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["app_task_forward"] = {
				["app_uri"] = "p2p_continuous_driving_mileage_remind",
			},
		},
	},
	["l_f_fetch_4_miles_ahead_poi"] = {
		["bool"] = {
			["is_4_miles_ahead_have_poi"] = {
				["type_delay"] = 300,
			},
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["app_task_forward"] = {
				["app_uri"] = "p2p_fetch_4_miles_ahead_poi",
			},
		},
	},
	["l_f_over_speed"] = {
		["bool"] = {
			["is_over_speed"] = {
				["speed"] = 125,
			},
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["app_task_forward"] = {
				["app_uri"] = "p2p_over_speed",
			},
		},
	},
	["l_f_weather_forcast"] = {
		["bool"] = {
			["is_weather_forcast"] = {
				["type_delay"] = 300,
			},
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["app_task_forward"] = {
				["app_uri"] = "p2p_weather_forcast",
			},
		},
	},
	["l_f_home_offsite"] = {
		["bool"] = {
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "fixed_time",
				["fix_num"] = 1,
				["delay"] = 180,
			},
		},
		["work"] = {
			["app_task_forward"] = {
				["app_uri"] = "p2p_offsite_remind",
			},
		},
	},
	["l_f_fatigue_driving"] = {
		["bool"] = {
			["drive_online_point"] = {
				["increase"] = 3600,
				["idx_key"] = "driveOnlineHoursPoint",
			},
		},
		["ways"] = {
			["cause"] = {
				["trigger_type"] = "every_time",
				["fix_num"] = 1,
				["delay"] = 0,
			},
		},
		["work"] = {
			["app_task_forward"] = {
				["app_uri"] = "appcenterApply.json?app_name=a_d_fatigue_driving&app_mode=alone",
			},
		},
	},
}
