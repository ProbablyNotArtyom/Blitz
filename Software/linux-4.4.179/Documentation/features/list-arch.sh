// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

/dts-v1/;

#include "rk3036.dtsi"

/ {
	model = "Rockchip RK3036 KylinBoard";
	compatible = "rockchip,rk3036-kylin", "rockchip,rk3036";

	memory@60000000 {
		device_type = "memory";
		reg = <0x60000000 0x20000000>;
	};

	leds: gpio-leds {
		compatible = "gpio-leds";

		work {
			gpios = <&gpio2 RK_PD6 GPIO_ACTIVE_HIGH>;
			label = "kylin:red:led";
			pinctrl-names = "default";
			pinctrl-0 = <&led_ctl>;
		};
	};

	sdio_pwrseq: sdio-pwrseq {
		compatible = "mmc-pwrseq-simple";
		pinctrl-names = "default";
		pinctrl-0 = <&bt_wake_h>;

		/*
		 * On the module itself this is one of these (depending
		 * on the actual card populated):
		 * - 