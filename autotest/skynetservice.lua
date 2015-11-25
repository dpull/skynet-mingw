local skynet = require "skynet"

local CMD = {}

function CMD.send_func(msg)
    print ("This is a send from anthor service:", msg)
    assert("MAIN_SEND" == msg)
end
 
function CMD.call_func(msg)
    print ("This is a call from another service:", msg)
    skynet.response()(true, msg);
end

skynet.start(function ()
    skynet.dispatch("lua",function (session, source, cmd, msg)
        local func = assert(CMD[cmd])
        func(msg)
    end)
end)