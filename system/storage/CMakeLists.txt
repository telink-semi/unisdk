target_include_directories(${TARGET_NAME} PRIVATE .)

target_sources(${TARGET_NAME} PRIVATE
	storage_device.c
	storage.c
)

add_sources_ifdef(CONFIG_TLK_SOC_FLASH devices/soc_flash.c)
