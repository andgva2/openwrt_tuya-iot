module("luci.controller.tuya_daemon_controller", package.seeall)

function index()
    entry({"admin", "services", "tuya_daemon"}, cbi("tuya_daemon_model"), "Tuya Cloud", 1)
end
