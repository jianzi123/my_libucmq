local luasignal = require("luasignal")
local memshare = require('memshare')
local msg = "xxxxxxxxxxxxxxxxxx"
memshare.push("ngx_logs", msg)
luasignal.send(8029)
