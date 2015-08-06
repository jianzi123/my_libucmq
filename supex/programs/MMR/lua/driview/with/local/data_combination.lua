local APP_POOL  = require("pool")

module("data_combination",package.seeall)



local list = {
    ["l_f_continuous_driving_mileage"] = nil,
    ["l_f_weather_forcast"]    = nil,
    ["l_f_home_offsite"] = nil,
    ["l_f_fatigue_driving"] = nil,
    ["l_f_over_speed"] = nil,
}
--设置模块私有参数到表中
function set_data(app_name,data)
    list[app_name] = data

end
 

--公有信息和模块产生的数据放到table中，并传给对ptop中对应的模块，减少redis调用
function info_combination(app_name)
    local lon       = pool["OUR_BODY_TABLE"]["longitude"] and pool["OUR_BODY_TABLE"]["longitude"][1]
    local lat       = pool["OUR_BODY_TABLE"]["latitude"] and pool["OUR_BODY_TABLE"]["latitude"][1]
    local speed     = pool["OUR_BODY_TABLE"]["speed"] and pool["OUR_BODY_TABLE"]["speed"][1]
    local direction = pool["OUR_BODY_TABLE"]["direction"] and pool["OUR_BODY_TABLE"]["direction"][1]
    local altitude  = pool["OUR_BODY_TABLE"]["altitude"] and pool["OUR_BODY_TABLE"]["altitude"][1]
    local accountID = pool["OUR_BODY_TABLE"]["accountID"]
    local collect   = pool["OUR_BODY_TABLE"]["collect"] 
    if not collect then
        only.log('E',"pool is nil:function info_combination")
        return false
    end
    local info_table = {}
    info_table["accountID"] = accountID
    info_table["lon"]       = lon
    info_table["lat"]       = lat
    local module_info = list[app_name]
    info_table["private_data"] = module_info 
    return info_table
end
