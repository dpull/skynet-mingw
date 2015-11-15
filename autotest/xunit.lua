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

local function save(self, filename)
	local file = io.open(filename,"w")
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

local function create_testsuite(testsuite)
	return {testsuite = testsuite, testcase = {}, add = add_testcase, save = save, upload = upload, }
end

return create_testsuite;

