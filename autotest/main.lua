local skynet = require "skynet"
local xunit = require "testsuite"

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
	testsuite:add("startwatchdog", sucess and watchdog, watchdog)
	testsuite:add("testdemo1", false, "test!")
	testsuite:add("testdemo2", true, "test!")

	testsuite:save("xunit_results.xml")
	os.exit()
end)

