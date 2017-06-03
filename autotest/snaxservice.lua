local skynet = require "skynet"
local snax = require "skynet.snax"

local msg = "I am a snax service"

function init(...)
    print ("snax service start: ", msg, ...)
end

function exit(...)
    print ("snax service exit: ", ...)
end

function response.hello(hello)
    return hello
end

function accept.hello(hello)
    assert("MAIN_POST" == hello)
end
