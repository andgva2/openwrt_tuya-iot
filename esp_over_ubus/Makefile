include $(TOPDIR)/rules.mk

PKG_NAME:=esp_over_ubus
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/esp_over_ubus
	CATEGORY:=ESP_Controller
	TITLE:=esp_over_ubus
	DEPENDS:=+libserialport +libubus +libubox +libblobmsg-json +jansson
endef

define Package/esp_over_ubus/description
	Controll ESP over ubus
endef

define Package/esp_over_ubus/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/esp_over_ubus $(1)/usr/bin/esp_over_ubus
	$(INSTALL_BIN) ./files/esp_over_ubus.init $(1)/etc/init.d/esp_over_ubus
endef

$(eval $(call BuildPackage,esp_over_ubus))
