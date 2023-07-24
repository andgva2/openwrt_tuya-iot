map = Map("tuya_daemon", "Tuya Cloud")

enableSct = map:section(NamedSection, "tuya_daemon_sct", "tuya_daemon", "Enable program")
flag = enableSct:option(Flag, "enable", "Enable", "Enable program")

connectionSct = map:section(NamedSection, "tuya_daemon_sct", "tuya_daemon", "Connection details")
productid = connectionSct:option(Value, "product_id", "Product ID")
productid.size = 30
productid.maxlength = 16

deviceid = connectionSct:option(Value, "device_id", "Device ID")
deviceid.size = 30
deviceid.maxlength = 22

devicesecret = connectionSct:option(Value, "device_secret", "Device secret")
devicesecret.size = 30
devicesecret.maxlength = 16

return map
