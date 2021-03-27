# SPDX-License-Identifier: GPL-2.0
menuconfig ARCH_BCM
	bool "Broadcom SoC Support"
	depends on ARCH_MULTI_V6_V7
	help
	  This enables support for Broadcom ARM based SoC chips

if ARCH_BCM

comment "IPROC architected SoCs"

config ARCH_BCM_IPROC
	bool
	select ARM_GIC
	select CACHE_L2X0
	select HAVE_ARM_SCU if SMP
	select HAVE_ARM_TWD if SMP
	select ARM_GLOBAL_TIMER
	select CLKSRC_MMIO
	select GPIOLIB
	select ARM_AMBA
	select PINCTRL
	select PCI_DOMAINS if PCI
	help
	  This enables support for systems based on Broadcom IPROC architected SoCs.
	  The IPROC complex contains one or more ARM CPUs along with common
	  core peripherals. Application specific SoCs are created by adding a
	  uArchitecture containing peripherals outside of the IPROC complex.
	  Currently supported SoCs are Cygnus.

config ARCH_BCM_CYGNUS
	bool "Broadcom Cygnus Support"
	depends on ARCH_MULTI_V7
	select ARCH_BCM_IPROC
	help
	  Enable support for the Cygnus family,
	  which includes the following variants:
	  BCM11300, BCM11320, BCM11350, BCM11360,
	  BCM58300, BCM58302, BCM58303, BCM58305.

config ARCH_BCM_HR2
	bool "Broadcom Hurricane 2 SoC support"
	depends on ARCH_MULTI_V7
	select ARCH_BCM_IPROC
	help
	  Enable support for the Hurricane 2 family,
	  which includes the following variants:
	  BCM53342, BCM53343, BCM53344, BCM53346.

config ARCH_BCM_NSP
	bool "B