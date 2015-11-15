local skynet = require "skynet"

local max_client = 64

local function log_test_result(testcase, sucess, errormsg)
	local cmd = string.format("appveyor AddTest %s -Outcome %s", testcase, sucess and "Passed" or "Failed")
	if errormsg then
		cmd = cmd .. " -StdErr " .. errormsg
	end
	print(cmd)
	pcall(function () os.execute(cmd) end)
end

skynet.start(function()
	local sucess, watchdog = pcall(function ()
		local watchdog = skynet.newservice("watchdog")
		skynet.call(watchdog, "lua", "start", {
			port = 8888,
			maxclient = max_client,
			nodelay = true,
		})
		return watchdog;
	end)

	log_test_result("start watchdog", sucess and watchdog, watchdog)
	os.exit()
end)
