/*
 * am335x_hc.h
 *
 * Based on am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_AM335X_EVM_H
#define __CONFIG_AM335X_EVM_H

#include <configs/ti_am335x_common.h>
#include <linux/sizes.h>

#ifndef CONFIG_SPL_BUILD
# define CONFIG_TIMESTAMP
#endif

#define CONFIG_BOOT_RETRY_TIME 60
#define CONFIG_RESET_TO_RETRY

#define CONFIG_SYS_BOOTM_LEN		SZ_16M

#define CONFIG_MACH_TYPE		MACH_TYPE_AM335XEVM

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

/* Custom script for NOR */
#define CONFIG_SYS_LDSCRIPT		"board/ti/lunar_hc/u-boot.lds"

#define BOOTENV_DEV_LEGACY_MMC(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
	"setenv devtype mmc; " \
	"setenv mmcdev " #instance"; "\
	"setenv bootpart " #instance":1 ; "\
	"run boot\0"

#define BOOTENV_DEV_NAME_LEGACY_MMC(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#if CONFIG_IS_ENABLED(CMD_PXE)
# define BOOT_TARGET_PXE(func) func(PXE, pxe, na)
#else
# define BOOT_TARGET_PXE(func)
#endif

#if CONFIG_IS_ENABLED(CMD_DHCP)
# define BOOT_TARGET_DHCP(func) func(DHCP, dhcp, na)
#else
# define BOOT_TARGET_DHCP(func)
#endif

#include <config_distro_bootcmd.h>

#ifndef CONFIG_SPL_BUILD
#include <environment/ti/dfu.h>
#include <environment/ti/mmc.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttyS0,115200n8\0" \
	"search_path=/ /boot/\0" \
	"environment_file=uEnv.txt\0" \
	"bootscript_file=boot.scr\0" \
	"env_load_addr=0x82000000\0" \
	"bootscript_load_addr=0x82000000\0" \
	"fdt_file=am335x-hc.dtb\0" \
	"fdt_load_addr=0x88000000\0" \
	"kernel_file=zImage\0" \
	"kernel_load_addr=0x82000000\0" \
	"bootlimit=2\0" \
	"rootfs_rollback=0\0" \
	"boot_system=echo Booting system from ${boot_device_type}${root_partition} (${boot_device}); " \
		"bootz ${kernel_load_addr} - ${fdt_load_addr}; " \
		"panic \"ERROR: Failed to boot system\"\0" \
	"set_kernel_args=setenv bootargs console=${console} " \
		"root=PARTUUID=${root_partition_uuid} ro,noload,norecovery rootwait ${optargs}; " \
		"echo Setting kernel boot args to ${bootargs}\0" \
	"get_rootfs_uuid=part uuid ${boot_device_type} ${root_partition} root_partition_uuid; " \
		"echo UUID for root partition is ${root_partition_uuid}\0" \
	"load_kernel=if test -e ${boot_device_type} ${root_partition} ${boot_dir}${kernel_file}; then " \
		"if load ${boot_device_type} ${root_partition} ${kernel_load_addr} ${boot_dir}${kernel_file}; then " \
		"echo Loaded kernel image ${boot_dir}${kernel_file} from ${boot_device_type}${root_partition} to ${kernel_load_addr}; else " \
		"panic \"ERROR: Failed to load kernel image ${boot_dir}${file_file} from ${boot_device_type}${root_partition}\"; fi; " \
		"else panic \"ERROR: Failed to find kernel image ${boot_dir}${file_file} on ${boot_device_type}${root_partition}\"; fi;\0" \
	"load_fdt=if test -e ${boot_device_type} ${root_partition} ${boot_dir}${fdt_file}; then " \
		"if load ${boot_device_type} ${root_partition} ${fdt_load_addr} ${boot_dir}${fdt_file}; then " \
		"echo Loaded FDT file ${boot_dir}${fdt_file} from ${boot_device_type}${root_partition} to ${fdt_load_addr}; else " \
		"panic \"ERROR: Failed to load FDT file ${boot_dir}${fdt_file} from ${boot_device_type}${root_partition}\"; fi; " \
		"else panic \"ERROR: Failed to find FDT file ${boot_dir}${fdt_file} on ${boot_device_type}${root_partition}\"; fi;\0" \
	"set_fdt_name=echo Board is ${board_name}; " \
		"if test ${board_name} = LEHC; then " \
		"setenv fdt_file am335x-hc.dtb; else " \
		"echo \"WARN: Unable to set FDT name for board ${board_name} using default\"; fi; " \
		"echo Setting FDT filename to ${fdt_file};\0" \
	"run_bootscript=echo Running bootscript ...; source ${bootscript_load_addr}; echo WARN: Failed to run bootscript\0" \
	"load_bootscript=echo Loading bootscript file ${path}${bootscript_file} ...; " \
		"load ${boot_device_type} ${root_partition} ${bootscript_load_addr} ${path}${bootscript_file}\0" \
	"check_for_bootscript=echo Scanning ${boot_device_type}${root_partition} for bootscript file ...; " \
		"for path in ${search_path}; " \
		"do if test -e ${boot_device_type} ${root_partition} ${path}${bootscript_file}; then " \
		"if run load_bootscript; then run run_bootscript; else echo WARN: Failed to load bootscript file; fi; " \
		"fi; done\0" \
	"check_for_rollback=echo Selected root partition ${boot_device_type}${root_partition}; " \
		"if test $rootfs_rollback = 1; then " \
		"if test $boot_device = emmc; then " \
		"if test $root_partition = ${boot_device_num}:2; then " \
		"setenv root_partition ${boot_device_num}:3; else " \
		"setenv root_partition ${boot_device_num}:2; fi; " \
		"echo Alternative boot - falling back to root partition ${boot_device_type}${root_partition}; fi; fi;\0" \
	"import_environment=echo Importing environment ...; env import -t ${env_load_addr} ${filesize}\0" \
	"load_environment=echo Loading environment file ${path}${environment_file} ...; " \
		"load ${boot_device_type} ${boot_partition} ${env_load_addr} ${path}${environment_file}\0" \
	"check_for_environment=echo Scanning ${boot_device_type}${boot_partition} for environment file ...; " \
		"for path in ${search_path}; " \
		"do if test -e ${boot_device_type} ${boot_partition} ${path}${environment_file}; then " \
		"if run load_environment; then run import_environment; else echo WARN: Failed to load environment file; fi; " \
		"fi; done\0" \
	"set_partition_config=setenv boot_device_type mmc; setenv boot_dir /boot/; if test $boot_device = sdcard; then " \
		"setenv boot_device_num 0; setenv boot_partition ${boot_device_num}:1; " \
		"setenv root_partition ${boot_device_num}:1; else " \
		"if test $boot_device = emmc; then setenv boot_device_num 1; setenv boot_partition ${boot_device_num}:1; " \
		"setenv root_partition ${boot_device_num}:2; else " \
		"panic \"ERROR: Unknown boot device\"; fi; fi; " \
		"mmc dev ${boot_device_num}; " \
		"echo Selected boot partition ${boot_device_type}${boot_partition} (${boot_device})\0" \
	"bootcmd=run set_partition_config; " \
		"run check_for_environment; " \
		"run check_for_rollback; " \
		"run check_for_bootscript; " \
		"run set_fdt_name; " \
		"run load_fdt; " \
		"run load_kernel; " \
		"run get_rootfs_uuid; " \
		"run set_kernel_args; " \
		"run boot_system\0" \
	"altbootcmd=echo Alternative boot triggered ...; " \
		"setenv rootfs_rollback 1; " \
		"run bootcmd\0"
#endif

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CONFIG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CONFIG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CONFIG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#define CONFIG_DM_ETH

#ifndef CONFIG_NOR_BOOT
/* Bootcount using the RTC block */
#define CONFIG_SYS_BOOTCOUNT_BE
#endif

/*
 * For NOR boot, we must set this to the start of where NOR is mapped
 * in memory.
 */

/*
 * USB configuration.  We enable MUSB support, both for host and for
 * gadget.  We set USB0 as peripheral and USB1 as host, based on the
 * board schematic and physical port wired to each.  Then for host we
 * add mass storage support and for gadget we add both RNDIS ethernet
 * and DFU.
 */
#define CONFIG_USB_MUSB_DISABLE_BULK_COMBINE_SPLIT
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_PERIPHERAL
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

/*
 * Disable MMC DM for SPL build and can be re-enabled after adding
 * DM support in SPL
 */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_MMC
#undef CONFIG_TIMER
#endif

/* USB Device Firmware Update support */
#ifndef CONFIG_SPL_BUILD
#define DFUARGS \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_NAND
#endif

#if defined(CONFIG_SPI_BOOT)
/* SPL related */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE		(4 << 10) /* 4 KB sectors */
#define CONFIG_ENV_OFFSET		(768 << 10) /* 768 KiB in */
#define CONFIG_ENV_OFFSET_REDUND	(896 << 10) /* 896 KiB in */
#elif defined(CONFIG_EMMC_BOOT)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		0
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_SYS_MMC_MAX_DEVICE	2
#elif defined(CONFIG_NOR_BOOT)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		(512 << 10)	/* 512 KiB */
#define CONFIG_ENV_OFFSET_REDUND	(768 << 10)	/* 768 KiB */
#elif defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_ENV_OFFSET		0x001c0000
#define CONFIG_ENV_OFFSET_REDUND	0x001e0000
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_SYS_NAND_BLOCK_SIZE
#endif

/* SPI flash. */

/* Network. */
#define CONFIG_PHY_SMSC

/*
 * NOR Size = 16 MiB
 * Number of Sectors/Blocks = 128
 * Sector Size = 128 KiB
 * Word length = 16 bits
 * Default layout:
 * 0x000000 - 0x07FFFF : U-Boot (512 KiB)
 * 0x080000 - 0x09FFFF : First copy of U-Boot Environment (128 KiB)
 * 0x0A0000 - 0x0BFFFF : Second copy of U-Boot Environment (128 KiB)
 * 0x0C0000 - 0x4BFFFF : Linux Kernel (4 MiB)
 * 0x4C0000 - 0xFFFFFF : Userland (11 MiB + 256 KiB)
 */
#if defined(CONFIG_NOR)
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BASE		(0x08000000)
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#endif  /* NOR support */


#endif	/* ! __CONFIG_AM335X_EVM_H */




