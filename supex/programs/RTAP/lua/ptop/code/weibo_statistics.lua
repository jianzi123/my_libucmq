module('weibo_statistics',package.seeall)


function Statistics(bizid, appkey)  
        if bizid and appkey then
                local time = os.time()
                local travelID  = nil
                local ok, imei =  redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:IMEI", accountID))
                if ok and imei then
                        ok, travelID = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:travelID", imei))
                end
                --local ok_date, cur_date = redis_api.cmd('private',APP_POOL["OUR_BODY_TABLE"]["accountID"],'get', string.format("%s:speedDistribution", accountID))
                --if not ok_date or not cur_date then
                local cur_date = os.date("%Y%m")
                if ok and imei and accountID and travelID and cur_date then
                        local datacore_statistics_var_key = accountID .. ":" .. travelID .. ":" .. appKey ..":" .. cur_date
                        local datacore_statistics_var_value = bizid .. ":" .. time
                        redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'sadd', datacore_statistics_var_key, datacore_statistics_var_value)
                        redis_api.cmd('dataCoreRedis',APP_POOL["OUR_BODY_TABLE"]["accountID"],'expire', datacore_statistics_var_key, 48*3600)
                end
        end
end