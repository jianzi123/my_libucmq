local serv_name = app_lua_get_serv_name()

local path_list ={
	"lua/core/?.lua;",
	string.format("lua/%s/list/?.lua;", serv_name),
	string.format("lua/%s/deploy/?.lua;", serv_name),
	string.format("lua/%s/code/?.lua;", serv_name),
	
	"../../open/lib/?.lua;",
	"../../open/apply/?.lua;",
	"../../open/spxonly/?.lua;",
	"../../open/linkup/?.lua;",
	"../../open/public/?.lua;",

	"open/?.lua;",
}

package.path = table.concat(path_list) .. package.path
package.cpath = "../../open/lib/?.so;" .. "open/?.so;" .. package.cpath
