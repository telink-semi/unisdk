#include "common/include/tlk_utils.h"
#include "core/include/tlk_spi.h"
#include "core/include/tlk_gpio.h"
#include "tlk_board_pinout.h"

#if (UNISDK_SPI_COUNT < 2)
#error "This sample requires at least 2 SPI modules on the chip"
#endif

void init_led(void) {
    tlk_gpio_configure(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
}

int main(void)
{
    init_led();
    
    struct tlk_spi_device *spi_slave = &tlk_spi0;
    struct tlk_spi_device *spi_master = &tlk_spi1;

    struct tlk_spi_config slave_config = {
        .pins = {
            .cs_port_pin = {
                .port = TLK_GPIO_PORT_E, 
                .pin = TLK_GPIO_PIN_0,
            },
            .sck_port_pin = {
                .port = TLK_GPIO_PORT_E, 
                .pin = TLK_GPIO_PIN_1,
            },
            .mosi_port_pin = {
                .port = TLK_GPIO_PORT_E, 
                .pin = TLK_GPIO_PIN_2,
            },
            .miso_port_pin = {
                .port = TLK_GPIO_PORT_E, 
                .pin = TLK_GPIO_PIN_3,
            },
        },
        .role = TLK_SPI_SLAVE,
        .mode = TLK_SPI_MODE0,
        .io_mode = TLK_SPI_SINGLE_MODE,
        .div_clock = 200,
    };

    struct tlk_spi_config master_config = {
        .pins = {
            .cs_port_pin = {
                .port = TLK_GPIO_PORT_A, 
                .pin = TLK_GPIO_PIN_0,
            },
            .sck_port_pin = {
                .port = TLK_GPIO_PORT_A, 
                .pin = TLK_GPIO_PIN_1,
            },
            .mosi_port_pin = {
                .port = TLK_GPIO_PORT_A,
                .pin = TLK_GPIO_PIN_2,
            },
            .miso_port_pin = {
                .port = TLK_GPIO_PORT_B, 
                .pin = TLK_GPIO_PIN_0,
            },
        },
        .role = TLK_SPI_MASTER,
        .mode = TLK_SPI_MODE0,
        .io_mode = TLK_SPI_SINGLE_MODE,
        .div_clock = 200,
    };

    unsigned char tx_data = 0xAB;
    unsigned char rx_data = 0;

    tlk_spi_configure(spi_slave, &slave_config);
    tlk_spi_configure(spi_master, &master_config);
  
    tlk_spi_receive(spi_slave, &rx_data, 1);
    
    while (1) {
        tlk_gpio_pin_write(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, 0);
        tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 0);
        tlk_sys_delay(TLK_USEC(1));

        tlk_spi_write(spi_master, &tx_data, 1);
        while (!spi_slave->rx_done) { }
        
        if (rx_data == tx_data) {
            tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 1);
            tlk_spi_receive(spi_slave, &rx_data, 1);
        }

        tx_data++;
        
        tlk_gpio_pin_write(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, 1);
        tlk_sys_delay(TLK_USEC(1));
    }

    return 0;
}
