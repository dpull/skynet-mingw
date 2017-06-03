local skynet = require "skynet"
local test = require "xunit"
local snax = require "skynet.snax"
require "skynet.manager"

local max_client = 64

local function start_watchdog()
    local watchdog = skynet.newservice("watchdog")
    skynet.call(watchdog, "lua", "start", {
        port = 8888,
        maxclient = max_client,
        nodelay = true,
    })
    assert(watchdog) 
    skynet.kill(watchdog)
    return true
end

local function test_skynet_api()
    local service = skynet.uniqueservice("skynetservice")
    assert(service)
    local service1 = skynet.queryservice("skynetservice")
    assert(service1)
    skynet.send(service, "lua", "send_func", "MAIN_SEND")
    assert("MAIN_CALL" == skynet.call(service, "lua", "call_func", "MAIN_CALL"))

    skynet.kill(service)
    return true
end

local function test_snax_api()
    local service = snax.uniqueservice("snaxservice", "hello world")
    assert(service)
    local service1 = snax.queryservice("snaxservice")
    assert(service1 == service)
    service.post.hello("MAIN_POST")
    assert("MAIN_REQ" == service.req.hello("MAIN_REQ"))

    snax.kill(service)
    return true    
end

skynet.start(function()
    test("start watchdog", start_watchdog)
    test("test skynet api", test_skynet_api)
    test("test snax api", test_snax_api)
    print("Test finished...")
    skynet.abort()
    
    print("Shit, use os.exit exit!")
    os.exit(true)
end)
