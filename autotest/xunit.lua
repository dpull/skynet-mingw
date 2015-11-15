local template_head = [[
<assembly name="C:\Projects\GitHub\appvyr-xunit-tests\xUnit_x86_Clr4\bin\Debug\xUnit_x86_Clr4.dll" run-date="2014-11-28" run-time="14:11:45" configFile="C:\Tools\xUnit\xunit.console.clr4.x86.exe.Config" time="0.130" total="1" passed="0" failed="1" skipped="0" environment="32-bit .NET 4.0.30319.34014" test-framework="xUnit.net 1.9.2.1705">
	<class time="0.130" name="%s" total="1" passed="0" failed="1" skipped="0">
]]
local template_foot = [[
	</class>
</assembly>
]]

local template_failed_item = [=[
    	<test name="%s" type="%s" result="Fail">
      		<failure exception-type="Xunit.Sdk.TrueException">
	        	<message><![CDATA[Assert.True() Failure]]></message>
    	        <stack-trace>   
    	        	%s
    	        </stack-trace>
	      	</failure>
    	</test>
]=]

local template_passed_item = [[
    	<test name="%s" type="%s" result="Pass" />
]]

local function add_testcase(self, testcase, is_passed, stacktrace)
	table.insert(self.testcase, {testcase = testcase, is_passed = is_passed, stacktrace = stacktrace})
end

local function save(self)
	local file = io.open(self.filename,"w")
	file:write(string.format(template_head, self.testsuite))
	for k, v in ipairs(self.testcase) do
		if v.is_passed then
			file:write(string.format(template_passed_item, v.testcase, self.testsuite))
		else
			file:write(string.format(template_failed_item, v.testcase, self.testsuite, stacktrace))
		end
	end
	file:write(template_foot)
	file:close()
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
local function upload(self)
	pcall(function () 
		self:save()

		for k, v in ipairs(self.testcase) do
			local cmd = string.format("appveyor AddTest %s -Framework xUnit -FileName %s", v.testcase, self.filename)
			print(cmd)
			os.execute(cmd)
		end
	end)  
end

local function create_testsuite(testsuite, filename)
	filename = filename or "xunit_output.xml"
	return {testsuite = testsuite, filename = filename, testcase = {}, add = add_testcase, save = save, upload = upload, }
end

return create_testsuite;

