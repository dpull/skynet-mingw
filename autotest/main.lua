local skynet = require "skynet"

local max_client = 64

local function log_test_result(testcase, sucess, errormsg)
--[[
AddTest options:
 
  -Name             - Required. The name of test.
  -Framework        - Required. The name of testing framework, e.g. NUnit, xUnit, MSTest.
  -FileName         - Required. File name containg test.
  -Outcome          - Test outcome: None, Running, Passed, Failed, Ignored, Skipped, Inconclusive, NotFound, Cancelled, NotRunnable
  -Duration         - Test duration in milliseconds.
  -ErrorMessage     - Error message of failed test.
  -ErrorStackTrace  - Error stack trace of failed test.
  -StdOut           - Standard console output from the test.
  -StdErr           - Error output from the test.

	local cmd = string.format("appveyor AddTest %s -Outcome %s", testcase, sucess and "Passed" or "Failed")
	if not sucess and errormsg then
		cmd = cmd .. " -StdErr " .. errormsg
	end
	pcall(function () os.execute(cmd) end)  
]]

	local cmd = string.format("Test %s %s", testcase, sucess and "Passed" or "Failed")
	if not sucess and errormsg then
		cmd = cmd .. " : " .. errormsg
	end
	print(cmd);
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
