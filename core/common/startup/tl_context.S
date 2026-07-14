/******************************************************************************
 * Copyright (c) 2023 Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************/

#define mmisc_ctl       0x7d0
#define mxstatus        0x7c4
#define mdcause         0x7c9

// Reserve backup buffer in .bss
    .section .bss, "aw", @nobits
    .align 4
    .global tlk_context_backup_registers
tlk_context_backup_registers:
    .skip 128 * 4

/******************************************
 *  Register layout comments unchanged...
 *****************************************/

    .text
    .align 2
    .global tlk_context_save
    .type tlk_context_save, @function
tlk_context_save:
    addi   sp, sp, (-2 * 4)
    sw     x30, (0 * 4)(sp)
    sw     x31, (1 * 4)(sp)

    lui    x31, %hi(tlk_context_backup_registers)
    addi   x31, x31, %lo(tlk_context_backup_registers)

    sw     x1, (0 * 4)(x31)
    sw     x3, (2 * 4)(x31)
    sw     x4, (3 * 4)(x31)
    sw     x5, (4 * 4)(x31)
    sw     x6, (5 * 4)(x31)
    sw     x7, (6 * 4)(x31)
    sw     x8, (7 * 4)(x31)
    sw     x9, (8 * 4)(x31)
    sw     x10, (9 * 4)(x31)
    sw     x11, (10 * 4)(x31)
    sw     x12, (11 * 4)(x31)
    sw     x13, (12 * 4)(x31)
    sw     x14, (13 * 4)(x31)
    sw     x15, (14 * 4)(x31)
    sw     x16, (15 * 4)(x31)
    sw     x17, (16 * 4)(x31)
    sw     x18, (17 * 4)(x31)
    sw     x19, (18 * 4)(x31)
    sw     x20, (19 * 4)(x31)
    sw     x21, (20 * 4)(x31)
    sw     x22, (21 * 4)(x31)
    sw     x23, (22 * 4)(x31)
    sw     x24, (23 * 4)(x31)
    sw     x25, (24 * 4)(x31)
    sw     x26, (25 * 4)(x31)
    sw     x27, (26 * 4)(x31)
    sw     x28, (27 * 4)(x31)
    sw     x29, (28 * 4)(x31)
    sw     x30, (29 * 4)(x31)

    mv     x30, x31

    /* save x2 - sp */
    mv     x31, x2
    addi   x31, x31, (2 * 4)
    sw     x31, (1 * 4)(x30)

    /* save csrs */
    csrr   x31, mepc
    sw     x31, (31 * 4)(x30)

    csrr   x31, mcause
    sw     x31, (32 * 4)(x30)

    csrr   x31, mtval
    sw     x31, (33 * 4)(x30)

    csrr   x31, mscratch
    sw     x31, (34 * 4)(x30)

    csrr   x31, mtvec
    sw     x31, (35 * 4)(x30)

    csrr   x31, mmisc_ctl
    sw     x31, (36 * 4)(x30)

    csrr   x31, mstatus
    sw     x31, (37 * 4)(x30)

    csrr   x31, mxstatus
    sw     x31, (38 * 4)(x30)

    csrr   x31, mdcause
    sw     x31, (39 * 4)(x30)

    csrr   x31, mip
    sw     x31, (40 * 4)(x30)

    csrr   x31, mie
    sw     x31, (41 * 4)(x30)

#ifdef __riscv_flen
    frcsr  x31
    fsw    f0, (42 * 4)(x30)
    fsw    f1, (43 * 4)(x30)
    fsw    f2, (44 * 4)(x30)
    fsw    f3, (45 * 4)(x30)
    fsw    f4, (46 * 4)(x30)
    fsw    f5, (47 * 4)(x30)
    fsw    f6, (48 * 4)(x30)
    fsw    f7, (49 * 4)(x30)
    fsw    f8, (50 * 4)(x30)
    fsw    f9, (51 * 4)(x30)
    fsw    f10, (52 * 4)(x30)
    fsw    f11, (53 * 4)(x30)
    fsw    f12, (54 * 4)(x30)
    fsw    f13, (55 * 4)(x30)
    fsw    f14, (56 * 4)(x30)
    fsw    f15, (57 * 4)(x30)
    fsw    f16, (58 * 4)(x30)
    fsw    f17, (59 * 4)(x30)
    fsw    f18, (60 * 4)(x30)
    fsw    f19, (61 * 4)(x30)
    fsw    f20, (62 * 4)(x30)
    fsw    f21, (63 * 4)(x30)
    fsw    f22, (64 * 4)(x30)
    fsw    f23, (65 * 4)(x30)
    fsw    f24, (66 * 4)(x30)
    fsw    f25, (67 * 4)(x30)
    fsw    f26, (68 * 4)(x30)
    fsw    f27, (69 * 4)(x30)
    fsw    f28, (70 * 4)(x30)
    fsw    f29, (71 * 4)(x30)
    fsw    f30, (72 * 4)(x30)
    fsw    f31, (73 * 4)(x30)
    sw     x31, (74 * 4)(x30)
#endif

    /* save x31 - t6 */
    lw     x31, (1 * 4)(sp)
    sw     x31, (30 * 4)(x30)

    lw     x30, (0 * 4)(sp)
    addi   sp, sp, (2 * 4)

    jr     ra

    .align 2
    .global tlk_context_restore
    .type tlk_context_restore, @function
tlk_context_restore:
    lui    x31, %hi(tlk_context_backup_registers)
    addi   x31, x31, %lo(tlk_context_backup_registers)

    /* restore x2 - sp */
    /* moved to cstartup_flash.S */

    /* restore csrs */
    lw     x30, (31 * 4)(x31)
    csrw   mepc, x30

    lw     x30, (32 * 4)(x31)
    csrw   mcause, x30

    lw     x30, (33 * 4)(x31)
    csrw   mtval, x30

    lw     x30, (34 * 4)(x31)
    csrw   mscratch, x30

    lw     x30, (35 * 4)(x31)
    csrw   mtvec, x30

    lw     x30, (36 * 4)(x31)
    csrw   mmisc_ctl, x30

    lw     x30, (37 * 4)(x31)
    csrw   mstatus, x30

    lw     x30, (38 * 4)(x31)
    csrw   mxstatus, x30

    lw     x30, (39 * 4)(x31)
    csrw   mdcause, x30

    lw     x30, (40 * 4)(x31)
    csrw   mip, x30

    lw     x30, (41 * 4)(x31)
    csrw   mie, x30

#ifdef __riscv_flen
    flw    f0, (42 * 4)(x31)
    flw    f1, (43 * 4)(x31)
    flw    f2, (44 * 4)(x31)
    flw    f3, (45 * 4)(x31)
    flw    f4, (46 * 4)(x31)
    flw    f5, (47 * 4)(x31)
    flw    f6, (48 * 4)(x31)
    flw    f7, (49 * 4)(x31)
    flw    f8, (50 * 4)(x31)
    flw    f9, (51 * 4)(x31)
    flw    f10, (52 * 4)(x31)
    flw    f11, (53 * 4)(x31)
    flw    f12, (54 * 4)(x31)
    flw    f13, (55 * 4)(x31)
    flw    f14, (56 * 4)(x31)
    flw    f15, (57 * 4)(x31)
    flw    f16, (58 * 4)(x31)
    flw    f17, (59 * 4)(x31)
    flw    f18, (60 * 4)(x31)
    flw    f19, (61 * 4)(x31)
    flw    f20, (62 * 4)(x31)
    flw    f21, (63 * 4)(x31)
    flw    f22, (64 * 4)(x31)
    flw    f23, (65 * 4)(x31)
    flw    f24, (66 * 4)(x31)
    flw    f25, (67 * 4)(x31)
    flw    f26, (68 * 4)(x31)
    flw    f27, (69 * 4)(x31)
    flw    f28, (70 * 4)(x31)
    flw    f29, (71 * 4)(x31)
    flw    f30, (72 * 4)(x31)
    flw    f31, (73 * 4)(x31)
    lw     x30, (74 * 4)(x31)
    fscsr  x30
#endif

    lw     x1, (0 * 4)(x31)
    lw     x3, (2 * 4)(x31)
    lw     x4, (3 * 4)(x31)
    lw     x5, (4 * 4)(x31)
    lw     x6, (5 * 4)(x31)
    lw     x7, (6 * 4)(x31)
    lw     x8, (7 * 4)(x31)
    lw     x9, (8 * 4)(x31)
    lw     x10, (9 * 4)(x31)
    lw     x11, (10 * 4)(x31)
    lw     x12, (11 * 4)(x31)
    lw     x13, (12 * 4)(x31)
    lw     x14, (13 * 4)(x31)
    lw     x15, (14 * 4)(x31)
    lw     x16, (15 * 4)(x31)
    lw     x17, (16 * 4)(x31)
    lw     x18, (17 * 4)(x31)
    lw     x19, (18 * 4)(x31)
    lw     x20, (19 * 4)(x31)
    lw     x21, (20 * 4)(x31)
    lw     x22, (21 * 4)(x31)
    lw     x23, (22 * 4)(x31)
    lw     x24, (23 * 4)(x31)
    lw     x25, (24 * 4)(x31)
    lw     x26, (25 * 4)(x31)
    lw     x27, (26 * 4)(x31)
    lw     x28, (27 * 4)(x31)
    lw     x29, (28 * 4)(x31)
    lw     x30, (29 * 4)(x31)
    lw     x31, (30 * 4)(x31)

    jr     ra
