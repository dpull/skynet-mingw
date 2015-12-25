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
local function report(testcase, success, traceback, result, errormessage)
	pcall(function () 
		local cmd = string.format("appveyor AddTest \"%s\" -Framework xUnit -FileName none -Outcome %s", 
			testcase, (success and result) and "Passed" or "Failed")
		if errormessage then
			cmd = string.format("%s -ErrorMessage \"%s\"", cmd, errormessage)
		end

		if traceback then
			cmd = string.format("%s -ErrorStackTrace \"%s\"", cmd, traceback)
		end

		print(cmd)
		os.execute(cmd)
	end)  
end

--[[
	testcase 测试用例
	fn 测试函数  返回值（测试结果，测试失败原因）
]]
local function test(testcase, fn, ...)
	local traceback;
	local success, result, errormessage = xpcall(fn, function() traceback = debug.traceback() end, ...)
	report(testcase, success, traceback, result, errormessage)
end

return test;