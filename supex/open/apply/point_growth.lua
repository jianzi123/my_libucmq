-- auth: coanor
-- date: Wed Aug 28 20:40:57 CST 2013

local common_cfg = require 'cfg'
local only = require('only')
local sha = require 'sha1'
local cutils = require 'cutils'
local utils = require 'utils'
local http = require('supex').http
local string = require 'string'
local secret = common_cfg.secret
local agent = common_cfg.agent
local link = require 'link'
local point_server = link.OWN_DIED.api.point_server

module('point_growth')

local active_point_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local active_point_body_fmt = '{"action":"%s","agent":"%s","serviceType":"%s","sign":"%s","accountID":"%s"}'
local active_point_sign_fmt = 'accountID%saction%sagent%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type}
function active(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(active_point_sign_fmt,
        api_data.accountID, 'activateMirrtalk', agent, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)

    local body = string.format(active_point_body_fmt,'activateMirrtalk', 
        agent, api_data.service_type, string.upper(sign), api_data.accountID)

    local req = string.format(active_point_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)
    only.log('D', req)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end


local poweron_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local poweron_body_fmt = '{"action":"%s","agent":"%s","serviceType":"%s","sign":"%s","accountID":"%s"}'
local poweron_sign_fmt = 'accountID%saction%sagent%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type}
function poweron(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(poweron_sign_fmt, 
        api_data.accountID, 'poweronAward', agent, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)

    local body = string.format(poweron_body_fmt,
        'poweronAward', agent, api_data.service_type, string.upper(sign), api_data.accountID)

    local req = string.format(poweron_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)
    only.log('D', req)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end

local drive_mileage_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local drive_mileage_body_fmt = '{"accountID":"%s","action":"%s","agent":"%s","serviceType":"%s","sign":"%s"}'
local drive_mileage_sign_fmt = 'accountID%saction%sagent%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type}
function drive_mileage(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(drive_mileage_sign_fmt,
        api_data.accountID, 'drivedMileage', agent, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)

    local body = string.format(drive_mileage_body_fmt,
        api_data.accountID, 'drivedMileage', agent, api_data.service_type, string.upper(sign))

    local req = string.format(drive_mileage_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)
    only.log('D', req)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end

local yes_accepted_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local yes_accepted_body_fmt = '{"action":"%s","agent":"%s","bizid":"%s","serviceType":"%s","sign":"%s"}'
local yes_accepted_sign_fmt = 'action%sagent%sbizid%ssecret%sserviceType%s'

-- api_data  = {service_type, bizid}
function yes_accepted(api_data)

    local bizid = api_data.bizid
    if bizid == nil then
        return nil, nil
    end

    local sign_str = string.format(yes_accepted_sign_fmt,
        'yesAccepted', agent, bizid, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)
    local body = string.format(yes_accepted_body_fmt,
        'yesAccepted', agent, bizid, api_data.service_type, string.upper(sign))

    local req = string.format(yes_accepted_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)
    only.log('D', req)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end

local pass_by_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local pass_by_body_fmt = '{"action":"%s","agent":"%s","serviceType":"%s","sign":"%s","accountID":"%s"}'
local pass_by_sign_fmt = 'accountID%saction%sagent%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type}
function pass_by(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(pass_by_sign_fmt,
        api_data.accountID, 'daokePassby', agent, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)

    local body = string.format(pass_by_body_fmt,
        agent, api_data.service_type, string.upper(sign), api_data.accountID)

    local req = string.format(pass_by_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)
    only.log('D', req)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end

local send_yes_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local send_yes_body_fmt = '{"action":"%s","agent":"%s","serviceType":"%s","sign":"%s","accountID":"%s"}'
local send_yes_sign_fmt = 'accountID%saction%sagent%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type}
function send_yes(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(send_yes_sign_fmt,
        api_data.accountID, 'sendYes', agent, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)

    local body = string.format(send_yes_body_fmt,
        'sendYes', agent, api_data.service_type, string.upper(sign), api_data.accountID)

    local req = string.format(send_yes_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)
    only.log('D', req)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end

local award_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local award_body_fmt = '{"accountID":"%s","agent":"%s","action":"%s","serviceType":"%s","sign":"%s","medalID":%d,"points":%d,"remarks":"%s"}'
local award_sign_fmt = 'accountID%saction%sagent%smedalID%spoints%dremarks%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type, text, points, medal}
function award(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(award_sign_fmt,
        api_data.accountID, 'consumePoints', agent, api_data.medal, api_data.points, api_data.text, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)
   
    local body = string.format(award_body_fmt,
        api_data.accountID, agent, 'consumePoints', api_data.service_type, string.upper(sign), api_data.medal, api_data.points, cutils.url_encode(api_data.text))
    only.log('D', 'body:' .. body)

    local req = string.format(award_http_fmt,
        point_server.path,  point_server.host, point_server.port, #body, body)

    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end

local tweet_http_fmt = 'POST /%s HTTP/1.0\r\nHost:%s:%d\r\nContent-Length:%d\r\n\r\n%s'
local tweet_body_fmt = '{"accountID":"%s","agent":"%s","serviceType":"%s","sign":"%s","action":"%s"}'
local tweet_sign_fmt = 'accountID%saction%sagent%ssecret%sserviceType%s'

-- api_data  = {accountID, service_type}
function tweet(api_data)

    if api_data.accountID == nil then return nil, nil end

    local sign_str = string.format(tweet_sign_fmt,
        api_data.accountID, 'tweet', agent, secret, api_data.service_type)

    local sign = sha.sha1(sign_str)

    local body = string.format(tweet_body_fmt,
        api_data.accountID, agent, api_data.service_type, string.upper(sign), "tweet")

    local req = string.format(tweet_http_fmt,
        point_server.path, point_server.host, point_server.port, #body, body)

    only.log('D', req)
    local ok, resp = http(point_server.host, point_server.port, req, #req)
    only.log('D', resp)

    return ok, resp
end


