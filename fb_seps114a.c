/*
 * Custom FB driver for SEPS114A display
 *
 * Copyright (C) 2013 Noralf Tronnes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include "fbtft.h"

#define DRVNAME		"fb_seps114a"
#define WIDTH		96
#define HEIGHT		96

#define SEPS114A_SOFT_RESET 0x01
#define SEPS114A_DISPLAY_ON_OFF 0x02
#define SEPS114A_ANALOG_CONTROL 0x0F    //
#define SEPS114A_STANDBY_ON_OFF 0x14
#define SEPS114A_OSC_ADJUST 0x1A
#define SEPS114A_ROW_SCAN_DIRECTION 0x09
#define SEPS114A_DISPLAY_X1 0x30
#define SEPS114A_DISPLAY_X2 0x31
#define SEPS114A_DISPLAY_Y1 0x32
#define SEPS114A_DISPLAY_Y2 0x33
#define SEPS114A_DISPLAYSTART_X 0x38
#define SEPS114A_DISPLAYSTART_Y 0x39
#define SEPS114A_CPU_IF 0x0D
#define SEPS114A_MEM_X1 0x34
#define SEPS114A_MEM_X2 0x35
#define SEPS114A_MEM_Y1 0x36
#define SEPS114A_MEM_Y2 0x37
#define SEPS114A_MEMORY_WRITE_READ 0x1D
#define SEPS114A_DDRAM_DATA_ACCESS_PORT 0x08
#define SEPS114A_DISCHARGE_TIME 0x18
#define SEPS114A_PEAK_PULSE_DELAY 0x16
#define SEPS114A_PEAK_PULSE_WIDTH_R 0x3A
#define SEPS114A_PEAK_PULSE_WIDTH_G 0x3B
#define SEPS114A_PEAK_PULSE_WIDTH_B 0x3C
#define SEPS114A_PRECHARGE_CURRENT_R 0x3D
#define SEPS114A_PRECHARGE_CURRENT_G 0x3E
#define SEPS114A_PRECHARGE_CURRENT_B 0x3F
#define SEPS114A_COLUMN_CURRENT_R 0x40
#define SEPS114A_COLUMN_CURRENT_G 0x41
#define SEPS114A_COLUMN_CURRENT_B 0x42
#define SEPS114A_ROW_OVERLAP 0x48
#define SEPS114A_SCAN_OFF_LEVEL 0x49
#define SEPS114A_ROW_SCAN_ON_OFF 0x17
#define SEPS114A_ROW_SCAN_MODE 0x13
#define SEPS114A_SCREEN_SAVER_CONTEROL 0xD0
#define SEPS114A_SS_SLEEP_TIMER 0xD1
#define SEPS114A_SCREEN_SAVER_MODE 0xD2
#define SEPS114A_SS_UPDATE_TIMER 0xD3
#define SEPS114A_RGB_IF 0xE0
#define SEPS114A_RGB_POL 0xE1
#define SEPS114A_DISPLAY_MODE_CONTROL 0xE5

static int init_display(struct fbtft_par *par)
{
	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	//par->fbtftops.reset(par);

	if (par->gpio.reset != -1) {
		gpio_set_value(par->gpio.reset, 0);
		mdelay(10);
		gpio_set_value(par->gpio.reset, 1);
		mdelay(10);
	}

	/*  Soft reset */
	write_reg(par, SEPS114A_SOFT_RESET,0x00);      
	
	/* Standby ON/OFF*/
	write_reg(par, SEPS114A_STANDBY_ON_OFF,0x01);          // Standby on
	mdelay(5);                                           // Wait for 5ms (1ms Delay Minimum)
	write_reg(par, SEPS114A_STANDBY_ON_OFF,0x00);          // Standby off
	mdelay(5);                                           // 1ms Delay Minimum (1ms Delay Minimum)
	
	/* Display OFF */
	write_reg(par, SEPS114A_DISPLAY_ON_OFF,0x00);
	
	/* Set Oscillator operation */
	write_reg(par, SEPS114A_ANALOG_CONTROL,0x00);          // using external resistor and internal OSC
	
	/* Set frame rate */
	write_reg(par, SEPS114A_OSC_ADJUST,0x03);              // frame rate : 95Hz
	
	/* Set active display area of panel */
	write_reg(par, SEPS114A_DISPLAY_X1,0x00);
	write_reg(par, SEPS114A_DISPLAY_X2,0x5F);
	write_reg(par, SEPS114A_DISPLAY_Y1,0x00);
	write_reg(par, SEPS114A_DISPLAY_Y2,0x5F);
	
	/* Select the RGB data format and set the initial state of RGB interface port */
	write_reg(par, SEPS114A_RGB_IF,0x00);                 // RGB 8bit interface
	
	/* Set RGB polarity */
	write_reg(par, SEPS114A_RGB_POL,0x00);
	
	/* Set display mode control */
	write_reg(par, SEPS114A_DISPLAY_MODE_CONTROL,0x80);   // SWAP:BGR, Reduce current : Normal, DC[1:0] : Normal
	
	/* Set MCU Interface */
	write_reg(par, SEPS114A_CPU_IF,0x00);                 // MPU External interface mode, 8bits
	
	/* Set Memory Read/Write mode */
	write_reg(par, SEPS114A_MEMORY_WRITE_READ,0x00);
	
	/* Set row scan direction */
	write_reg(par, SEPS114A_ROW_SCAN_DIRECTION,0x00);     // Column : 0 --> Max, Row : 0 --> Max
	
	/* Set row scan mode */
	write_reg(par, SEPS114A_ROW_SCAN_MODE,0x00);          // Alternate scan mode
	
	/* Set column current */
	write_reg(par, SEPS114A_COLUMN_CURRENT_R,0x6E);
	write_reg(par, SEPS114A_COLUMN_CURRENT_G,0x4F);
	write_reg(par, SEPS114A_COLUMN_CURRENT_B,0x77);
	
	/* Set row overlap */
	write_reg(par, SEPS114A_ROW_OVERLAP,0x00);            // Band gap only
	
	/* Set discharge time */
	write_reg(par, SEPS114A_DISCHARGE_TIME,0x01);         // Discharge time : normal discharge
	
	/* Set peak pulse delay */
	write_reg(par, SEPS114A_PEAK_PULSE_DELAY,0x00);
	
	/* Set peak pulse width */
	write_reg(par, SEPS114A_PEAK_PULSE_WIDTH_R,0x02);
	write_reg(par, SEPS114A_PEAK_PULSE_WIDTH_G,0x02);
	write_reg(par, SEPS114A_PEAK_PULSE_WIDTH_B,0x02);
	
	/* Set precharge current */
	write_reg(par, SEPS114A_PRECHARGE_CURRENT_R,0x14);
	write_reg(par, SEPS114A_PRECHARGE_CURRENT_G,0x50);
	write_reg(par, SEPS114A_PRECHARGE_CURRENT_B,0x19);
	
	/* Set row scan on/off  */
	write_reg(par, SEPS114A_ROW_SCAN_ON_OFF,0x00);        // Normal row scan
	
	/* Set scan off level */
	write_reg(par, SEPS114A_SCAN_OFF_LEVEL,0x04);         // VCC_C*0.75
	
	/* Set memory access point */
	write_reg(par, SEPS114A_DISPLAYSTART_X,0x00);
	write_reg(par, SEPS114A_DISPLAYSTART_Y,0x00);
	
	/* Display ON */
	write_reg(par, SEPS114A_DISPLAY_ON_OFF,0x01);

	write_reg(par, SEPS114A_MEMORY_WRITE_READ,0x02);

	// Memory write
	write_reg(par, SEPS114A_DDRAM_DATA_ACCESS_PORT);


	char white[2] = {0xff,0xff};
	int j=0;
	for(j=0;j<9216;j++)
	{
		if (par->gpio.dc != -1)
			gpio_set_value(par->gpio.dc, 1);
		par->fbtftops.write(par, white, 2);
	}

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int start_x, int start_y, int end_x, int end_y)
{
	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		"%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, start_x, start_y, end_x, end_y);

	write_reg(par, SEPS114A_MEMORY_WRITE_READ,0x02);
	// Column address
	write_reg(par, SEPS114A_MEM_X1, start_x);
	write_reg(par, SEPS114A_MEM_X2, end_x);

	// Row adress
	write_reg(par, SEPS114A_MEM_Y1, start_y);
	write_reg(par, SEPS114A_MEM_Y2, end_y);
}

static void seps114a_display_reset(struct fbtft_par *par) {
	if (par->gpio.reset != -1) {
		gpio_set_value(par->gpio.reset, 0);
		mdelay(10);
		gpio_set_value(par->gpio.reset, 1);
		mdelay(10);
	}
}


static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.fps = 95,
	.fbtftops = {
		.reset = seps114a_display_reset,
		.init_display = init_display,
		.set_addr_win = set_addr_win,
	},
};
FBTFT_REGISTER_DRIVER(DRVNAME, "syncoam,seps114a", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("spi:seps114a");

MODULE_DESCRIPTION("Custom FB driver for seps114a display");
MODULE_AUTHOR("Noralf Tronnes");
MODULE_LICENSE("GPL");

