#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "core/include/tlk_spi.h"
#include "registers/tlk_spi_reg.h"

struct tlk_spi_device tlk_spi0 = {
    .module = TLK_SPI0
};

struct tlk_spi_device tlk_spi1 = {
    .module = TLK_SPI1
};

enum tlk_spi_gpio_function
{
    TLK_GSPI_CS   = 13,
    TLK_GSPI_SCK  = 14,
    TLK_GSPI_MISO = 17,
    TLK_GSPI_MOSI = 18,
};

enum tlk_spi_trans_mode
{
    TLK_SPI_MODE_WRITE_AND_READ, //write and read at the same.must enable CmdEn
    TLK_SPI_MODE_WRITE_ONLY,         //write
    TLK_SPI_MODE_READ_ONLY,          // read must enable CmdEn
    TLK_SPI_MODE_WRITE_READ,         //write_ read
    TLK_SPI_MODE_READ_WRITE,         //read_write
    TLK_SPI_MODE_WRITE_DUMMY_READ,   //write_dummy_read
    TLK_SPI_MODE_READ_DUMMY_WRITE,   //read_ dummy_write must enable CmdEn
    TLK_SPI_MODE_NONE_DATA,          //must enable CmdEn
    TLK_SPI_MODE_DUMMY_WRITE,        //dummy_write
    TLK_SPI_MODE_DUMMY_READ,         //dummy_read
    TLK_SPI_MODE_RESERVED,
};

enum tlk_spi_irq_status
{
    TLK_SPI_IRQ_RXFIFO_OR = TLK_BIT(0),
    TLK_SPI_IRQ_TXFIFO_UR = TLK_BIT(1),
    TLK_SPI_IRQ_RXFIFO    = TLK_BIT(2),
    TLK_SPI_IRQ_TXFIFO    = TLK_BIT(3),
    TLK_SPI_IRQ_END       = TLK_BIT(4),
    TLK_SPI_IRQ_CMD       = TLK_BIT(5),
};

#define TLK_SPI_TIMEOUT_US  0xffffffff

#define TLK_MIN(a, b) ((a) < (b) ? (a) : (b))

/* --- config dependencies --- */
void tlk_lspi_set_pin_mux(struct tlk_gpio_port_pin port_pin)
{
    const struct tlk_gpio_port_pin lspi_cs_def = {TLK_GPIO_PORT_E, TLK_GPIO_PIN_0};

    if (port_pin.port == lspi_cs_def.port && port_pin.pin == lspi_cs_def.pin) {
        tlk_gpio_configure(port_pin.port, port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
    } else {
        tlk_gpio_configure(port_pin.port, port_pin.pin, TLK_GPIO_INPUT_NO_PULL);
    }

    tlk_gpio_disable(port_pin.port, port_pin.pin);
}

void tlk_gspi_set_pin_mux(struct tlk_gpio_port_pin port_pin, enum tlk_spi_gpio_function function)
{
    if (function == TLK_GSPI_CS) {
        tlk_gpio_configure(port_pin.port, port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
    } else {
        tlk_gpio_configure(port_pin.port, port_pin.pin, TLK_GPIO_INPUT_NO_PULL);
    }

    tlk_gpio_disable(port_pin.port, port_pin.pin);
    tlk_gpio_set_mux(port_pin.port, port_pin.pin, function);
}

void tlk_spi_set_pins(enum tlk_spi_module module, struct tlk_spi_config* config)
{
    if (module == TLK_SPI0) {
        tlk_lspi_set_pin_mux(config->pins.cs_port_pin);
        tlk_lspi_set_pin_mux(config->pins.sck_port_pin);
        tlk_lspi_set_pin_mux(config->pins.mosi_port_pin);
        tlk_lspi_set_pin_mux(config->pins.miso_port_pin);
    }

    if (module == TLK_SPI1) {
        tlk_gspi_set_pin_mux(config->pins.cs_port_pin, TLK_GSPI_CS);
        tlk_gspi_set_pin_mux(config->pins.sck_port_pin, TLK_GSPI_SCK);
        tlk_gspi_set_pin_mux(config->pins.mosi_port_pin, TLK_GSPI_MOSI);
        tlk_gspi_set_pin_mux(config->pins.miso_port_pin, TLK_GSPI_MISO);
    }
}

static inline void tlk_spi_rx_irq_trig_cnt(enum tlk_spi_module module, unsigned char cnt)
{
    reg_spi_rxfifo_thres(module) = ((reg_spi_rxfifo_thres(module) & (~FLD_SPI_RXF_THRES)) | (cnt & FLD_SPI_RXF_THRES));
}

static inline void tlk_spi_tx_irq_trig_cnt(enum tlk_spi_module module, unsigned char cnt)
{
    reg_spi_txfifo_thres(module) = ((reg_spi_txfifo_thres(module) & (~FLD_SPI_TXF_THRES)) | (cnt & FLD_SPI_TXF_THRES));
}

static inline void tlk_spi_set_slave_irq_trigs(enum tlk_spi_module module)
{
    tlk_spi_rx_irq_trig_cnt(module, 4);
    tlk_spi_tx_irq_trig_cnt(module, 4);
}

static inline void tlk_spi_set_transmode(enum tlk_spi_module module, enum tlk_spi_trans_mode mode)
{
    reg_spi_ctrl2(module) = ((reg_spi_ctrl2(module) & (~FLD_SPI_TRANSMODE)) | ((mode & 0xf) << 4));
}

void tlk_spi_set_io_mode(enum tlk_spi_module module, enum tlk_spi_io_mode mode)
{
    TLK_BM_CLR(reg_spi_ctrl1(module), FLD_SPI_DUAL); // spi_dual_mode_dis
    TLK_BM_CLR(reg_spi_ctrl1(module), FLD_SPI_QUAD); // spi_quad_mode_dis
    TLK_BM_CLR(reg_spi_ctrl3(module), FLD_SPI_3LINE); // spi_3line_mode_dis

    if (mode == TLK_SPI_DUAL_MODE) {
        TLK_BM_SET(reg_spi_ctrl1(module), FLD_SPI_DUAL); // spi_dual_mode_en
    } else if (mode == TLK_SPI_QUAD_MODE) {
        TLK_BM_SET(reg_spi_ctrl1(module), FLD_SPI_QUAD); // spi_quad_mode_en
    } else if (mode == TLK_SPI_3_LINE_MODE) {
        TLK_BM_SET(reg_spi_ctrl3(module), FLD_SPI_3LINE); // spi_3line_mode_en
    }
}


static inline void tlk_spi_set_irq_mask(enum tlk_spi_module module, enum tlk_spi_irq_status mask)
{
    TLK_BM_SET(reg_spi_ctrl0(module), mask);
}

static inline unsigned char tlk_spi_get_irq_status(enum tlk_spi_module module, enum tlk_spi_irq_status status)
{
    return reg_spi_int_status0(module) & status;
}

static inline void tlk_spi_clr_irq_status(enum tlk_spi_module module, enum tlk_spi_irq_status status)
{
    reg_spi_int_status0(module) = status;
}
/* --- config dependencies --- */

/* --- other dependencies --- */
static inline bool tlk_spi_is_busy(enum tlk_spi_module module)
{
    return reg_spi_status(module) & FLD_SPI_BUSY;
}

void tlk_timeout_handler(unsigned int err_code)
{
    (void)err_code; // empty handler
}
/* --- other dependencies --- */

/* --- write dependencies --- */
static inline void tlk_spi_tx_fifo_clr(enum tlk_spi_module module)
{
    TLK_BM_SET(reg_spi_status(module), FLD_SPI_TXF_CLR_LEVEL);
    while (FLD_SPI_TXF_CLR_LEVEL == (reg_spi_status(module) & FLD_SPI_TXF_CLR_LEVEL)) ;
}

static inline bool tlk_spi_txfifo_num_is_word(enum tlk_spi_module module)
{
    unsigned char tx_fifo_depth = (module == TLK_SPI0) ? 20 : 8;
    return (tx_fifo_depth - (reg_spi_txfifo_status(module) & FLD_SPI_TXF_ENTRIES) < 4);
}

static inline bool tlk_spi_txfifo_is_full(enum tlk_spi_module module)
{
    return reg_spi_txfifo_status(module) & FLD_SPI_TXF_FULL;
}
/* --- write dependencies --- */

/* --- read dependencies --- */
static inline void tlk_spi_rx_fifo_clr(enum tlk_spi_module module)
{
    TLK_BM_SET(reg_spi_status(module), FLD_SPI_RXF_CLR_LEVEL);
    while (FLD_SPI_RXF_CLR_LEVEL == (reg_spi_status(module) & FLD_SPI_RXF_CLR_LEVEL)) ;
}

static inline unsigned char tlk_spi_get_rxfifo_num(enum tlk_spi_module module)
{
    return reg_spi_rxfifo_status(module) & FLD_SPI_RXF_ENTRIES;
}

static inline bool tlk_spi_rxfifo_num_is_word(enum tlk_spi_module module)
{
    return ((reg_spi_rxfifo_status(module) & FLD_SPI_RXF_ENTRIES) < 4);
}

static inline bool tlk_spi_rxfifo_is_empty(enum tlk_spi_module module)
{
    return reg_spi_rxfifo_status(module) & FLD_SPI_RXF_EMPTY;
}

void tlk_spi_read(enum tlk_spi_module module, unsigned char *data, unsigned int len)
{
    unsigned int  word_len   = len >> 2;
    unsigned char single_len = len & 3;

    //When the data size in rx_fifo is not less than 4 bytes, the MCU moves the data according to the word length
    for (unsigned int i = 0; i < word_len; i++) {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (!tlk_spi_rxfifo_num_is_word(module)), 
            (TLK_SPI_TIMEOUT_US), 
            (
                tlk_timeout_handler(0);
                return;
            )
        )

        ((unsigned int *)data)[i] = reg_spi_wr_rd_data_word(module);
    }

    //When the data size in rx_fifo is less than 4 bytes, the MCU moves the data according to the word length
    for (unsigned char i = 0; i < single_len; i++) {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (!tlk_spi_rxfifo_is_empty(module)), 
            (TLK_SPI_TIMEOUT_US), 
            (
                tlk_timeout_handler(0);
                return;
            )
        )

        data[(word_len * 4) + i] = reg_spi_wr_rd_data((module), i % 4);
    }
}
/* --- read dependencies --- */

/* --- IRQ --- */
_tlk_attribute_ram_code_sec_ void tlk_spi_irq_handler(struct tlk_spi_device *device)
{
    if (tlk_spi_get_irq_status(device->module, TLK_SPI_IRQ_RXFIFO)) {
        unsigned int read_size = (device->rx_cnt + 4 <= device->rx_buf_size) ? 4 : device->rx_buf_size % 4;
        
        tlk_spi_read(device->module, device->rx_buf + device->rx_cnt, read_size);
        device->rx_cnt += read_size;
        device->rx_done = (device->rx_cnt == device->rx_buf_size);

        tlk_spi_clr_irq_status(device->module, TLK_SPI_IRQ_RXFIFO);
    }

    if (tlk_spi_get_irq_status(device->module, TLK_SPI_IRQ_END)) {
        unsigned char read_size = TLK_MIN(tlk_spi_get_rxfifo_num(device->module), device->rx_buf_size - device->rx_cnt);
        
        tlk_spi_read(device->module, device->rx_buf + device->rx_cnt, read_size);
        device->rx_cnt += read_size;
        device->rx_done = true;

        tlk_spi_rx_fifo_clr(device->module);
        tlk_spi_clr_irq_status(device->module, TLK_SPI_IRQ_END | TLK_SPI_IRQ_RXFIFO);
    }
}

_tlk_attribute_ram_code_sec_ void tlk_lspi_irq_handler(void) {
    tlk_spi_irq_handler(&tlk_spi0);
}

_tlk_attribute_ram_code_sec_ void tlk_gspi_irq_handler(void) {
    tlk_spi_irq_handler(&tlk_spi1);
}

TLK_PLIC_ISR_REGISTER(tlk_lspi_irq_handler, CONFIG_SPI0_IRQ_NUM);
TLK_PLIC_ISR_REGISTER(tlk_gspi_irq_handler, CONFIG_SPI1_IRQ_NUM);
/* --- IRQ --- */

void tlk_spi_configure(struct tlk_spi_device* device, struct tlk_spi_config* config)
{
    tlk_spi_set_pins(device->module, config);

    if (config->role == TLK_SPI_MASTER) {
        if (device->module == TLK_SPI0) {
            if (config->div_clock > 63) { //LSPI clock source select pll_clk when div_clock <= 63,select RC 24M when div_clock > 63.
                reg_lspi_clk_set = ((FLD_LSPI_CLK_MOD & (unsigned char)(config->div_clock * 24 / (tlk_sys_clk.pll_clk))) | FLD_LSPI_DIV_RSTN);
            } else {
                reg_lspi_clk_set = ((FLD_LSPI_CLK_MOD & (unsigned char)config->div_clock) | FLD_LSPI_DIV_RSTN | FLD_LSPI_DIV_IN_SEL);
            }
        }

        if (device->module == TLK_SPI1) {
            if (config->div_clock > 255) { //GSPI clock source select pll_clk when div_clock <= 255,select RC 24M when div_clock > 255.
                reg_gspi_clk_set = ((FLD_GSPI_CLK_MOD & (unsigned char)(config->div_clock * 24 / (tlk_sys_clk.pll_clk))) | FLD_GSPI_DIV_RSTN);
            } else {
                reg_gspi_clk_set = ((FLD_GSPI_CLK_MOD & (unsigned char)config->div_clock) | FLD_GSPI_DIV_RSTN | FLD_GSPI_DIV_IN_SEL);
            }
        }

        reg_spi_ctrl3(device->module) |= (FLD_SPI_MASTER_MODE | FLD_SPI_DMATX_SOF_CLRTXF_EN | FLD_SPI_DMARX_EOF_CLRRXF_EN | FLD_SPI_AUTO_HREADY_EN); //master
        reg_spi_ctrl3(device->module) = ((reg_spi_ctrl3(device->module) & (~FLD_SPI_WORK_MODE)) | (config->mode << 2));
        
        tlk_spi_rx_irq_trig_cnt(device->module, 4);
        tlk_spi_tx_irq_trig_cnt(device->module, 4);
        
        tlk_spi_set_transmode(device->module, TLK_SPI_MODE_WRITE_ONLY);
    }

    if (config->role == TLK_SPI_SLAVE) {
        if (device->module == TLK_SPI0) {
            reg_lspi_clk_set = ((FLD_LSPI_CLK_MOD & (unsigned char)(tlk_sys_clk.pll_clk / tlk_sys_clk.hclk)) | FLD_LSPI_DIV_RSTN | FLD_LSPI_DIV_IN_SEL);
        } else {
            reg_gspi_clk_set = ((FLD_GSPI_CLK_MOD & (unsigned char)(tlk_sys_clk.pll_clk / tlk_sys_clk.hclk)) | FLD_GSPI_DIV_RSTN | FLD_GSPI_DIV_IN_SEL);
        }
        
        reg_spi_ctrl3(device->module) &= (~FLD_SPI_MASTER_MODE);                                                               //slave
        reg_spi_ctrl3(device->module) |= (FLD_SPI_DMATX_SOF_CLRTXF_EN | FLD_SPI_DMARX_EOF_CLRRXF_EN | FLD_SPI_AUTO_HREADY_EN); //slave
        reg_spi_ctrl3(device->module) = ((reg_spi_ctrl3(device->module) & (~FLD_SPI_WORK_MODE)) | (config->mode << 2));                       // select SPI mode, support four modes
        
        tlk_spi_set_slave_irq_trigs(device->module);
        tlk_spi_set_transmode(device->module, TLK_SPI_MODE_READ_ONLY);

        tlk_spi_clr_irq_status(device->module, TLK_SPI_IRQ_RXFIFO | TLK_SPI_IRQ_END);
        tlk_spi_set_irq_mask(device->module, TLK_SPI_IRQ_RXFIFO | TLK_SPI_IRQ_END);

        tlk_plic_interrupt_enable((device->module == TLK_SPI0) ? CONFIG_SPI0_IRQ_NUM : CONFIG_SPI1_IRQ_NUM);
        tlk_core_interrupt_enable(); //!!
    }

    // config
    tlk_spi_set_io_mode(device->module, config->io_mode);

    TLK_BM_CLR(reg_spi_ctrl4(device->module), FLD_SPI_XIP_ENABLE); // spi_xip_dis
    TLK_BM_CLR(reg_spi_ctrl1(device->module), FLD_SPI_CMD_EN); // spi_cmd_dis
    TLK_BM_CLR(reg_spi_ctrl1(device->module), FLD_SPI_ADDR_EN); // spi_addr_dis
}

bool tlk_spi_write(struct tlk_spi_device* device, const unsigned char *data, unsigned int len)
{
    tlk_spi_tx_fifo_clr(device->module);
    reg_spi_tx_cnt(device->module) = ((len - 1) & 0xffffff); // spi_tx_cnt
    reg_spi_cmd(device->module) = 0x00; // spi_set_cmd //!!
    
    unsigned int  word_len   = len >> 2;
    unsigned char single_len = len & 3;
    
    //When the remaining size in tx_fifo is not less than 4 bytes, the MCU moves the data according to the word length.
    for (unsigned int i = 0; i < word_len; i++) {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (!tlk_spi_txfifo_num_is_word(device->module)), 
            (TLK_SPI_TIMEOUT_US), 
            (
                tlk_timeout_handler(0);
                return false;
            )
        )

        reg_spi_wr_rd_data_word(device->module) = ((const unsigned int *)data)[i];
    }
    
    //When the remaining size in tx_fifo is less than 4 bytes, the MCU moves the data according to the byte length.
    for (unsigned int i = 0; i < single_len; i++) {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (!tlk_spi_txfifo_is_full(device->module)), 
            (TLK_SPI_TIMEOUT_US), 
            (
                tlk_timeout_handler(0);
                return false;
            )
        )

        reg_spi_wr_rd_data(device->module, i % 4) = data[(word_len * 4) + i];
    }

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
        (!tlk_spi_is_busy(device->module)), 
        (TLK_SPI_TIMEOUT_US), 
        (
            tlk_timeout_handler(0);
            return false;
        )
    )

    return true;
}

void tlk_spi_receive(struct tlk_spi_device* device, unsigned char *buf, unsigned int size)
{
    device->rx_done = 0;
    device->rx_buf = buf;
    device->rx_buf_size = size;
    device->rx_cnt = 0;
}

#if IS_ENABLED(CONFIG_SPI_AUTO_CONFIG)
#include "common/include/tlk_init.h"

#define TLK_SPI_AUTO_CONFIG(num) \
    struct tlk_spi_config tlk_spi_config##num = {\
        .pins = {\
            {CONFIG_SPI##num##_CS_PORT,   CONFIG_SPI##num##_CS_PIN},\
            {CONFIG_SPI##num##_SCK_PORT,  CONFIG_SPI##num##_SCK_PIN},\
            {CONFIG_SPI##num##_MOSI_PORT, CONFIG_SPI##num##_MOSI_PIN},\
            {CONFIG_SPI##num##_MISO_PORT, CONFIG_SPI##num##_MISO_PIN},\
        },\
        .role = CONFIG_SPI##num##_ROLE,\
        .mode = CONFIG_SPI##num##_MODE,\
        .io_mode = CONFIG_SPI##num##_IO_MODE,\
        .div_clock = CONFIG_SPI##num##_DIV_CLOCK\
    };\
    tlk_spi_configure(&tlk_spi##num, &tlk_spi_config##num);

void tlk_spi_auto_init(void) {
    IF_ENABLED(CONFIG_SPI0_AUTO_CONFIG, (TLK_SPI_AUTO_CONFIG(0)) )

    IF_ENABLED(CONFIG_SPI1_AUTO_CONFIG, (TLK_SPI_AUTO_CONFIG(1)) )
}

TLK_REGISTER_PRE_INIT(tlk_spi_auto_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
#endif

//spi_rx_tx_irq_trig_cnt in spi_master_init