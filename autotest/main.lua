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

	local output = "output.xml"
	testsuite:save(output);
	pcall(function () 
		local cmd = string.format("appveyor AddTest %s -Framework xUnit -FileName %s", testsuite_name, output)
		print(cmd)
		os.execute(cmd)
	end)  
	os.exit()
end)

--[[
appveyor AddTest options:
 
  -Name             - Required. The name of test.
  -Framework        - Required. The name of testing framework, e.g. NUnit, xUnit, MSTest.
  -FileName         - Required. File name containg test.
  -Outcome          - Test outcome: None, Running, Passed, Failed, Ignored, Skipped, Inconclusive, NotFound, Cancelled, NotRunnable
  -Duration         - Test duration in milliseconds.
  -ErrorMessage     - Error message of failed test.
  -ErrorStackTrace  - Error stack trace of failed test.
  -StdOut           - Standard console output from the test.
  -StdErr           - Error output from the test.
]]
