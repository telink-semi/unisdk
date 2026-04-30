set(TINYUSB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/repo")
target_sources(${TARGET_NAME} PRIVATE
    ${TINYUSB_DIR}/src/tusb.c
    ${TINYUSB_DIR}/src/common/tusb_fifo.c
    ${TINYUSB_DIR}/src/device/usbd.c
    ${TINYUSB_DIR}/src/device/usbd_control.c
)

# Add device controller driver
target_sources(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/port/tlk_usb_dcd.c)

if (CONFIG_TLK_USB_CLASS_CDC)
target_sources(${TARGET_NAME} PRIVATE
    ${TINYUSB_DIR}/src/class/cdc/cdc_device.c
)
endif () # CONFIG_TLK_USB_CLASS_CDC

target_include_directories(${TARGET_NAME} PRIVATE 
    ${TINYUSB_DIR}/src
    ${TINYUSB_DIR}/../config
)