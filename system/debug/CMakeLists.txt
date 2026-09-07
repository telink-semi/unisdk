add_sources_ifdef(CONFIG_TLK_DEBUG_LOG_CONSOLE "log/tlk_log.c")
add_sources_ifdef(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC "log/tlk_log_usb.c")
add_sources_ifdef(CONFIG_TLK_DEBUG_ASSERT "assert/tlk_assert.c")
