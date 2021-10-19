/*
 * Copyright (C) 2010-2012 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm-offsets.h>
#include <linux/linkage.h>

// suspend and resume for mx6
.section ".low_power_code_suspend_resume", "x"
//.section ".low_power_code", "x"

#define ARM_CTRL_DCACHE               0x4
#define ARM_CTRL_ICACHE               0x1000
#define ARM_AUXCR_L2EN                0x2
#define TTRBIT_MASK                   0xffffc000
#define TABLE_INDEX_MASK              0xfff00000
#define TABLE_ENTRY                   0x00000c02
#define CACHE_DISABLE_MASK            0xfffffffb
#define MMDC_MAPSR_OFFSET             0x404
#define MMDC_MAPSR_PSS                0x10
#define MMDC_MAPSR_PSD                0x1
#define IRAM_SUSPEND_SIZE             0x1000

#define L2_BASE_ADDR                  0x00a02000

#define L2X0_CACHE_ID                 0x000
#define L2X0_CACHE_TYPE               0x004
#define L2X0_CTRL                     0x100
#define L2X0_AUX_CTRL                 0x104
#define L2X0_TAG_LATENCY_CTRL         0x108
#define L2X0_DATA_LATENCY_CTRL        0x10C
#define L2X0_EVENT_CNT_CTRL           0x200
#define L2X0_EVENT_CNT1_CFG           0x204
#define L2X0_EVENT_CNT0_CFG           0x208
#define L2X0_EVENT_CNT1_VAL           0x20C
#define L2X0_EVENT_CNT0_VAL           0x210
#define L2X0_INTR_MASK                0x214
#define L2X0_MASKED_INTR_STAT         0x218
#define L2X0_RAW_INTR_STAT            0x21C
#define L2X0_INTR_CLEAR               0x220
#define L2X0_CACHE_SYNC               0x730
#define L2X0_DUMMY_REG                0x740
#define L2X0_INV_LINE_PA              0x770
#define L2X0_INV_WAY                  0x77C
#define L2X0_CLEAN_LINE_PA            0x7B0
#define L2X0_CLEAN_LINE_IDX           0x7B8
#define L2X0_CLEAN_WAY                0x7BC
#define L2X0_CLEAN_INV_LINE_PA        0x7F0
#define L2X0_CLEAN_INV_LINE_IDX       0x7F8
#define L2X0_CLEAN_INV_WAY            0x7FC
#define L2X0_LOCKDOWN_WAY_D           0x900
#define L2X0_LOCKDOWN_WAY_I           0x904
#define L2X0_TEST_OPERATION           0xF00
#define L2X0_LINE_DATA                0xF10
#define L2X0_LINE_TAG                 0xF30
#define L2X0_DEBUG_CTRL               0xF40
#define L2X0_PREFETCH_CTRL            0xF60
#define L2X0_POWER_CTRL               0xF80

#define L2X0_DYNAMIC_CLK_GATING_EN               (1 << 1)
#define L2X0_STNDBY_MODE_EN                      (1 << 0)
#define L2X0_CACHE_ID_PART_MASK                  (0xf << 6)
#define L2X0_CACHE_ID_PART_L210                  (1 << 6)
#define L2X0_CACHE_ID_PART_L310                  (3 << 6)
#define L2X0_AUX_CTRL_MASK                       0xc0000fff
#define L2X0_AUX_CTRL_ASSOCIATIVITY_SHIFT        16
#define L2X0_AUX_CTRL_WAY_SIZE_SHIFT             17
#define L2X0_AUX_CTRL_WAY_SIZE_MASK              (0x7 << 17)
#define L2X0_AUX_CTRL_SHARE_OVERRIDE_SHIFT       22
#define L2X0_AUX_CTRL_NS_LOCKDOWN_SHIFT          26
#define L2X0_AUX_CTRL_NS_INT_CTRL_SHIFT          27
#define L2X0_AUX_CTRL_DATA_PREFETCH_SHIFT        28
#define L2X0_AUX_CTRL_INSTR_PREFETCH_SHIFT       29
#define L2X0_AUX_CTRL_EARLY_BRESP_SHIFT          30

//Additional define added for validation code environment
#define GPIO1_BASE_ADDR				 0x0209C000 
#define PERIPBASE_VIRT                           0x0
#define SRC_BASE_ADDR                            0x020d8000
#define SRC_GPR1_OFFSET                          0x020
#define SRC_GPR2_OFFSET                          0x024
#define MX6Q_IOMUXC_BASE_ADDR                    0x020e0000
#define MMDC_P0_BASE_ADDR                        0x021b0000
#define PAGE_OFFSET                              0x0
#define MX6_PHYS_OFFSET                          0x0

// define for bus clock divider in wait mode
#define CCM_BASE_ADDR                            0x020C4000
#define CCM_CBCDR_OFFSET                         0x14
#define CCM_CACCR_OFFSET                         0x10
#define CCM_CCGR7_OFFSET                         0x84

//PREPH_DIV                                ((1-1) << 27)
//PREPH2_DIV                               ((1-1) << 0 )
//FABRIC_DIV                               ((1-1) << 3 )
//OCRAM_DIV                                ((1-1) << 16)
//AHB_DIV                                  ((1-1) << 10)
//IPG_DIV                                  ((2-1) << 8 )

#define PREPH_DIV                                ((1-1) << 27)
#define PREPH2_DIV                               ((8-1) << 0 )
#define FABRIC_DIV                               ((3-1) << 3 )
#define OCRAM_DIV                                ((8-1) << 16)
#define AHB_DIV                                  ((8-1) << 10)
#define IPG_DIV                                  ((2-1) << 8 )

#define CCM_CBCDR_WAIT_MODE                      (0x06008000 | PREPH_DIV | PREPH2_DIV | OCRAM_DIV | AHB_DIV | IPG_DIV | FABRIC_DIV)
#define CCM_CBCDR_ON_WAKEUP                      0x06008100

#define CCM_CACCR_WAIT_MODE                      (0x7)

   //GBLL CONFIG_DRAM_IO
   //GBLL CONFIG_CACHE_L2X0
   //GBLL CONFIG_CACHE_L1
//CONFIG_DRAM_IO         SETL   {FALSE}
//CONFIG_CACHE_L1        SETL   {TRUE}
//CONFIG_CACHE_L2X0      SETL   {TRUE}

#define CONFIG_CACHE_L1     1

#define CONFIG_DRAM_IO

//#define CONFIG_CACHE_L2X0   1

/*************************************************************
; mx6q_suspend:
;
; Suspend the processor (eg, wait for interrupt).
; Set the DDR into Self Refresh
; IRQs are already disabled.
;
; The following code contain both standby and
; dormant mode for MX6, decided by the parameter
; passed in r0:
; see define in include/linux/suspend.h
; 1 -> cpu enter stop mode;
; 3 -> cpu enter dormant mode.
; r1: iram_paddr
; r2: suspend_iram_base
*************************************************************/

.global mx6q_suspend
mx6q_suspend:

     stmfd    sp!, {r0-r12}     @ Save registers

/*************************************************************
; suspend mode entry
*************************************************************/

     cmp    r0, #0x3
     beq    dormant        /* dormant mode */
     cmp    r0, #0x4
     beq    dormant        /* dormant mode */






enter_wait_mode_no_srpg:
     ldr    r1, =MMDC_P0_BASE_ADDR

     /* Disable Automatic power savings. */
     ldr    r3, [r1, #0x404]
     orr    r3, r3, #0x01
     str    r3, [r1, #0x404]

     /* Make the DDR explicitly enter self-refresh. */
     ldr     r3, [r1, #0x404]
     orr     r3, r3, #0x200000
     str     r3, [r1, #0x404]

poll_dvfs_set_2:
     ldr     r3, [r1, #0x404]
     and     r3, r3, #0x2000000
     cmp     r3, #0x2000000
     bne     poll_dvfs_set_2

     /* set SBS step-by-step mode */
     ldr     r3, [r1, #0x410]
     orr     r3, r3, #0x100
     str     r3, [r1, #0x410]

     /* set ddr io */
     ldr	r6, =0x20e0000
     ldr	r7, =0x0
     str	r7, [r6, #0x5b4]

     str	r7, [r6, #0x30c]
     str	r7, [r6, #0x310]
     str	r7, [r6, #0x314]
     str	r7, [r6, #0x318]

     str	r7, [r6, #0x5c4]
     str	r7, [r6, #0x5cc]
     str	r7, [r6, #0x5d4]
     str	r7, [r6, #0x5d8]

     str	r7, [r6, #0x300]
     str	r7, [r6, #0x31c]
     str	r7, [r6, #0x338]

     str	r7, [r6, #0x5ac]

     str	r7, [r6, #0x5b0]
     str	r7, [r6, #0x5c0]
     str	r7, [r6, #0x33c]
     str	r7, [r6, #0x340]
	
     ldr	r7, =0x1000
     str	r7, [r6, #0x5b4]
     str	r7, [r6, #0x330]
     str	r7, [r6, #0x334]
	
     ldr	r7, =0x80000
     str	r7, [r6, #0x320]
	

//  ; save into r2
     ldr    r3,=CCM_BASE_ADDR
     ldr    r2,[r3,#CCM_CBCDR_OFFSET]

//    ; use optimized bus clock freq for wait mode
     cmp    r0, #0x2
     bne    enter_wait_wfi

     // change podf for MMDC
     ldr     r1, [r3, #CCM_CBCDR_OFFSET]
     orr     r1, r1, #0x7
     str     r1, [r3, #CCM_CBCDR_OFFSET]

podf_loop5:
     ldr     r1, [r3, #0x48]
     cmp     r1, #0x0
     bne     podf_loop5

     // change podf for MMDC
     ldr     r1, [r3, #CCM_CBCDR_OFFSET]
     orr     r1, r1, #0x10
     str     r1, [r3, #CCM_CBCDR_OFFSET]

podf_loop6:
     ldr     r1, [r3, #0x48]
     cmp     r1, #0x0
     bne     podf_loop6

     // change podf for AHB, IPG and OCRAM
     ldr     r1, [r3, #CCM_CBCDR_OFFSET]
     orr     r1, r1, #0x1C00
     orr     r1, r1, #0x100
     orr     r1, r1, #0x70000
     str     r1, [r3, #CCM_CBCDR_OFFSET]

podf_loop4:
     ldr     r1, [r3, #0x48]
     cmp     r1, #0x0
     bne     podf_loop4

    ldr    r1,=CCM_CACCR_WAIT_MODE
    str    r1,[r3,#CCM_CACCR_OFFSET]
podf_loop0:
    ldr    r1, [r3, #0x48]
    cmp    r1, #0x0
    bne    podf_loop0

enter_wait_wfi:
    dsb
    wfi

1:  b 1b

exit_wait_mode_no_srpg:
//  ; restore original value
    ldr    r0,=CCM_BASE_ADDR
    str    r2,[r0,#CCM_CBCDR_OFFSET]

    nop
    nop
    nop
    nop
/* Due to the L2 cache errata(TKT065875)
; , need to wait at least 170ns, each IO read
; takes about 76ns, but the actual wait time
to make system more stable is about 380ns */
    ldr    r0, =SRC_BASE_ADDR
    add r0, r0, #PERIPBASE_VIRT
    ldr    r1, [r0]
    ldr    r1, [r0, #0x4]
    ldr    r1, [r0, #0x8]
    ldr    r1, [r0, #0xc]
    ldr    r1, [r0, #0x10]
/***********************************************************
; never run to here
************************************************************/
    b     out    /* exit standby */

/************************************************************
; dormant entry, data save in stack, save sp in the src_gpr2
************************************************************/
dormant:
//dormant
//  ; save r0 into an unused register
    ldr     r4, =CCM_BASE_ADDR
    add     r4, r4, #PERIPBASE_VIRT
    str     r0, [r4, #CCM_CCGR7_OFFSET]

    mov     r3, r1
    mov     r0, r1
    add     r0, r0, #IRAM_SUSPEND_SIZE /* 4K */
    ldr     r4, =SRC_BASE_ADDR
    add     r4, r4, #PERIPBASE_VIRT
    str     r0, [r4, #SRC_GPR2_OFFSET] /* set src_gpr2 */

/************************************************************
; saved register and context as below:
;     ddr_iomux set
;     sp
;     spsr
;     lr
;     CPACR
;     TTBR0
;     TTBR1
;     TTBCR
;     DACR
;     PRRR
;     NMRR
;     ACTLR
;     Context ID
;     User r/w thread ID
;     Secure or NS VBAR
;     CPSR
;     SCTLR
************************************************************/

//; ddr_iomux_save:
/* save mmdc iomux setting, stack is from the tail of
   iram_suspend base */

    mov    r0, r2                        /* get suspend_iram_base */
    add    r0, r0, #IRAM_SUSPEND_SIZE    /* 4K */
    ldr    r1, =MX6Q_IOMUXC_BASE_ADDR
    add    r1, r1, #PERIPBASE_VIRT

#ifdef CONFIG_DRAM_IO
    mov r8, lr
    bl ddr_io_save
    mov lr, r8
#endif

    mov    r4, sp                        @ Store sp
    mrs    r5, spsr                      @ Store spsr
    mov    r6, lr                        @ Store lr
    stmfd    r0!, {r4-r6}

//; save cpu registers

    /* c1 and c2 registers */
    mrc    p15, 0, r4, c1, c0, 2    @ CPACR
    mrc    p15, 0, r5, c2, c0, 0    @ TTBR0
    mrc    p15, 0, r6, c2, c0, 1    @ TTBR1
    mrc    p15, 0, r7, c2, c0, 2    @ TTBCR
    stmfd    r0!, {r4-r7}

    /* c3 and c10 registers */
    mrc    p15, 0, r4, c3, c0, 0    @ DACR
    mrc    p15, 0, r5, c10, c2, 0   @ PRRR
    mrc    p15, 0, r6, c10, c2, 1   @ NMRR
    mrc    p15, 0, r7, c1, c0, 1    @ ACTLR
    stmfd    r0!,{r4-r7}

    /* c12, c13 and CPSR registers */
    mrc    p15, 0, r4, c13, c0, 1   @ Context ID
    mrc    p15, 0, r5, c13, c0, 2   @ User r/w thread ID
    mrc    p15, 0, r6, c12, c0, 0   @ Secure or NS VBAR
    mrs    r7, cpsr                 @ Store CPSR
    stmfd    r0!, {r4-r7}

    /* c1 control register */
    mrc    p15, 0, r4, c1, c0, 0    @ SCTLR
    stmfd    r0!, {r4}

#ifdef CONFIG_CACHE_L2X0
     ldr r2, =L2_BASE_ADDR
     add r2, r2, #PERIPBASE_VIRT

     ldr    r4, [r2, #L2X0_CTRL]
     ldr    r5, [r2, #L2X0_AUX_CTRL]
     ldr    r6, [r2, #L2X0_TAG_LATENCY_CTRL]
     ldr    r7, [r2, #L2X0_DATA_LATENCY_CTRL]
     stmfd    r0!, {r4-r7}

     ldr    r4, [r2, #L2X0_PREFETCH_CTRL]
     ldr    r5, [r2, #L2X0_POWER_CTRL]
     stmfd    r0!, {r4-r5}
#endif

#ifdef CONFIG_CACHE_L1
    /*
     * Flush all data from the L1 data cache before disabling
     * SCTLR.C bit.
     */
    push   {r0-r12, lr}
    ldr    r0, =v7_flush_dcache_all
    mov    lr, pc
    mov    pc, r0
    pop    {r0-r12, lr}

   /*
    * Clear the SCTLR.C bit to prevent further data cache
    * allocation. Clearing SCTLR.C would make all the data accesses
    * strongly ordered and would not hit the cache.
    */
    mrc    p15, 0, r0, c1, c0, 0
    bic    r0, r0, #(1 << 2)      @ Disable the C bit
    mcr    p15, 0, r0, c1, c0, 0
    isb

   /*
    * Invalidate L1 data cache. Even though only invalidate is
    * necessary exported flush API is used here. Doing clean
    * on already clean cache would be almost NOP.
    */
    push   {r0-r12, lr}
    ldr    r0, =v7_flush_dcache_all
    mov    lr, pc
    mov    pc, r0
    pop    {r0-r12, lr}

   /*
    * Execute an ISB instruction to ensure that all of the
    * CP15 register changes have been committed.
    */
    isb

   /*
    * Execute a barrier instruction to ensure that all cache,
    * TLB and branch predictor maintenance operations issued
    * by any CPU in the cluster have completed.
    */
    dsb
    dmb

   /* Clean L2 cache to write the dirty data into DRAM to make
    sure the data alignment between DRAM and L2 cache.
   */
#endif

#ifdef CONFIG_CACHE_L2X0
    /* Clean L2 cache here */
    ldr    r1, =L2_BASE_ADDR
    add    r1, r1, #PERIPBASE_VIRT
    /* Make way to 0xFFFF 16 ways */
    mov    r0, #0x10000
    sub    r0, r0, #0x1
    /* 0x7BC is L2X0_CLEAN_WAY */
    mov    r4, #0x700
    orr    r4, #0xBC
    str    r0, [r1, r4]
wait:
    ldr    r2, [r1, r4]
    ands    r2, r2, r0
    bne    wait
l2x0_sync:
    mov    r2, #0x0
    /* 0x730 is L2X0_CACHE_SYNC */
    mov    r4, #0x700
    orr    r4, #0x30
    str    r2, [r1, r4]
sync:
    ldr    r2, [r1, r4]
    ands    r2, r2, #0x1
    bne    sync
#endif

 /****************************************************************
 set ddr iomux to low power mode
 ****************************************************************/
#ifdef CONFIG_DRAM_IO


	ldr	r1, =MMDC_P0_BASE_ADDR
	add	r1, r1, #PERIPBASE_VIRT

	/* Put DDR explicitly into self-refresh. */
	 /* Disable Automatic power savings. */
	 ldr    r0, [r1, #0x404]
	 orr    r0, r0, #0x01
	 str    r0, [r1, #0x404]

	 /* Make the DDR explicitly enter self-refresh. */
	 ldr     r0, [r1, #0x404]
	 orr    r0, r0, #0x200000
	 str     r0, [r1, #0x404]

poll_dvfs_set_dormant:
	 ldr     r0, [r1, #0x404]
	 and     r0, r0, #0x2000000
	 cmp    r0, #0x2000000
	 bne     poll_dvfs_set_dormant

	ldr    r1, =MX6Q_IOMUXC_BASE_ADDR
	add    r1, r1, #PERIPBASE_VIRT

	bl ddr_io_set_lpm

#endif

/****************************************************************
; save resume pointer into SRC_GPR1
****************************************************************/
//    ;ldr    r0, =mx6q_suspend
//    ;ldr    r1, =resume
//    ;sub    r1, r1, r0
//    ;add    r3, r3, r1
//    ;ldr    r1, =SRC_BASE_ADDR
//    ;add    r1, r1, #PERIPBASE_VIRT
//    ;str    r3, [r1, #SRC_GPR1_OFFSET]

    ldr     r0, =0x020C4054
    ldr     r1,[r0,#0]
    and     r1, r1, #0x00000003
    cmp     r1, #1
    beq     suspend_to_wait_mode

suspend_to_stop_mode:
   /* For Lab126:
   Set the GPIOs to be at 1.8V */
   ldr     r1, =GPIO1_BASE_ADDR
   /* Drive the output low */
   ldr     r0, [r1, #0x0]
   bic     r0, r0, #0x400000
   str     r0, [r1, #0x0]

    ldr    r0, =resume
    ldr    r1, =SRC_BASE_ADDR
    add    r1, r1, #PERIPBASE_VIRT
    str    r0, [r1, #SRC_GPR1_OFFSET]

/****************************************************************
; execute a wfi instruction to let SOC go into stop mode.
****************************************************************/
    wfi

    nop
    nop
    nop
    nop

suspend_to_wait_mode:
    ldr    r0, =resume_from_wait_mode
    ldr    r1, =SRC_BASE_ADDR
    add    r1, r1, #PERIPBASE_VIRT
    str    r0, [r1, #SRC_GPR1_OFFSET]

//;    ; set soc voltage to 0.85V for power saving
//;    ; arm is also set to 0.85V since it can not run higher than soc
//;    ldr     r0,=0x020C8000
//;    ldr     r1,[r0,#0x140]
//;	bic     r1,#0x007C0000
//;	bic     r1,#0x0000001F
//;	orr     r1,#0x00180000
//;	orr     r1,#0x00400000
//;	orr     r1,#0x00000006
//;	orr     r1,#0x00000010
//;	str     r1,[r0,#0x140]

     ldr     r0, =CCM_BASE_ADDR
     ldr     r1, [r0, #CCM_CCGR7_OFFSET]
     and     r1,#0xf
     cmp     r1,#0x4
     bne     enter_suspend_wfi

     //; set ocram/ahb/ipg/mmdc clock to lower frequency for power saving
     //;                          prediv  bus div  clk
     //;ldr     r1,=0x06008100  ;    1       1      24
     //;ldr     r1,=0x06028900  ;    1       3      8
     //;ldr     r1,=0x1E000103  ;    4       1      6
     //;ldr     r1,=0x06079D38  ;    1       8      3
     //;ldr     r1,=0x3E079D3F  ;    8       8      0.375

     ldr    r1, =MMDC_P0_BASE_ADDR

     /* Disable Automatic power savings. */
     ldr    r0, [r1, #0x404]
     orr    r0, r0, #0x01
     str    r0, [r1, #0x404]

     /* Make the DDR explicitly enter self-refresh. */
     ldr     r0, [r1, #0x404]
     orr    r0, r0, #0x200000
     str     r0, [r1, #0x404]

poll_dvfs_set_1:
     ldr     r0, [r1, #0x404]
     and     r0, r0, #0x2000000
     cmp    r0, #0x2000000
     bne     poll_dvfs_set_1

     /* set SBS step-by-step mode */
     ldr     r0, [r1, #0x410]
     orr     r0, r0, #0x100
     str     r0, [r1, #0x410]

     ldr     r0,=CCM_BASE_ADDR

     // change podf for MMDC
     ldr     r1, [r0, #CCM_CBCDR_OFFSET]
     orr     r1, r1, #0x7
     str     r1, [r0, #CCM_CBCDR_OFFSET]
/*
podf_loop3:
     ldr     r1, [r0, #0x48]
     cmp     r1, #0x0
     bne     podf_loop3
*/
     // change podf for MMDC
     ldr     r1, [r0, #CCM_CBCDR_OFFSET]
     bic     r1, r1, #0x38
     orr     r1, r1, #0x10
     str     r1, [r0, #CCM_CBCDR_OFFSET]

podf_loop1:
     ldr     r1, [r0, #0x48]
     cmp     r1, #0x0
     bne     podf_loop1

     // change podf for AHB, IPG and OCRAM
     ldr     r1, [r0, #CCM_CBCDR_OFFSET]
     orr     r1, r1, #0x1C00
     orr     r1, r1, #0x100
     orr     r1, r1, #0x70000
     str     r1, [r0, #CCM_CBCDR_OFFSET]

podf_loop2:
     ldr     r1, [r0, #0x48]
     cmp     r1, #0x0
     bne     podf_loop2

/****************************************************************
; execute a wfi instruction to let SOC go into wait mode.
****************************************************************/
    dsb

enter_suspend_wfi:
    wfi

    nop
    nop
    nop
    nop

/****************************************************************
; if go here, means there is a wakeup irq pending, we should resume
; system immediately.
****************************************************************/
    mov    r0, r2                        /* get suspend_iram_base */
    add    r0, r0, #IRAM_SUSPEND_SIZE    /* 4K */

 #ifdef CONFIG_DRAM_IO
    ldr    r1, =MX6Q_IOMUXC_BASE_ADDR
    add    r1, r1, #PERIPBASE_VIRT

    bl ddr_io_restore
#endif
//;   mrc    p15, 0, r1, c1, c0, 0
//;   orr    r1, r1, #(1 << 2)    @ Enable the C bit
//;   mcr    p15, 0, r1, c1, c0, 0

    b     out                            /* exit standby */

/****************************************************************
; when SOC exit wait mode, LDO, PLL and clock needs to be
; reconfigured again since they won't recover automatically
****************************************************************/
//; resume:

resume_from_wait_mode:
//;    b resume

    ldr     r2,=CCM_BASE_ADDR
    ldr     r1,=CCM_CBCDR_ON_WAKEUP
    str     r1,[r2,#CCM_CBCDR_OFFSET]

//;    bl     toggle_gpio

//    ; offset for ANATOP
    ldr     r0,=0x020C8000

//    ; enabel ARM PLL
    mov     r1,#0x0
    orr     r1,r1,#0x00001000
    str     r1,[r0,#0x8]              @ use clear register

//	; set voltage back to norminal, arm = 1.1V,  soc = 1.1V
//	;mov     r1,#0x0
//	;orr     r1,#0x00400000
//	;orr     r1,#0x00000010
//	;str     r1,[r0,#0x140]

//    ; enabel 528 PLL
    mov     r1,#0x0
    orr     r1,r1,#0x00001000
    str     r1,[r0,#0x38]              @ use clear register

//    ; set MMDC clock div to 2
    ldr     r1,=0x06008101
    str     r1,[r2,#0x14]

//    ; wait ARM PLL to lock
wait_arm_pll_lock:
    ldr     r1,[r0,#0]
    and     r1, r1, #0x80000000
    cmp     r1, #0
    beq     wait_arm_pll_lock

//	; bring ARM PLL out of bypass mode
    mov     r1,#0x0
    orr     r1,r1,#0x00010000
    str     r1,[r0,#8]              @ use clear register

//    ;bl     toggle_gpio

//    ; wait 528 PLL to lock
//;wait_528_pll_lock
//;	 ldr     r0,=0x020C8030
//;    ldr     r1,[r0,#0]
//;    and     r1, r1, #0x80000000
//;    cmp     r1, #0
//;    beq     wait_528_pll_lock

//;    bl     toggle_gpio

//    ; set mmdc_ch1 divider to 1/2 since 528MHz is too high
//;    ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
//;    bic     r1, r1, #0x00000038
//;    orr     r1, r1, #0x00000008
//;    str     r1,[r0,#0]

//    ; bring 528 PLL out of bypass mode
//;    ldr     r0,=0x020C8030
    mov     r1,#0x0
    orr     r1,r1,#0x00010000
    str     r1,[r0,#0x38]              @ use clear register

//    ; enable 400MHz PFD
//;    ldr     r0, =0x020C8100
    mov     r1, #0x0
    orr     r1, r1, #0x00800000
    str     r1, [r0,#0x108]

//    ; select PFD as the source for ARM
//    ;ldr     r0, =0x020C400C
//    ;ldr     r1,[r0,#0]
//    ;orr     r1, r1, #0x00000100
//    ;orr     r1, r1, #0x00000004
//    ;str     r1,[r0,#0]

#if 0
//        ; select PFD as the source for mmdc ch1
    ldr     r0, =0x020C4014
    ldr     r1,[r0,#0]
    bic     r1, r1, #0x04000000
//;    str     r1,[r0,#0]

//    ; set mmdc_ch1 divider back to 1
//;	ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
    bic     r1, r1, #0x00000038 ;mmdc ch1
//;    str     r1,[r0,#0]

//	; set ahb divider to 4
//	; set ocram divider to 2
//;    ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
    bic     r1, r1, #0x00001C00 ;ahb
    bic     r1, r1, #0x00070000 ;ocram
    orr     r1, r1, #0x00000C00 ;ahb 1/4
    orr     r1, r1, #0x00010000 ;ocram 1/2
//;    str     r1,[r0,#0]

//    ; switch ahb/ocram source from OSC to 528 PLL
//;    ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
    bic     r1, r1, #0x02000000
    str     r1,[r0,#0]
#else
//    ;ldr     r0,=0x020C4000
    ldr     r1,=0x00038D00
    str     r1,[r2,#0x14]
#endif

//    ; select PFD as the source for mmdc ch1
//;    ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
//;    bic     r1, r1, #0x04000000
//;    str     r1,[r0,#0]

//    ; set mmdc_ch1 divider back to 1
//;	ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
//;    bic     r1, r1, #0x00000038 ;mmdc ch1
//;    str     r1,[r0,#0]

//	; set ahb divider to 4
//	; set ocram divider to 2
//;    ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
//;    bic     r1, r1, #0x00001C00 ;ahb
//;    bic     r1, r1, #0x00070000 ;ocram
//;    orr     r1, r1, #0x00000C00 ;ahb 1/4
//;    orr     r1, r1, #0x00010000 ;ocram 1/2
//;    str     r1,[r0,#0]

//    ; switch ahb/ocram source from OSC to 528 PLL
//;    ldr     r0, =0x020C4014
//;    ldr     r1,[r0,#0]
//;    bic     r1, r1, #0x02000000
//;    str     r1,[r0,#0]

    bl     toggle_gpio

    //; select ARM PLL as the source for ARM
    //;ldr     r0, =0x020C400C
    //;ldr     r1,[r0,#0]
    //;bic     r1, r1, #0x00000004
    //;str     r1,[r0,#0]

/****************************************************************
; when SOC exit stop mode, arm core restart from here, currently
; are running with MMU off.
****************************************************************/
resume:
//resume

/******************************************************************
; ARM SRPG Start
******************************************************************/

    /* Invalidate L1 I-cache first */
    mov    r1,    #0x0
    mcr p15, 0, r1, c7, c5, 0       @ Invalidate I-Cache

    //; enable i-cahce and branch predictor to make resume faster
    mcr p15,0,r1,c7,c5,6            @ invalidate branch predictor (this core)
	mov r1, #0x1800
	mcr p15,0,r1,c1,c0,0	        @ enable the Icache and branch prediction
	isb						        @ (as soon as possible)

/******************************************************************
; invalidate l1 dcache, r0-r4, r6, r7 used
******************************************************************/
#ifdef CONFIG_CACHE_L1
    mov     r0, #0
    mcr     p15, 2, r0, c0, c0, 0
    mrc     p15, 1, r0, c0, c0, 0

    ldr     r1, =0x7fff
    and     r2, r1, r0, lsr #13

    ldr     r1, =0x3ff

    and     r3, r1, r0, lsr #3    @ NumWays - 1
    add     r2, r2, #1        @ NumSets

    and     r0, r0, #0x7
    add     r0, r0, #4        @ SetShift

    clz     r1, r3            @ WayShift
    add     r4, r3, #1        @ NumWays
b1:
    sub     r2, r2, #1        @ NumSets--
    mov     r3, r4            @ Temp = NumWays
b2:
    subs    r3, r3, #1        @ Temp--
    mov     r7, r3, lsl r1
    mov     r6, r2, lsl r0
    orr     r7, r7, r6
    mcr     p15, 0, r7, c7, c6, 2
    bgt     b2
    cmp     r2, #0
    bgt     b1
    dsb
    isb

#endif

    ldr    r0, =SRC_BASE_ADDR
    str    r1, [r0, #SRC_GPR1_OFFSET]  /* clear SRC_GPR1 */
    ldr    r0, [r0, #SRC_GPR2_OFFSET]

#ifdef CONFIG_DRAM_IO
    ldr    r1, =MX6Q_IOMUXC_BASE_ADDR
    bl ddr_io_restore
#endif

 /* Restore cp15 registers */
    ldmea    r0!, {r4-r6}
    mov    sp, r4
    msr    spsr_cxsf, r5                @ Restore spsr
    mov    lr, r6                       @ Restore lr

    /* c1 and c2 registers */
    ldmea    r0!, {r4-r7}
    mcr    p15, 0, r4, c1, c0, 2   @ CPACR
    mcr    p15, 0, r5, c2, c0, 0   @ TTBR0
    mcr    p15, 0, r6, c2, c0, 1   @ TTBR1
    mcr    p15, 0, r7, c2, c0, 2   @ TTBCR

    /* c3 and c10 registers */
    ldmea    r0!,{r4-r7}
    mcr    p15, 0, r4, c3, c0, 0    @ DACR
    mcr    p15, 0, r5, c10, c2, 0   @ PRRR
    mcr    p15, 0, r6, c10, c2, 1   @ NMRR
    mcr    p15, 0, r7, c1, c0, 1    @ ACTLR

    /* c12, c13 and CPSR registers */
    ldmea    r0!,{r4-r7}
    mcr    p15, 0, r4, c13, c0, 1    @ Context ID
    mcr    p15, 0, r5, c13, c0, 2    @ User r/w thread ID
    mrc    p15, 0, r6, c12, c0, 0    @ Secure or NS VBAR
    msr    cpsr_cxfs, r7           @ store cpsr

#ifdef CONFIG_CACHE_L1
   /*
    * Enabling MMU here. Page entry needs to be altered
    * to create temporary 1:1 map and then resore the entry
    * ones MMU is enabled
    */
    mrc    p15, 0, r7, c2, c0, 2   @ Read TTBRControl
    and    r7, #0x7                @ Extract N (0:2) to decide
    cmp    r7, #0x0                @ TTBR0/TTBR1
    beq    use_ttbr0
ttbr_error:
    b    ttbr_error                @ Only N = 0 supported
use_ttbr0:
    mrc    p15, 0, r2, c2, c0, 0    @ Read TTBR0
    ldr    r5, =TTRBIT_MASK
    and    r2, r5
    mov    r4, pc
    ldr    r5, =TABLE_INDEX_MASK
    and    r4, r5                    @ r4 = 31 to 20 bits of pc
    ldr    r1, =TABLE_ENTRY
    add    r1, r1, r4                @ r1 has value of table entry
    lsr    r4, #18                   @ Address of table entry
    add    r2, r4                    @ r2 - location to be modified

    /* Storing previous entry of location being modified */
    ldr    r4, [r2]
    mov    r9, r4
    str    r1, [r2]

    /*
     * Storing address of entry being modified
     * It will be restored after enabling MMU
     */
    mov     r10, r2

    mov    r1, #0
    mcr    p15, 0, r1, c7, c5, 4   @ Flush prefetch buffer
    mcr    p15, 0, r1, c7, c5, 6   @ Invalidate BTB
    mcr    p15, 0, r1, c8, c5, 0   @ Invalidate ITLB
    mcr    p15, 0, r1, c8, c6, 0   @ Invalidate DTLB

    /*
     * Restore control register  but don't enable Data caches here.
     * Caches will be enabled after restoring MMU table entry.
     */
    ldmea    r0!, {r4}
    mov r11, r4
    ldr    r2, =CACHE_DISABLE_MASK
    and    r4, r4, r2
    orr    r4, r4, #0x1800
    mcr    p15, 0, r4, c1, c0, 0
    isb
    dsb
    ldr    r1, =mmu_on_label
    bx    r1
mmu_on_label:
    mov     r8, lr
    push    {r0}

    /* Set up the per-CPU stacks */
    bl    cpu_init
    pop    {r0}
#endif

#ifdef CONFIG_CACHE_L2X0
    ldr r2, =L2_BASE_ADDR
    add r2, r2, #PERIPBASE_VIRT

    ldmea    r0!, {r4-r7}
    /* L2 will be enabled after L1 is enabled */
    mov r4, #0x0
    str    r4, [r2, #L2X0_CTRL]
    str    r5, [r2, #L2X0_AUX_CTRL]
    str    r6, [r2, #L2X0_TAG_LATENCY_CTRL]
    str    r7, [r2, #L2X0_DATA_LATENCY_CTRL]

    ldmea    r0!, {r4-r5}
    str    r4, [r2, #L2X0_PREFETCH_CTRL]
    str    r5, [r2, #L2X0_POWER_CTRL]
#endif

#ifdef CONFIG_CACHE_L1
    /*
     * Restore the MMU table entry that was modified for
     * enabling MMU.
     */
    ldr    r4, =PAGE_OFFSET
    ldr    r5, =MX6_PHYS_OFFSET
    sub    r4, r4, r5
    add    r4, r4, r10
    str    r9, [r4]

    mov    r0, #0
    mcr    p15, 0, r0, c7, c1, 6    @ flush TLB and issue barriers
    mcr    p15, 0, r0, c7, c5, 4    @ Flush prefetch buffer
    mcr    p15, 0, r0, c7, c5, 6    @ Invalidate BTB
    mcr    p15, 0, r0, c8, c5, 0    @ Invalidate ITLB
    mcr    p15, 0, r0, c8, c6, 0    @ Invalidate DTLB
    dsb
    isb

/************************************************************
; restore control register to enable cache
************************************************************/
    mov    r0, r11
    mcr    p15, 0, r0, c1, c0, 0    @ with caches enabled.
    dsb
    isb
#endif

#ifdef CONFIG_CACHE_L2X0
    /* Enable L2 cache here */
    ldr r2, =L2_BASE_ADDR
    add r2, r2, #PERIPBASE_VIRT
    mov r4, #0x1
    str    r4, [r2, #L2X0_CTRL]
#endif

/***********************************************************
; return back to mx6_suspend_enter for dormant
***********************************************************/

//; restore lr from r8 since it is used when enabling L1 cache
#ifdef CONFIG_CACHE_L1
    mov    lr, r8
#endif

    mov    r1,    #0x0
    mcr p15, 0, r1, c7, c5, 0       @ Invalidate I-Cache

    ldmfd    sp!, {r0-r12}
    mov    pc, lr
/************************************************
; return back to mx6_suspend_enter for suspend
*************************************************/
out:
    ldmfd    sp!, {r0-r12}
    mov    pc, lr


/************************************************
; ddr io configuration save and restore
*************************************************/
//    .macro    ddr_io_save
    ddr_io_save:

	ldr	r4, [r1, #0x30c] /* DRAM_DQM0 */
	ldr	r5, [r1, #0x310] /* DRAM_DQM1 */
	ldr	r6, [r1, #0x314] /* DRAM_DQM2 */
	ldr	r7, [r1, #0x318] /* DRAM_DQM3 */
	stmfd	r0!, {r4-r7}

	ldr	r4, [r1, #0x5c4] /* GPR_B0DS */
	ldr	r5, [r1, #0x5cc] /* GPR_B1DS */
	ldr	r6, [r1, #0x5d4] /* GPR_B2DS */
	ldr	r7, [r1, #0x5d8] /* GPR_B3DS */
	stmfd	r0!, {r4-r7}

	ldr	r4, [r1, #0x300] /* DRAM_CAS */
	ldr	r5, [r1, #0x31c] /* DRAM_RAS */
	ldr	r6, [r1, #0x338] /* DRAM_SDCLK_0 */
	ldr	r7, [r1, #0x5ac] /* GPR_ADDS*/
	stmfd	r0!, {r4-r7}

	ldr	r4, [r1, #0x5b0] /* DDRMODE_CTL */
	ldr	r5, [r1, #0x5c0] /* DDRMODE */
	ldr	r6, [r1, #0x33c] /* DRAM_SODT0*/
	ldr	r7, [r1, #0x340] /* DRAM_SODT1*/
	stmfd	r0!, {r4-r7}

	ldr	r4, [r1, #0x330] /* DRAM_SDCKE0 */
	ldr	r5, [r1, #0x334] /* DRAM_SDCKE1 */
	ldr	r6, [r1, #0x320] /* DRAM_RESET */
	stmfd	r0!, {r4-r6}


    bx lr


//    .macro    ddr_io_restore
ddr_io_restore:

	ldmea	r0!, {r4-r7}
	str	r4, [r1, #0x30c] /* DRAM_DQM0 */
	str	r5, [r1, #0x310] /* DRAM_DQM1 */
	str	r6, [r1, #0x314] /* DRAM_DQM2 */
	str	r7, [r1, #0x318] /* DRAM_DQM3 */

	ldmea	r0!, {r4-r7}
	str	r4, [r1, #0x5c4] /* GPR_B0DS */
	str	r5, [r1, #0x5cc] /* GPR_B1DS */
	str	r6, [r1, #0x5d4] /* GPR_B2DS */
	str	r7, [r1, #0x5d8] /* GPR_B3DS */

	ldmea	r0!, {r4-r7}
	str	r4, [r1, #0x300] /* DRAM_CAS */
	str	r5, [r1, #0x31c] /* DRAM_RAS */
	str	r6, [r1, #0x338] /* DRAM_SDCLK_0 */
	str	r7, [r1, #0x5ac] /* GPR_ADDS*/

	ldmea	r0!, {r4-r7}
	str	r4, [r1, #0x5b0] /* DDRMODE_CTL */
	str	r5, [r1, #0x5c0] /* DDRMODE */
	str	r6, [r1, #0x33c] /* DRAM_SODT0*/
	str	r7, [r1, #0x340] /* DRAM_SODT1*/

	ldmea	r0!, {r4-r6}
	str	r4, [r1, #0x330] /* DRAM_SDCKE0 */
	str	r5, [r1, #0x334] /* DRAM_SDCKE1 */
	str	r6, [r1, #0x320] /* DRAM_RESET */

   bx lr


//    .macro    ddr_io_set_lpm
ddr_io_set_lpm:

	mov	r0, #0
	str	r0, [r1, #0x30c] /* DRAM_DQM0 */
	str	r0, [r1, #0x310] /* DRAM_DQM1 */
	str	r0, [r1, #0x314] /* DRAM_DQM2 */
	str	r0, [r1, #0x318] /* DRAM_DQM3 */

	str	r0, [r1, #0x5c4] /* GPR_B0DS */
	str	r0, [r1, #0x5cc] /* GPR_B1DS */
	str	r0, [r1, #0x5d4] /* GPR_B2DS */
	str	r0, [r1, #0x5d8] /* GPR_B3DS */

	str	r0, [r1, #0x300] /* DRAM_CAS */
	str	r0, [r1, #0x31c] /* DRAM_RAS */
	str	r0, [r1, #0x338] /* DRAM_SDCLK_0 */
	str	r0, [r1, #0x5ac] /* GPR_ADDS*/

	str	r0, [r1, #0x5b0] /* DDRMODE_CTL */
	str	r0, [r1, #0x5c0] /* DDRMODE */
	str	r0, [r1, #0x33c] /* DRAM_SODT0*/
	str	r0, [r1, #0x340] /* DRAM_SODT1*/

	mov	r0, #0x80000
	str	r0, [r1, #0x320] /* DRAM_RESET */
	mov	r0, #0x1000
	str	r0, [r1, #0x330] /* DRAM_SDCKE0 */
	str	r0, [r1, #0x334] /* DRAM_SDCKE1 */


    bx lr


cpu_init:
    bx lr

/*
 *    v7_flush_dcache_all()
 *
 *    Flush the whole D-cache.
 *
 *    Corrupted registers: r0-r7, r9-r11 (r6 only in Thumb mode)
 *
 *    - mm    - mm_struct describing address space
 */

v7_flush_dcache_all:
//.macro    v7_flush_dcache_all
    dmb                              @ ensure ordering with previous memory accesses
    mrc    p15, 1, r0, c0, c0, 1     @ read clidr
    ands   r3, r0, #0x7000000        @ extract loc from clidr
    mov    r3, r3, lsr #23           @ left align loc bit field
    beq    finished                  @ if loc is 0, then no need to clean
    mov    r10, #0                   @ start clean at cache level 0
loop1:
    add    r2, r10, r10, lsr #1      @ work out 3x current cache level
    mov    r1, r0, lsr r2            @ extract cache type bits from clidr
    and    r1, r1, #7                @ mask of the bits for current cache only
    cmp    r1, #2                    @ see what cache we have at this level
    blt    skip                      @ skip if no cache, or just i-cache
    mcr    p15, 2, r10, c0, c0, 0    @ select current cache level in cssr
    isb                              @ isb to sych the new cssr&csidr
    mrc    p15, 1, r1, c0, c0, 0     @ read the new csidr
    and    r2, r1, #7                @ extract the length of the cache lines
    add    r2, r2, #4                @ add 4 (line length offset)
    ldr    r4, =0x3ff
    ands    r4, r4, r1, lsr #3       @ find maximum number on the way size
    clz    r5, r4                    @ find bit position of way size increment
    ldr    r7, =0x7fff
    ands    r7, r7, r1, lsr #13      @ extract max number of the index size
loop2:
    mov    r9, r4                    @ create working copy of max way size
loop3:
    orr    r11, r10, r9, lsl r5      @ factor way and cache number into r11
    orr    r11, r11, r7, lsl r2      @ factor index number into r11
    mcr    p15, 0, r11, c7, c14, 2   @ clean & invalidate by set/way
    subs    r9, r9, #1               @ decrement the way
    bge    loop3
    subs    r7, r7, #1               @ decrement the index
    bge    loop2
skip:
    add    r10, r10, #2              @ increment cache number
    cmp    r3, r10
    bgt    loop1
finished:
    mov    r10, #0                   @ swith back to cache level 0
    mcr    p15, 2, r10, c0, c0, 0    @ select current cache level in cssr
    dsb
    isb
    mov    pc, lr

//; toggle gpio
toggle_gpio:

    ldr     r0,=0x020A4000
    ldr     r1,[r0,#0]
    orr     r1,r1,#0x00800000
    str     r1,[r0,#0]
    bic     r1,r1,#0x00800000
    str     r1,[r0,#0]

    bx lr



