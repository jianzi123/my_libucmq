local LIB_CJSON = require('cjson')
local APP_CFG = require('cfg1')
local APP_CFG_KEYWORDS = APP_CFG["slotlist"]
local APP_CFG_KEY_LEN = APP_CFG["OWN_INFO"]["key_len"]
local APP_CFG_BIT_STEP = APP_CFG["OWN_INFO"]["bit_step"]

local os = require('os')
local io = require('io')
local pcall = pcall
local print = print
local assert = assert
local concat = table.concat

local BASE = _G

local APP_MATCH_MAPINIT = _G.app_lua_mapinit
local APP_MATCH_CONVERT = _G.app_lua_convert
local APP_MATCH_REVERSE = _G.app_lua_reverse
local APP_MATCH_IFMATCH = _G.app_lua_ifmatch

module("pool1")

_FINAL_STAGE	= false
_SOCKET_HANDLE	= 0
_TASKER_SCHEME	= 0
--<[===============================USER DATA===============================]>--
OUR_BODY_DATA		= nil
OUR_BODY_TABLE		= {}
OUR_URI_ARGS		= nil
OUR_URI_TABLE		= {}

--<[===============================LOGS DATA===============================]>--
OWN_APPLY_NAME		= "access"
OWN_FETCH_NAME		= "manage"
OWN_MERGE_NAME		= "manage"

OWN_EXACT_NAME		= "exact"
OWN_LOCAL_NAME		= "local"
OWN_WHOLE_NAME		= "whole"
OWN_ALONE_NAME		= "alone"

OWN_APP_NAME            = OWN_APPLY_NAME
OWN_APP_MODE		= OWN_APPLY_NAME

--<[===============================POOL DATA===============================]>--
--> every member has one bit
OWN_MAP_BITS = APP_CFG_KEY_LEN * APP_CFG_BIT_STEP
OWN_MAP_DATA = {}
OWN_MAP_NUMB = {}
OWN_MAP_CHAR = {}


function init_map( )
	for i=1, OWN_MAP_BITS do
		OWN_MAP_DATA[ i ] = nil
		OWN_MAP_NUMB[ i ] = 0
		OWN_MAP_CHAR[ i ] = '0'
	end
end

function set_map( key, val )
	local idx = APP_CFG_KEYWORDS[ key ]
	if not idx then print( key .. " is nil") end
	OWN_MAP_DATA[ idx ] = val
	OWN_MAP_NUMB[ idx ] = 1
	OWN_MAP_CHAR[ idx ] = '1'
end

function reset_map( )
	for i=1, OWN_MAP_BITS do
		OWN_MAP_DATA[ i ] = nil
		OWN_MAP_NUMB[ i ] = 0
		OWN_MAP_CHAR[ i ] = '0'
	end
end

--<[===============================EXACT FUNCTION===============================]>--
--------->for exact method<-----------
OWN_SEED = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
  'u', 'v', 'w', 'x', 'y', 'z',
  '-', '+'
} --64
OWN_HEX = {1,2,4,8,16,32}
OWN_KEY = {}
--------------------------------------
function reckon_exact_match_word( )
	local step = 0
	local bits = 0
	for i=1, APP_CFG_KEY_LEN do
		step = (i - 1) * APP_CFG_BIT_STEP	-->[32 hexadecimal] -->4 is also ok [16 hexadecimal]
		bits = 0
		for j=1, APP_CFG_BIT_STEP do
			bits = bits + OWN_MAP_NUMB[ step + j ] * OWN_HEX[j]
		end
		OWN_KEY[i] = OWN_SEED [ bits + 1 ]-->seed tablebegin from 1
	end
	return concat(OWN_KEY)
end
function create_exact_match_word( info )
	local ok,keys = pcall(LIB_CJSON.decode, info)
	if not ok then
		print("error json info!")
		return
	end
	for i=1,#keys do
		set_map( keys[i], nil )
	end
	local result = reckon_exact_match_word()
	reset_map()
	return result
end
--<[===============================LOCAL FUNCTION===============================]>--
--------->for local method<-----------
OWN_BUF = 0
--------------------------------------
function init_local_match()
	OWN_BUF = APP_MATCH_MAPINIT(OWN_MAP_BITS)
end
function reckon_local_match_word( )
	return APP_MATCH_CONVERT( OWN_BUF, concat(OWN_MAP_CHAR) )
end
function reverse_local_match_word( )
	return APP_MATCH_REVERSE( OWN_BUF, concat(OWN_MAP_CHAR) )
end
function check_local_match(dst, src )
	return APP_MATCH_IFMATCH(dst, src)
end
function create_local_match_word( info )
	local ok,keys = pcall(LIB_CJSON.decode, info)
	if not ok then
		print("error json info!")
		return
	end
	for i=1,#keys do
		set_map( keys[i], nil )
	end
	local result = reckon_local_match_word()
	reset_map()
	return result
end
