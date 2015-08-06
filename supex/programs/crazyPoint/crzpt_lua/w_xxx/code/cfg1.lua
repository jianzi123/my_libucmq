module("cfg1")


keywords = {
	powerOn		= {{true},{},{},{"power_on_mon_drive_mileage_point"}},
	powerOff	= {{true},{},{},{}},
	collect		= {{true},{},{},{}},

	accountID	= {{},{},{},{}},
	IMEI		= {{},{},{},{}},
	model		= {{},{},{"SG900"},{}},
	tokenCode	= {{},{},{},{}},

	GSENSORTime	= {{},{},{},{}},
	gx		= {{},{0,2},{},{"eq","ne","gt","lt","le","ge"}},
	gy		= {{},{0,2},{},{"eq","ne","gt","lt","le","ge"}},
	gz		= {{},{0,2},{},{"eq","ne","gt","lt","le","ge"}},

	GPSTime		= {{},{},{},{"drive_online_point","drive_online_time_period","check_time_is_between_in"}},
	speed		= {{},{0},{},{"check_speed_is_less_than","check_speed_is_more_than"}},
	altitude	= {{},{0,200},{},{"eq","ne","gt","lt","le","ge"}},
	direction	= {{},{},{},{"drive_mileage_point", "mon_drive_mileage_point", "is_continuous_driving_mileage_point"}},
	position	= {{},{},{},{"is_off_site","judge_position","is_road_change","is_4_miles_ahead_have_poi", "is_limit_speed_remind", "driving_pattern_recognition", "is_over_speed", "is_weather_forcast"}},
}

workfunc = {
	["exact"] = {"app_task_forward"},
	["local"] = {"app_task_forward"},
	["whole"] = {"app_task_forward"},
	["alone"] = {},
}

ranklist = {-->other default is 0,the higher rank function will be run the first.
	["check_speed_is_less_than"]	= 99,
	["check_speed_is_more_than"]	= 99,
	["driving_pattern_recognition"]	= 89,
	["check_time_is_between_in"]	= 88,
	["drive_online_time_period"]	= 88,
	["drive_mileage_point"]		= 77,
	["power_on_mon_drive_mileage_point"]		= 72,
	["is_continuous_driving_mileage_point"]		= 71,
	["mon_drive_mileage_point"]		= 70,
	["drive_online_point"]		= 66,
	["is_off_site"]			= 55,
	["is_4_miles_ahead_have_poi"]	= 44,
	["is_weather_forcast"]	= 44,
	["judge_position"]		= 33,
	["is_over_speed"]		= 32,
	["is_limit_speed_remind"]		= 23,
	["is_road_change"]		= 22,
}

slotlist = {
	powerOn		= 1,
	powerOff	= 2,
	collect		= 3,

	accountID	= 20,
	IMEI		= 21,
	model		= 22,
	tokenCode	= 23,

	GSENSORTime	= 31,
	gx		= 32,
	gy		= 33,
	gz		= 34,

	GPSTime		= 35,
	speed		= 36,
	altitude	= 37,
	direction	= 38,
	longitude	= 39,
	latitude	= 40,
	extragps 	= 50,
}

OWN_INFO = {
	DEFAULT_APP_PATH = "./lua/driview/data/",

	LOGLV = 0,
        SYSLOGLV = false,
	DEFAULT_LOG_PATH = "./logs/",
	--OPEN_LOGS_CLASSIFY = false,
	OPEN_LOGS_CLASSIFY = true,
	
	--if current system is for real customer set "true" or "false"
	CUSTOM_ORIENTED_SYSTEM = true,


	key_len = 20,
	bit_step = 5, --> must <= 6
	--> 20 * 5 kinds
}

-- testing users accounts
TEST_ACCOUNT = {
}
