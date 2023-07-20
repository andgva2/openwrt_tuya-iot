map = Map("tuya_daemon", "Tuya Cloud")

enableSection = map:section(NamedSection, "tuya_daemon_sct", "tuya_daemon", "Enable program")
flag = enableSection:option(Flag, "enable", "Enable", "Enable program")
dataSection = map:section(NamedSection, "tuya_daemon_sct", " tuya_daemon", "Device info")
product = dataSection:option(Value, "product_id", "Product ID")

deviceid = dataSection:option(Value, "device_id", "Device ID")
secret = dataSection:option(Value, "device_secret", "Device secret")
productid.size = 30
productid.maxlength = 16
deviceid.size = 30
deviceid.maxlength = 22
devicesecret.size = 30
devicesecret.maxlength = 16

return map
