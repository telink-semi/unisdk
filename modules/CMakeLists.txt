if (CONFIG_TLK_USB)

add_subdirectory(tinyUSB)

endif() # CONFIG_TLK_USB

if (CONFIG_TLK_MBEDTLS)

add_subdirectory(mbedtls)

endif() # CONFIG_TLK_MBEDTLS
