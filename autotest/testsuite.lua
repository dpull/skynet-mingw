local function add_testcase(self, testcase, is_passed, error_message)
	table.insert(self.testcase, {testcase = testcase, is_passed = is_passed, error_message = error_message})
end

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
local function save(self)
	pcall(function () 
		for k, v in ipairs(self.testcase) do
			local cmd = string.format("appveyor AddTest \"%s\" -Framework xUnit -FileName nil -Outcome %s", 
				v.testcase, v.is_passed and "Passed" or "Failed")
			if not v.is_passed then
				cmd = string.format("%s -ErrorMessage %s", cmd, v.error_message)
			end

			print(cmd)
			os.execute(cmd)
		end
	end)  
end

local function allpass(self)
	for k, v in ipairs(self.testcase) do
		if not v.is_passed then
			return false
		end
	end
	return true
end

local function create_testsuite(testsuite)
	return {testsuite = testsuite, testcase = {}, add = add_testcase, save = save, upload = upload, allpass = allpass,}
end

return create_testsuite;

