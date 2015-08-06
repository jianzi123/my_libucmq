-- jiangzhuansheng 
-- 2014-04-29
-- mp3 to voice 

local APP_POOL	= require('pool')
local utils     = require('utils')
local supex	= require('supex')
local only      = require('only')
local msg       = require('msg')
local safe      = require('safe')
local gosay     = require('gosay')
local sha       = require('sha1')
local link	= require('link')
local redis_api = require('redis_short_api')
local dfs_api_serv = link["OWN_DIED"]["http"]["dfsapi/v2/mp32voice"]

local host_s = "s.api.aispeech.com"
local port_s = 80
local appkey_s = "139762545900025f" 
local secretkey_s = "0588ddbd719e6962375510881f648c1e" 

local post_format =  "POST /api/v3.0/score HTTP/1.0\r\n" ..
"Host:%s:%s\r\n" ..
"Content-Length:%d\r\n" ..
"Content-Type:text/plain\r\n\r\n%s" 

---- 思必驰接口发言人
local speechAnnouncer_tab = {
	syn_chnsnt_anonyf   = "anony",
	syn_chnsnt_zhilingf = "zhiling",
}
local speechAnnouncer_index = {
	syn_chnsnt_anonyf   = 0,
	syn_chnsnt_zhilingf = 1,
}

module('spx_txt_to_voice', package.seeall)

local function check_mp3_head( data_binary )
	if not data_binary then return false end
	if string.byte(data_binary,1) == 0xff then return true end
end

local function txt_to_voice(speechRate, speechAnnouncer,  speechVolume , txt)
	if not txt then return false,nil end
	if #txt < 1 then return false,nil end
	local data_format = [[{"cmd":"start","param":{"app":{"applicationId":"%s","timestamp":"%s","sig":"%s"},"audio":{"audioType":"mp3","channel": 1,"sampleRate":16000,"sampleBytes":2},"request":{ "coreType":"cn.sent.syn","speechRate":%s,"speechVolume":%d,"res": "%s","refText":"%s"}}}]]
	local timestamp = os.time()
	local sign_string = string.format("%s%s%s", appkey_s,timestamp,secretKey_s)
	local sign_result = sha.sha1(sign_string)
	local body_result = string.format(data_format,appkey_s,timestamp,sign_result,speechRate, speechVolume, speechAnnouncer, txt)
	local req = string.format(post_format, host_s, tostring(port_s) , #body_result, body_result )
	local ok,ret = supex.http(host_s, port_s, req, #req)
	if not ok then
		--only.log('D',req)
		--only.log('E',string.format('host: %s , port : %s http post get txt_voice failed!!', host_s, port_s) )
		only.log('E',string.format('[spx][txt:%s][speech failed]',txt))
		return false,nil
	end

	if not ret then
		--only.log('D',req)
		--only.log('E','http post succed but return failed!!')
		return false,nil
	end

	local data_split = string.find(ret,'\r\n\r\n')
	if not data_split then
		return false,nil
	end
	local data_head = string.sub(ret,1,data_split)
	local data_binary = string.sub(ret,data_split+4,#ret)
	if  not ( string.find(data_head,'Content%-Type:% application/octet%-stream') 
		or string.find(data_head, 'Content%-Type:% audio/mpeg' )
		or check_mp3_head(data_binary) ) then
		----'Content%-Type:% audio/mpeg'
		----'Content%-Type:% application/octet%-stream'
		--only.log('D','request succed,but get Content-Type: audio/mpeg,application/octet-stream and check_mp3_head failed!')
		return false,nil
	end
	return true,data_binary
end


function handle()
	local args = APP_POOL["OUR_BODY_TABLE"]
	if not args or not next(args) then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'args')
	end
	if not args['text'] then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'text')
	end

	args['text'] = utils.url_decode(args['text'])
	args['text'] = string.gsub(args['text'],"'","")

	if not safe.sign_check(args) then
		return true
	end

	local txt = args['text']
	if not txt or string.len(txt) < 1 or string.len(txt) > 160*3 then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'text')
	end

	---- 替换标点符号,替换双引号,避免转码错误
	txt = string.gsub(txt,',','，')
	txt = string.gsub(txt,'%.','。')
	txt = string.gsub(txt,':','：')
	txt = string.gsub(txt,"\"","")
	txt = string.gsub(txt,"\"","")
	txt = string.gsub(txt,"“","")
	txt = string.gsub(txt,"”","")


	--only.log('D',string.format("%s",txt))

	local speechRate = 1.0
	speechRate = tonumber(args['speechRate']) or 1.0
	if speechRate > 2.0 or speechRate < 0.5 then
		speechRate = 1.0 
	end
	local org_speech_rate = speechRate * 10 ----语音合成速率
	local speech_Rate_str = string.format("%.1f",speechRate)
	local speechAnnouncer = args['speechAnnouncer'] or 'anony'
	speechAnnouncer = string.lower(speechAnnouncer)


	local speechVolume = 100
	speechVolume = tonumber(args['speechVolume']) or 100
	if speechVolume < 0 or speechVolume > 200 then
		speechVolume = 100
	end

	local speech_Announcer_str = 'syn_chnsnt_anonyf'
	local is_ok = false
	for i,v in pairs(speechAnnouncer_tab) do 
		if tostring(speechAnnouncer) == tostring(v) then
			speech_Announcer_str = i
			is_ok = true
		end
	end

	if is_ok == false then
		speech_Announcer_str = 'syn_chnsnt_anonyf'
	end

	local org_speech_announcer = tonumber(speechAnnouncer_index[speech_Announcer_str]) or 0 ----语音播放风格

	---- zhiling  且未设置音量大小,则默认使用200
	if not args['speechVolume'] and org_speech_announcer == 1 then
		speechVolume = 200
	end

	--only.log('D',string.format("speechRate:%s  speechAnnouncer:%s  speechVolume:%s", speech_Rate_str, speech_Announcer_str , speechVolume))

	--------txt --->---mp3 
	local ok_status,file_binary = txt_to_voice(speech_Rate_str,speech_Announcer_str, speechVolume ,txt)
	if not ok_status or file_binary == nil or #file_binary < 1  then
		return gosay.resp_msg(msg["MSG_ERROR_REQ_ARG"], 'txt to voice')
	end
	local info = {
		appKey = args["appKey"],
		length = #file_binary,
		text = txt,
	}
	local ok, secret = redis_api.cmd('public',APP_POOL["OUR_BODY_TABLE"]["accountID"], 'hget', args['appKey'] .. ':appKeyInfo', 'secret')
	info['sign'] = utils.gen_sign(info, secret)
	local file ={
		file_name = "temp.mp3",
		data_type = "audio/MP3",
		data = file_binary
	}
	local request = utils.compose_http_form_request(dfs_api_serv, "dfsapi/v2/mp32voice", nil, info, "mmfile", file)
	local ok,data = supex.http(dfs_api_serv["host"], dfs_api_serv["port"], request, #request)
	if not ok then
		--only.log('D', request)
		--only.log('E', 'dfsapi/v2/mp32voice api call failed!!')
		only.log('D',string.format('[spx][txt:%s][dfs failed]', txt))
		return gosay.resp_msg(msg["MSG_DO_HTTP_FAILED"])
	end
	only.log('D',string.format('[spx][txt:%s][successs]', txt))
	return supex.spill(data)
end
