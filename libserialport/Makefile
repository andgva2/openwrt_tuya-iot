#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libserialport
PKG_VERSION:=0.1.1
PKG_RELEASE:=7

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/sigrokproject/libserialport.git
PKG_SOURCE_DATE:=2022-01-28
PKG_SOURCE_VERSION:=master

PKG_MAINTAINER:=todd <todd@grizzlystar.com>
PKG_LICENSE:=LGPLv3+
PKG_LICENSE_FILES:=LGPLv3+

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(TARGET_CPPFLAGS)

define Package/libserialport
	CATEGORY:=Libraries
	TITLE:=Sigrok libserialport
	URL:=https://sigrok.org/wiki/Libserialport
endef

define Package/libserialport/description
	Library for accessing serial ports
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/{libserialport.so,libserialport.so.*} $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libserialport.h $(1)/usr/include/
endef

define Package/libserialport/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/{libserialport.so,libserialport.so.*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libserialport))
