local skynet = require "skynet"
local xunit = require "xunit"

local max_client = 64

skynet.start(function()
	local testsuite_name = "skynet-mingw"
	local testsuite = xunit(testsuite_name)
	local sucess, watchdog = pcall(function ()
		local watchdog = skynet.newservice("watchdog")
		skynet.call(watchdog, "lua", "start", {
			port = 8888,
			maxclient = max_client,
			nodelay = true,
		})
		return watchdog;
	end)
	testsuite:add("start watchdog", sucess and watchdog, watchdog)
	testsuite:add("test demo1", false, "test!")
	testsuite:add("test demo2", true, "test!")

	testsuite:upload()
	os.exit()
end)

