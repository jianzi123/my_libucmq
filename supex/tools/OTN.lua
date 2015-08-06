local file = os.getenv("MYFILE")
local path = os.getenv("MYPATH")

function exchange(data)
	-->> log
	--[[
	data = string.gsub(data, "utils%.logLV%[%'E%'%]", "'E'")
	data = string.gsub(data, 'utils%.logLV%[%"E%"%]', "'E'")
	data = string.gsub(data, "utils%.logLV%[%'I%'%]", "'I'")
	data = string.gsub(data, 'utils%.logLV%[%"I%"%]', "'I'")
	data = string.gsub(data, "utils%.logLV%[%'D%'%]", "'D'")
	data = string.gsub(data, 'utils%.logLV%[%"D%"%]', "'D'")
	data = string.gsub(data, "utils%.logLV%[%'B%'%]", "'D'")
	data = string.gsub(data, 'utils%.logLV%[%"B%"%]', "'D'")
	data = string.gsub(data, "local% utils% %= %require%(%'utils%'%)", "local utils = require('utils')\nlocal only = require('only')")
	data = string.gsub(data, 'local% utils% %= %require%(%"utils%"%)', "local utils = require('utils')\nlocal only = require('only')")
	data = string.gsub(data, "local% utils% %= %require% %'utils%'", "local utils = require('utils')\nlocal only = require('only')")
	data = string.gsub(data, 'local% utils% %= %require% %"utils%"', "local utils = require('utils')\nlocal only = require('only')")
	data = string.gsub(data, 'utils%.log', "only.log")
	]]--
	-->> net
	--[[
	data = string.gsub(data, "local% net%_cfg% %=% require%(%'net%_config%'%)", "local gosay = require('gosay')")
	data = string.gsub(data, "net%_cfg%.go%_false", "gosay.go_false")
	data = string.gsub(data, "net%_cfg%.go%_success", "gosay.go_success")
	data = string.gsub(data, "net%_cfg%.respond%_to%_device", "gosay.respond_to_device")
	]]--
	-->> json
	--[[
	data = string.gsub(data, 'local% json% %=% require%(%"json%"%)', "local json = require('cjson')")
	data = string.gsub(data, "local% json% %=% require%(%'json%'%)", "local json = require('cjson')")
	]]--
	-->> mysql
	--[[
	data = string.gsub(data, "local% mysql%_cfg% %=% require%(%'mysql%_config%'%)", "local mysql_api = require('mysql_short_api')")
	data = string.gsub(data, 'local% mysql%_cfg% %=% require%(%"mysql%_config%"%)', "local mysql_api = require('mysql_short_api')")
	data = string.gsub(data, "mysql%_cfg%.get%_sql%_table%(", "mysql_api.get_sql_table(")
	data = string.gsub(data, "mysql%_cfg%.query%(", "mysql_api.query(")
	data = string.gsub(data, "mysql%_cfg%.close%(", "mysql_api.close(")
	]]--
	-->> handle
	--data = string.gsub(data, "\nhandle%(%)", "\nutils.safe_call( handle )")
	--data = string.gsub(data, "utils%.safe%_call", "app.safe_call")
	--[[
	data = string.gsub(data, "singly%_random", "random_singly")
	data = string.gsub(data, "new%_random", "random_string")
	]]--
	--[[
	data = string.gsub(data, "APP%_UTILS%.log", "only.log")
	data = string.gsub(data, "utils%.log", "only.log")
	]]--
	--[[
	data = string.gsub(data, "require% %'CONFIG%_LIST%'", "require 'CONFIG_LIST_WRONG'")
	data = string.gsub(data, 'local% APP%_CFG%_IDX% %=% common%_cfg%[%"OWN%_INFO%"%]%[%"idx%"%]', "")
	]]--
	--[[
	data = string.gsub(data, '%[% APP%_CFG%_IDX% %]', "")
	]]--
	--data = string.gsub(data, 'async', "supex")
	data = string.gsub(data, '        ', "\t")
	data = string.gsub(data, '    ', "\t")
	data = string.gsub(data, '[ ]+\n', "\n")
	return data
end

function old_to_new( f )
	local fd = io.open(f, 'r')
	local old = fd:read('*a')
	fd:close()
	if not old then
		-->> is dir
		return
	end

	fd = io.open(f, 'w+')
	ok,new = pcall(exchange, old)
	if not ok then
		print(new)
	end
	fd:write( ok and new or old )
	fd:close()
end

if not ((file == "") or (not file)) then
	print(file)
	old_to_new( file )
else
	os.execute( string.format('ls %s > OTN.txt', path) )
	local fd = io.open("OTN.txt", 'r')
	repeat
		local fl = fd:read('*l')
		if fl then
			local file = path .. fl
			print(file)
			old_to_new( file )
		end
	until not fl
	fd:close()
end
