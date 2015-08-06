
local libhttps = require("libhttps")

local res = libhttps.https("sso.cisco.com", 443, "GET /autho/forms/CDClogin.html HTTP/1.0\r\n\r\n", 43)
print(res)
