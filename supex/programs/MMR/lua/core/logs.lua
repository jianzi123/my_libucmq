local APP_CFG = require('cfg')

module("logs", package.seeall)

OWN_LOG_POOL = {}

function open_log( name )
	if OWN_LOG_POOL[ name ] then
		OWN_LOG_POOL[ name ]:close()
	end
	OWN_LOG_POOL[ name ] = assert(io.open( string.format('%s/%s_%s.log', APP_CFG["OWN_INFO"]["DEFAULT_LOG_PATH"], name, os.date('%Y%m')), "a" ))
end

function init_log( list )
	for _,name in pairs(list or {}) do
		open_log( name )
	end
end

function rfsh_log( )
	for name in pairs(OWN_LOG_POOL) do
		open_log( name )
	end
end

function free_log( name )
	if OWN_LOG_POOL[ name ] then
		OWN_LOG_POOL[ name ]:close()
		OWN_LOG_POOL[ name ] = nil
	end
end

function exit_log( )
	for name in pairs(OWN_LOG_POOL) do
		free_log( name )
	end
end
