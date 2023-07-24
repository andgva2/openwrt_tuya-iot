module("luci.controller.tuya-iot_controller", package.seeall)

function index()
    entry({"admin", "services", "tuya_daemon"}, cbi("tuya-iot_model"), "Tuya Cloud", 1)
end
