#! /usr/local/bin/lua

local cjson = require "cjson"


conf = io.open("/root/dev_brks/brks/conf/brks.json", "r")
json = cjson.decode(conf:read("*a"))
conf:close()
