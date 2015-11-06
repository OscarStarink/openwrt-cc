/*
 *  Domino board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 alzhao <alzhao@gmail.com>
 *  Copyright (C) 2014 Michel Stempin <michel.stempin@wanadoo.fr>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DOMINO_GPIO_LED_WLAN		13
#define DOMINO_GPIO_LED_WAN			14
#define DOMINO_GPIO_BTN_RESET		16


#define DOMINO_KEYS_POLL_INTERVAL	20	/* msecs */
#define DOMINO_KEYS_DEBOUNCE_INTERVAL	(3 * DOMINO_KEYS_POLL_INTERVAL)

#define DOMINO_MAC0_OFFSET	0x0000
#define	DOMINO_MAC1_OFFSET	0x0000
#define DOMINO_CALDATA_OFFSET	0x1000
#define DOMINO_WMAC_MAC_OFFSET	0x0000

static struct gpio_led domino_leds_gpio[] __initdata = {
	{
		.name = "gl_ar300:wlan",
		.gpio = DOMINO_GPIO_LED_WLAN,
		.active_low = 1,
	},
	{
		.name = "gl_ar300:wan",
		.gpio = DOMINO_GPIO_LED_WAN,
		.active_low = 1,
	},
};

static struct gpio_keys_button domino_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = DOMINO_KEYS_DEBOUNCE_INTERVAL,
		.gpio = DOMINO_GPIO_BTN_RESET,
		.active_low = 1,
	},
};

static void __init domino_setup(void)
{

	/* ART base address */
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	//ath79_setup_ar933x_phy4_switch(false, false);

	/* register flash. */
	ath79_register_m25p80(NULL);

	/* register gpio LEDs and keys */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(domino_leds_gpio),
				 domino_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DOMINO_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(domino_gpio_keys),
					domino_gpio_keys);

	/* enable usb */
	ath79_register_usb();
	ath79_register_mdio(1, 0x0);

	/* register eth0 as WAN, eth1 as LAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, art+DOMINO_MAC0_OFFSET, 0);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	
	ath79_register_eth(0);

	ath79_init_mac(ath79_eth1_data.mac_addr, art+DOMINO_MAC1_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	/* register wireless mac with cal data */
	ath79_register_wmac(art + DOMINO_CALDATA_OFFSET, art + DOMINO_WMAC_MAC_OFFSET);
}

MIPS_MACHINE(ATH79_MACH_GL_AR300, "GL-AR300", "GL AR300",
	     domino_setup);