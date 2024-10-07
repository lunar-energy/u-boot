/*
 * am335x_evm.h
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com/
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

/* Clock Defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#ifdef CONFIG_MTD_RAW_NAND
#define NANDARGS \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=NAND.file-system,2048\0" \
	"nandrootfstype=ubifs rootwait\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${fdtaddr} NAND.u-boot-spl-os; " \
		"nand read ${loadaddr} NAND.kernel; " \
		"bootz ${loadaddr} - ${fdtaddr}\0"
#else
#define NANDARGS ""
#endif

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel "=" \
	"run nandboot\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#if IS_ENABLED(CONFIG_CMD_USB)
# define BOOT_TARGET_USB(func) func(USB, usb, 0)
#else
# define BOOT_TARGET_USB(func)
#endif

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

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(NAND, nand, 0) \
	BOOT_TARGET_USB(func) \
	BOOT_TARGET_PXE(func) \
	BOOT_TARGET_DHCP(func)

#include <config_distro_bootcmd.h>

#ifndef CONFIG_SPL_BUILD
#include <env/ti/dfu.h>

#define CFG_EXTRA_ENV_SETTINGS \
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
#define CFG_SYS_NS16550_COM1		0x44e09000	/* Base EVM has UART0 */
#define CFG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CFG_SYS_NS16550_COM3		0x48024000	/* UART2 */
#define CFG_SYS_NS16550_COM4		0x481a6000	/* UART3 */
#define CFG_SYS_NS16550_COM5		0x481a8000	/* UART4 */
#define CFG_SYS_NS16550_COM6		0x481aa000	/* UART5 */

#ifdef CONFIG_MTD_RAW_NAND
/* NAND: device related configs */
/* NAND: driver related configs */
#define CFG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	14
#endif /* !CONFIG_MTD_RAW_NAND */

/* USB Device Firmware Update support */
#ifndef CONFIG_SPL_BUILD
#define DFUARGS \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_NAND
#endif

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
#define CFG_SYS_FLASH_BASE		(0x08000000)
#define CFG_SYS_FLASH_SIZE		0x01000000
#endif  /* NOR support */

#endif	/* ! __CONFIG_AM335X_EVM_H */
