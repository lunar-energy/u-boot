/*
 * mux.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>
#include <asm/arch/mux.h>
#include <asm/io.h>
#include <i2c.h>
#include "../common/board_detect.h"
#include "board.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART0_RXD */
	{OFFSET(uart0_txd), (MODE(0) | PULLUDEN)},		/* UART0_TXD */
	{-1},
};

static struct module_pin_mux uart1_pin_mux[] = {
	{OFFSET(uart1_rxd), (MODE(0) | PULLUP_EN | RXACTIVE)},	/* UART1_RXD */
	{OFFSET(uart1_txd), (MODE(0) | PULLUDEN)},		/* UART1_TXD */
	{-1},
};

static struct module_pin_mux uart2_pin_mux[] = {
	{OFFSET(spi0_sclk), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART2_RXD */
	{OFFSET(spi0_d0), (MODE(1) | PULLUDEN)},		/* UART2_TXD */
	{-1},
};

static struct module_pin_mux uart3_pin_mux[] = {
	{OFFSET(spi0_cs1), (MODE(1) | PULLUP_EN | RXACTIVE)},	/* UART3_RXD */
	{OFFSET(ecap0_in_pwm0_out), (MODE(1) | PULLUDEN)},	/* UART3_TXD */
	{-1},
};

static struct module_pin_mux uart4_pin_mux[] = {
	{OFFSET(gpmc_wait0), (MODE(6) | PULLUP_EN | RXACTIVE)},	/* UART4_RXD */
	{OFFSET(gpmc_wpn), (MODE(6) | PULLUDEN)},		/* UART4_TXD */
	{-1},
};

static struct module_pin_mux uart5_pin_mux[] = {
	{OFFSET(lcd_data9), (MODE(4) | PULLUP_EN | RXACTIVE)},	/* UART5_RXD */
	{OFFSET(lcd_data8), (MODE(4) | PULLUDEN)},		/* UART5_TXD */
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_dat3), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT3 */
	{OFFSET(mmc0_dat2), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT2 */
	{OFFSET(mmc0_dat1), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT1 */
	{OFFSET(mmc0_dat0), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_DAT0 */
	{OFFSET(mmc0_clk), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | RXACTIVE | PULLUP_EN)},	/* MMC0_CMD */
	{OFFSET(mcasp0_aclkr), (MODE(4) | RXACTIVE)},		/* MMC0_WP */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
	{OFFSET(gpmc_ad7), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT7 */
	{OFFSET(gpmc_ad6), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT6 */
	{OFFSET(gpmc_ad5), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT5 */
	{OFFSET(gpmc_ad4), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT4 */
	{OFFSET(gpmc_ad3), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT3 */
	{OFFSET(gpmc_ad2), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT2 */
	{OFFSET(gpmc_ad1), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT1 */
	{OFFSET(gpmc_ad0), (MODE(1) | RXACTIVE | PULLUP_EN)},	/* MMC1_DAT0 */
	{OFFSET(gpmc_csn1), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CLK */
	{OFFSET(gpmc_csn2), (MODE(2) | RXACTIVE | PULLUP_EN)},	/* MMC1_CMD */
	{OFFSET(gpmc_csn0), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_WP */
	{OFFSET(gpmc_advn_ale), (MODE(7) | RXACTIVE | PULLUP_EN)},	/* MMC1_CD */
	{-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_DATA */
	{OFFSET(i2c0_scl), (MODE(0) | RXACTIVE |
			PULLUDEN | SLEWCTRL)}, /* I2C_SCLK */
	{-1},
};

static struct module_pin_mux i2c2_pin_mux[] = {
	{OFFSET(uart1_ctsn), (MODE(3) | RXACTIVE |
			PULLUDEN | PULLUP_EN | SLEWCTRL)},	/* I2C_DATA */
	{OFFSET(uart1_rtsn), (MODE(3) | RXACTIVE |
			PULLUDEN | PULLUP_EN | SLEWCTRL)},	/* I2C_SCLK */
	{-1},
};

static struct module_pin_mux rmii1_pin_mux[] = {
	{OFFSET(mdio_clk), MODE(0) | PULLUP_EN},	/* MDIO_CLK */
	{OFFSET(mdio_data), MODE(0) | RXACTIVE | PULLUP_EN}, /* MDIO_DATA */
	{OFFSET(mii1_crs), MODE(1) | RXACTIVE},		/* MII1_CRS */
	{OFFSET(mii1_rxerr), MODE(1) | RXACTIVE},	/* MII1_RXERR */
	{OFFSET(mii1_txen), MODE(1)},			/* MII1_TXEN */
	{OFFSET(mii1_txd1), MODE(1)},			/* MII1_TXD1 */
	{OFFSET(mii1_txd0), MODE(1)},			/* MII1_TXD0 */
	{OFFSET(mii1_rxd1), MODE(1) | RXACTIVE},	/* MII1_RXD1 */
	{OFFSET(mii1_rxd0), MODE(1) | RXACTIVE},	/* MII1_RXD0 */
	{OFFSET(rmii1_refclk), MODE(0) | RXACTIVE},	/* RMII1_REFCLK */
	{-1},
};

#ifdef CONFIG_MTD_RAW_NAND
static struct module_pin_mux nand_pin_mux[] = {
	{OFFSET(gpmc_ad0),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD0  */
	{OFFSET(gpmc_ad1),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD1  */
	{OFFSET(gpmc_ad2),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD2  */
	{OFFSET(gpmc_ad3),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD3  */
	{OFFSET(gpmc_ad4),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD4  */
	{OFFSET(gpmc_ad5),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD5  */
	{OFFSET(gpmc_ad6),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD6  */
	{OFFSET(gpmc_ad7),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD7  */
#ifdef CONFIG_SYS_NAND_BUSWIDTH_16BIT
	{OFFSET(gpmc_ad8),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD8  */
	{OFFSET(gpmc_ad9),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD9  */
	{OFFSET(gpmc_ad10),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD10 */
	{OFFSET(gpmc_ad11),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD11 */
	{OFFSET(gpmc_ad12),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD12 */
	{OFFSET(gpmc_ad13),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD13 */
	{OFFSET(gpmc_ad14),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD14 */
	{OFFSET(gpmc_ad15),	(MODE(0) | PULLUDDIS | RXACTIVE)}, /* AD15 */
#endif
	{OFFSET(gpmc_wait0),	(MODE(0) | PULLUP_EN | RXACTIVE)}, /* nWAIT */
	{OFFSET(gpmc_wpn),	(MODE(7) | PULLUP_EN)},		   /* nWP */
	{OFFSET(gpmc_csn0),	(MODE(0) | PULLUP_EN)},		   /* nCS */
	{OFFSET(gpmc_wen),	(MODE(0) | PULLDOWN_EN)},	   /* WEN */
	{OFFSET(gpmc_oen_ren),	(MODE(0) | PULLDOWN_EN)},	   /* OE */
	{OFFSET(gpmc_advn_ale),	(MODE(0) | PULLDOWN_EN)},	   /* ADV_ALE */
	{OFFSET(gpmc_be0n_cle),	(MODE(0) | PULLDOWN_EN)},	   /* BE_CLE */
	{-1},
};
#elif defined(CONFIG_NOR)
static struct module_pin_mux bone_norcape_pin_mux[] = {
	{OFFSET(gpmc_a0), MODE(0) | PULLUDDIS},			/* NOR_A0 */
	{OFFSET(gpmc_a1), MODE(0) | PULLUDDIS},			/* NOR_A1 */
	{OFFSET(gpmc_a2), MODE(0) | PULLUDDIS},			/* NOR_A2 */
	{OFFSET(gpmc_a3), MODE(0) | PULLUDDIS},			/* NOR_A3 */
	{OFFSET(gpmc_a4), MODE(0) | PULLUDDIS},			/* NOR_A4 */
	{OFFSET(gpmc_a5), MODE(0) | PULLUDDIS},			/* NOR_A5 */
	{OFFSET(gpmc_a6), MODE(0) | PULLUDDIS},			/* NOR_A6 */
	{OFFSET(gpmc_a7), MODE(0) | PULLUDDIS},			/* NOR_A7 */
	{OFFSET(gpmc_ad0), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD0 */
	{OFFSET(gpmc_ad1), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD1 */
	{OFFSET(gpmc_ad2), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD2 */
	{OFFSET(gpmc_ad3), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD3 */
	{OFFSET(gpmc_ad4), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD4 */
	{OFFSET(gpmc_ad5), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD5 */
	{OFFSET(gpmc_ad6), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD6 */
	{OFFSET(gpmc_ad7), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD7 */
	{OFFSET(gpmc_ad8), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD8 */
	{OFFSET(gpmc_ad9), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD9 */
	{OFFSET(gpmc_ad10), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD10 */
	{OFFSET(gpmc_ad11), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD11 */
	{OFFSET(gpmc_ad12), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD12 */
	{OFFSET(gpmc_ad13), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD13 */
	{OFFSET(gpmc_ad14), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD14 */
	{OFFSET(gpmc_ad15), MODE(0) | PULLUDDIS | RXACTIVE},	/* NOR_AD15 */
	{OFFSET(gpmc_csn0), MODE(0) | PULLUDEN | PULLUP_EN},     /* CE */
	{OFFSET(gpmc_advn_ale), MODE(0) | PULLUDEN | PULLDOWN_EN}, /* ALE */
	{OFFSET(gpmc_oen_ren), MODE(0) | PULLUDEN | PULLDOWN_EN},/* OEn_REN */
	{OFFSET(gpmc_be0n_cle), MODE(0) | PULLUDEN | PULLDOWN_EN},/* unused */
	{OFFSET(gpmc_wen), MODE(0) | PULLUDEN | PULLDOWN_EN},    /* WEN */
	{OFFSET(gpmc_wait0), MODE(0) | PULLUDEN | PULLUP_EN | RXACTIVE},/*WAIT*/
	{-1},
};
#endif

#if defined(CONFIG_NOR_BOOT)
void enable_norboot_pin_mux(void)
{
	configure_module_pin_mux(bone_norcape_pin_mux);
}
#endif

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_uart1_pin_mux(void)
{
	configure_module_pin_mux(uart1_pin_mux);
}

void enable_uart2_pin_mux(void)
{
	configure_module_pin_mux(uart2_pin_mux);
}

void enable_uart3_pin_mux(void)
{
	configure_module_pin_mux(uart3_pin_mux);
}

void enable_uart4_pin_mux(void)
{
	configure_module_pin_mux(uart4_pin_mux);
}

void enable_uart5_pin_mux(void)
{
	configure_module_pin_mux(uart5_pin_mux);
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}

void enable_i2c2_pin_mux(void)
{
	configure_module_pin_mux(i2c2_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(rmii1_pin_mux);
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);
}
