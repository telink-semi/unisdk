.extern tlk_reboot_reason

.global _WDT_ERRATA

_WDT_ERRATA:
    /* read and clear digital WDT status */
    li      t1, 0x80140140      /* digital WDT base address */

    lbu     t0, 0x20(t1)
    andi    t6, t0, 0x01        /* WDT status bit */

    beqz    t6, _WDT_STATUS_END

    sb      t6, 0x20(t1)        /* clear WDT status */

_WDT_STATUS_END:


#if TLK_IS_ENABLED(CONFIG_TLK_WDT_STARTUP_ENABLE)
    /* in order to cover all code, we need to enable digital WDT before disabling 32k WDT */
    li      t0, 24 * 5 * 1000000
    sw      t0, 0x0c(t1)        /* store 5 sec interval */

    lbu     t2, 0x22(t1)
    ori     t2, t2, 0x01
    sb      t2, 0x22(t1)        /* enable WDT */
#endif


    /* read the 32k WDT status before the stop, since it is cleared after that */
    li      t1, 0x80140180      /* base address */
    li      t0, 0x69            /* reg_ana_addr = 0x69 */
    sb      t0, 0x00(t1)

    li      t2, 1
    sb      t2, 0x03(t1)        /* reg_ana_len addr */

    li      t3, 0x40
    sb      t3, 0x02(t1)        /* reg_ana_ctrl addr - trigger read (FLD_ANA_CYC) */

_WDT_STATUS_ANALOG_WAIT1:
    lbu     t4, 0x02(t1)        /* poll reg_ana_ctrl & FLD_ANA_BUSY (0x80) */
    andi    t4, t4, 0x80
    bnez    t4, _WDT_STATUS_ANALOG_WAIT1

    lbu     t5, 4(t1)           /* read data byte at reg_ana_data0 (offset 4) */
    andi    t5, t5, 0x80        /* get the WDT status */

    or      t6, t6, t5
    /* we don't need to clear status, it is cleared automatically after disabling 32k WDT */


    /* store the TLK_SYS_REBOOT_REASON_WDT after both WDTs status checked if it is WDT reboot */
    la      t1, tlk_reboot_reason
    beqz    t6, _WDT_REBOOT_REASON_SET
    li      t6, 0x04            /* TLK_SYS_REBOOT_REASON_WDT */

_WDT_REBOOT_REASON_SET:
    /* store TLK_SYS_REBOOT_REASON_UNKNOWN (0x00) if it is not WDT reboot */
    sw      t6, 0(t1)


    /* stop 32k WDT, since it is enabled by default */
    /* it can't be reused, because the reference tick counter isn't reset on reboot */
    /* and updating the 32k WDT target tick would need to stop it anyway */
    li      t1, 0x80140180      /* base address */
    li      t0, 0x79            /* reg_ana_addr = 0x79 */
    sb      t0, 0(t1)

    addi    t2, t1, 3           /* reg_ana_len addr */
    li      t3, 1
    sb      t3, 0(t2)

    addi    t4, t1, 2           /* reg_ana_ctrl addr */
    li      t5, 0x40
    sb      t5, 0(t4)           /* trigger read (FLD_ANA_CYC) */

_WDT_STOP_ANALOG_WAIT1:
    lbu     t6, 0(t4)           /* poll reg_ana_ctrl & FLD_ANA_BUSY (0x80) */
    andi    t6, t6, 0x80
    bne     t6, x0, _WDT_STOP_ANALOG_WAIT1

    lbu     t6, 4(t1)           /* read data byte at reg_ana_data0 (offset 4) */
    andi    t6, t6, 0xFE        /* clear bit0 */
    sb      t6, 4(t1)           /* reg_ana_data0 = new value */

_WDT_STOP_ANALOG_WAIT2:
    lbu     t6, 8(t1)           /* poll reg_ana_buf_cnt */
    andi    t6, t6, 0xF0
    beq     t6, x0, _WDT_STOP_ANALOG_WAIT2

    li      t5, 0x60
    sb      t5, 0(t4)           /* reg_ana_ctrl = FLD_ANA_CYC|FLD_ANA_RW (0x60) */

_WDT_STOP_ANALOG_WAIT3:
    lbu     t6, 0(t4)
    andi    t6, t6, 0x80        /* wait busy clear */
    bne     t6, x0, _WDT_STOP_ANALOG_WAIT3

    sb      x0, 0(t4)           /* clear reg_ana_ctrl */

    ret
