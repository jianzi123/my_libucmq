local serv_name = app_lua_get_serv_name()

local path_list ={
	"crzpt_lua/core/?.lua;",
	string.format("crzpt_lua/%s/deploy/?.lua;", serv_name),
	string.format("crzpt_lua/%s/code/?.lua;", serv_name),
	
	"../../open/lib/?.lua;",
	"../../open/apply/?.lua;",
	"../../open/spxonly/?.lua;",
	"../../open/linkup/?.lua;",
	"../../open/public/?.lua;",

	"open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
