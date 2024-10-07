// SPDX-License-Identifier: GPL-2.0+
/*
 * board.c
 *
 * Board functions for TI AM335X based boards
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <errno.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <net.h>
#include <spl.h>
#include <serial.h>
#include <asm/arch/cpu.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk_synthesizer.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/emif.h>
#include <asm/gpio.h>
#include <asm/omap_common.h>
#include <asm/omap_mmc.h>
#include <i2c.h>
#include <miiphy.h>
#include <cpsw.h>
#include <linux/bitops.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <env_internal.h>
#include <watchdog.h>
#include "../common/board_detect.h"
#include "../common/cape_detect.h"
#include "board.h"
#include <net.h>
#include <asm/davinci_rtc.h>

DECLARE_GLOBAL_DATA_PTR;

/* GPIO that controls power to DDR on EVM-SK */
#define GPIO_TO_PIN(bank, gpio)		(32 * (bank) + (gpio))
#define GPIO_DDR_VTT_EN		GPIO_TO_PIN(0, 7)
#define ICE_GPIO_DDR_VTT_EN	GPIO_TO_PIN(0, 18)
#define GPIO_PR1_MII_CTRL	GPIO_TO_PIN(3, 4)
#define GPIO_MUX_MII_CTRL	GPIO_TO_PIN(3, 10)
#define GPIO_FET_SWITCH_CTRL	GPIO_TO_PIN(0, 7)
#define GPIO_PHY_RESET		GPIO_TO_PIN(2, 19)
#define GPIO_ETH0_MODE		GPIO_TO_PIN(0, 11)
#define GPIO_ETH1_MODE		GPIO_TO_PIN(1, 26)

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

#define GPIO0_RISINGDETECT	(AM33XX_GPIO0_BASE + OMAP_GPIO_RISINGDETECT)
#define GPIO1_RISINGDETECT	(AM33XX_GPIO1_BASE + OMAP_GPIO_RISINGDETECT)

#define GPIO0_IRQSTATUS1	(AM33XX_GPIO0_BASE + OMAP_GPIO_IRQSTATUS1)
#define GPIO1_IRQSTATUS1	(AM33XX_GPIO1_BASE + OMAP_GPIO_IRQSTATUS1)

#define GPIO0_IRQSTATUSRAW	(AM33XX_GPIO0_BASE + 0x024)
#define GPIO1_IRQSTATUSRAW	(AM33XX_GPIO1_BASE + 0x024)

/*
 * Save the boot device to RTC scratch0 from SPL so that we can
 * access it from the u-boot environment.
 */

#ifdef CONFIG_SPL_BUILD
static void save_boot_device(void)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;
	u32 boot_device;

	writel(RTC_KICK0R_WE, &reg->kick0r);
	writel(RTC_KICK1R_WE, &reg->kick1r);

	boot_device = spl_boot_device();
	writel(boot_device, &reg->scratch0);
}
#endif

u32 load_boot_device(void)
{
	struct davinci_rtc *reg =
		(struct davinci_rtc *)CONFIG_SYS_BOOTCOUNT_ADDR;

	return readl(&reg->scratch0);
}

/* Store the boot device in the environment variable 'boot_device' */
static void env_set_boot_device(void)
{
	u32 boot_device;

	boot_device = load_boot_device();
	switch (boot_device) {
		case BOOT_DEVICE_MMC1: {
			env_set("boot_device", "sdcard");
			break;
		}
		case BOOT_DEVICE_MMC2: {
			env_set("boot_device", "emmc");
			break;
		}
		default: {
			env_set("boot_device", "unknown");
			break;
		}
	}
}

void am33xx_spl_board_init(void)
{
	save_boot_device();
}

/*
 * Read header information from EEPROM into global structure.
 */
#ifdef CONFIG_TI_I2C_BOARD_DETECT
void do_board_detect(void)
{
	enable_i2c0_pin_mux();
	enable_i2c2_pin_mux();
	if (ti_i2c_eeprom_am_get(CONFIG_EEPROM_BUS_ADDRESS, CONFIG_EEPROM_CHIP_ADDRESS))
	{
		printf("ti_i2c_eeprom_init failed\n");
	}
}
#endif

#ifndef CONFIG_DM_SERIAL
struct serial_device *default_serial_console(void)
{
	return &eserial1_device;
}
#endif

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
#ifdef CONFIG_SPL_SERIAL
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;
#endif

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_load();
	if (env_get_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

const struct dpll_params *get_dpll_ddr_params(void)
{
	int ind = get_sys_clk_index();
	return &dpll_ddr3_303MHz[ind];
}

const struct dpll_params *get_dpll_mpu_params(void)
{
	int ind = get_sys_clk_index();
	int freq = 0;
	freq = MPUPLL_M_1000;
	switch (freq) {
	case MPUPLL_M_1000:
		return &dpll_mpu_opp[ind][5];
	case MPUPLL_M_800:
		return &dpll_mpu_opp[ind][4];
	case MPUPLL_M_720:
		return &dpll_mpu_opp[ind][3];
	case MPUPLL_M_600:
		return &dpll_mpu_opp[ind][2];
	case MPUPLL_M_500:
		return &dpll_mpu_opp100;
	case MPUPLL_M_300:
		return &dpll_mpu_opp[ind][0];
	}

	return &dpll_mpu_opp[ind][0];
}

void gpi2c_init(void)
{
	/* When needed to be invoked prior to BSS initialization */
	static bool first_time = true;

	if (first_time) {
		enable_i2c0_pin_mux();
		first_time = false;
	}
}

void scale_vcores(void)
{
	int freq;

	gpi2c_init();
	freq = am335x_get_efuse_mpu_max_freq(cdev);
}

void set_uart_mux_conf(void)
{
#if CONFIG_CONS_INDEX == 1
	enable_uart0_pin_mux();
#elif CONFIG_CONS_INDEX == 2
	enable_uart1_pin_mux();
#elif CONFIG_CONS_INDEX == 3
	enable_uart2_pin_mux();
#elif CONFIG_CONS_INDEX == 4
	enable_uart3_pin_mux();
#elif CONFIG_CONS_INDEX == 5
	enable_uart4_pin_mux();
#elif CONFIG_CONS_INDEX == 6
	enable_uart5_pin_mux();
#endif
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

const struct ctrl_ioregs ioregs_evmsk = {
	.cm0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J128MJT125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J128MJT125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_bonelt = {
	.cm0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.cm2ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt0ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
	.dt1ioctl		= MT41K256M16HA125E_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs_evm15 = {
	.cm0ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.cm1ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.cm2ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.dt0ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
	.dt1ioctl		= MT41J512M8RH125_IOCTRL_VALUE,
};

const struct ctrl_ioregs ioregs = {
	.cm0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.cm2ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt0ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
	.dt1ioctl		= MT47H128M16RT25E_IOCTRL_VALUE,
};

static const struct ddr_data	ddr3_HOMECONTROLLER_ddr_data	= {
	.datardsratio0 =	0x00000040	,
	.datawdsratio0 =	0x0000007D	,
	.datafwsratio0 =	0x000000E8	,
	.datawrsratio0 = 	0x000000BD	,
};

static const struct cmd_control 	ddr3_HOMECONTROLLER_cmd_ctrl_data	= {
	.cmd0csratio = 	0x00000100	,
	.cmd0iclkout = 	0x00000001	,
	.cmd1csratio = 	0x00000100	,
	.cmd1iclkout = 	0x00000001	,
	.cmd2csratio = 	0x00000100	,
	.cmd2iclkout = 	0x00000001	,
};

static struct emif_regs 	ddr3_HOMECONTROLLER_emif_reg_data	= {
	.sdram_config = 0x61A05332	,
	.ref_ctrl = 	0x0000093B	,
	.zq_config = 	0x50074BE1	,
	.sdram_tim1 = 	0x0888A39B 	,
	.sdram_tim2 = 	0x44517FDA	,
	.sdram_tim3 = 	0x50FFE4EF	,
	.emif_ddr_phy_ctlr_1 = 	0x00100208	,
};

const struct ctrl_ioregs 	ddr3_HOMECONTROLLER_ioregs_data	= {
	.cm0ioctl =	0x0000018B	,
	.cm1ioctl =	0x0000018B	,
	.cm2ioctl =	0x0000018B	,
	.dt0ioctl =	0x0000018B	,
	.dt1ioctl =	0x0000018B	,
};


void sdram_init(void)
{
	config_ddr(303, &ddr3_HOMECONTROLLER_ioregs_data,
		&ddr3_HOMECONTROLLER_ddr_data,
		&ddr3_HOMECONTROLLER_cmd_ctrl_data,
		&ddr3_HOMECONTROLLER_emif_reg_data, 0);
}
#endif

#if (!defined(CONFIG_SPL_BUILD) || \
	(defined(CONFIG_SPL_ETH) && defined(CONFIG_SPL_BUILD)))
static void request_and_set_gpio(int gpio, char *name, int val)
{
	int ret;

	ret = gpio_request(gpio, name);
	if (ret < 0) {
		printf("%s: Unable to request %s\n", __func__, name);
		return;
	}

	ret = gpio_direction_output(gpio, 0);
	if (ret < 0) {
		printf("%s: Unable to set %s  as output\n", __func__, name);
		goto err_free_gpio;
	}

	gpio_set_value(gpio, val);

	return;

err_free_gpio:
	gpio_free(gpio);
}
#endif

#define REQUEST_AND_SET_GPIO(N)	request_and_set_gpio(N, #N, 1);
#define REQUEST_AND_CLR_GPIO(N)	request_and_set_gpio(N, #N, 0);

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_CONTROL) && \
	defined(CONFIG_DM_ETH) && defined(CONFIG_DRIVER_TI_CPSW)

#define MAX_CPSW_SLAVES	2

/* At the moment, we do not want to stop booting for any failures here */
int ft_board_setup(void *fdt, struct bd_info *bd)
{

	return 0;
}
#endif

/*
 * Basic board specific setup.  Pinmux has been handled already.
 */
int board_init(void)
{
	u32 sys_reboot;

	sys_reboot = readl(PRM_RSTST);
	if (sys_reboot & (1 << 9))
		puts("RESET: IcePick reset has occurred\n");

	if (sys_reboot & (1 << 5))
		puts("RESET: Global external warm reset has occurred\n");

	if (sys_reboot & (1 << 4))
		puts("RESET: watchdog reset has occurred\n");

	if (sys_reboot & (1 << 1))
		puts("RESET: Global warm SW reset has occurred\n");

	if (sys_reboot & (1 << 0))
		puts("RESET: Power-on reset has occurred\n");

#if defined(CONFIG_HW_WATCHDOG)
	hw_watchdog_init();
#endif

	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;
#if defined(CONFIG_NOR) || defined(CONFIG_MTD_RAW_NAND)
	gpmc_init();
#endif

	REQUEST_AND_SET_GPIO(GPIO_PHY_RESET);
	puts("ENET:  Reset Phy\n");
	/* Reset PHYs to capture the Jumper setting */
	gpio_set_value(GPIO_PHY_RESET, 0);
	udelay(1000);
	gpio_set_value(GPIO_PHY_RESET, 1);
	udelay(1000);

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;
#if !defined(CONFIG_SPL_BUILD)
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char *name = NULL;

	if (board_is_lehc())
	{
		name = "LEHC";
	}

	set_board_info_env(name);

	/*
	 * Default FIT boot on HS devices. Non FIT images are not allowed
	 * on HS devices.
	 */
	if (get_device_type() == HS_DEVICE)
		env_set("boot_fit", "1");
#endif

#if !defined(CONFIG_SPL_BUILD)
	/* try reading mac address from efuse */
	mac_lo = readl(&cdev->macid0l);
	mac_hi = readl(&cdev->macid0h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!env_get("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
	}

	mac_lo = readl(&cdev->macid1l);
	mac_hi = readl(&cdev->macid1h);
	mac_addr[0] = mac_hi & 0xFF;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	mac_addr[4] = mac_lo & 0xFF;
	mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	if (!env_get("eth1addr")) {
		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("eth1addr", mac_addr);
	}
#endif

	if (!env_get("serial#")) {
		char *board_serial = env_get("board_serial");
		char *ethaddr = env_get("ethaddr");

		if (!board_serial || !strncmp(board_serial, "unknown", 7))
			env_set("serial#", ethaddr);
		else
			env_set("serial#", board_serial);
	}

	env_set_boot_device();

	/* Just probe the potentially supported cdce913 device */
	uclass_get_device_by_name(UCLASS_CLK, "cdce913@65", &dev);

	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	if (board_is_lehc() && !strcmp(name, "am335x-hc"))
		return 0;
	else
		return -1;
}
#endif

#if !CONFIG_IS_ENABLED(OF_CONTROL)
static const struct omap_hsmmc_plat am335x_mmc0_plat = {
	.base_addr = (struct hsmmc *)OMAP_HSMMC1_BASE,
	.cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_4BIT,
	.cfg.f_min = 400000,
	.cfg.f_max = 52000000,
	.cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195,
	.cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

U_BOOT_DRVINFO(am335x_mmc0) = {
	.name = "omap_hsmmc",
	.plat = &am335x_mmc0_plat,
};

static const struct omap_hsmmc_plat am335x_mmc1_plat = {
	.base_addr = (struct hsmmc *)OMAP_HSMMC2_BASE,
	.cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS | MMC_MODE_8BIT,
	.cfg.f_min = 400000,
	.cfg.f_max = 52000000,
	.cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195,
	.cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

U_BOOT_DRVINFO(am335x_mmc1) = {
	.name = "omap_hsmmc",
	.plat = &am335x_mmc1_plat,
};
#endif
