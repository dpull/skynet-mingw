local skynet = require "skynet"
local test = require "xunit"
require "skynet.manager"

local max_client = 64

local function start_watchdog()
	local watchdog = skynet.newservice("watchdog")
	skynet.call(watchdog, "lua", "start", {
		port = 8888,
		maxclient = max_client,
		nodelay = true,
	})
	assert(watchdog)	
	return true
end

skynet.start(function()
	test("start watchdog", start_watchdog)
	skynet.abort()
end)

