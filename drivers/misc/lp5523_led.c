/*
 * led_control.c
 *
 * Copyright 2015 Amazon Technologies, Inc. All Rights Reserved.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */
#include <common.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/err.h>

#include <lp5523.h>

#define I2C_LED_BUS_NUM		CONFIG_LP5523_I2C_BUS
#define DEFAULT_GPIO_HOLD_TIME		12000 /* 12 ms */

/* Register Address Mapping */
#define LP5523_REG_ENABLE		0x00
#define LP5523_RESET_REG		0x3d
#define LP5523_REG_CONFIG		0x36
#define LP5523_REG_GPO		0x3B
#define LP5523_AUTO_INC		0x40
#define LP5523_PWR_SAVE		0x20
#define LP5523_PWM_PWR_SAVE		0x04
#define LP5523_CP_AUTO		0x18
#define LP5523_AUTO_CLK		0x02
#define LP5523_REG_LED_PWM_BASE		0x16
#define LP5523_REG_LED_CURRENT_BASE		0x26
#define LP5523_REG_ENABLE_LEDS_MSB		0x04
#define LP5523_REG_ENABLE_LEDS_LSB		0x05
#define LP52232_RESET		0xFF
#define LP5523_ENABLE		0x40
#define LP5523_CMD_LOAD		0x15
#define LP5523_CMD_RUN		0x2a
#define LP5523_CMD_DISABLED		0x00
#define LP5523_REG_OP_MODE		0x01
#define LP5523_REG_PROG_MEM			0x50
#define LP5523_REG_PROG_PAGE_SEL		0x4f
#define LP5523_REG_CH1_PROG_START		0x4c
#define LP5523_REG_CH2_PROG_START		0x4d
#define LP5523_REG_CH3_PROG_START		0x4e
#define LP5523_GPIO_HIGH		0x5

#define LP5523_LEDS		9
#define LP5523_PAGE_SIZE		32
#define LED_CURRENT_DEFAULT		150 /* 15 mA */

/// Master handle used on registration
struct led_control* g_master_handle = NULL;

/*
 * i2c_initialize - Initialize the I2C controller for particular bus
 */
static void i2c_initialize(struct i2c_data* const i2c_handle)
{
	if (NULL == i2c_handle)
		return;

	/* Selecting the bus num will initialize this bus */
	i2c_set_bus_num(i2c_handle->bus_num);
}

/*
 * send_i2c_cmd - Send I2C command
 */
static int send_i2c_cmd(u8* const address_list, u8 reg, u8 value)
{
	int ret = -1;
	int i = 0;

	if (!address_list) {
		printf("send_i2c_cmd - invalid address_list\n");
		return ret;
	}

	while (address_list[i] != 0) {
		if ((ret = i2c_write(address_list[i], reg, 1, &value, 1)) != 0) {
			printf("i2c_write failed: addr=0x%x val=0x%x\n", address_list[i],
					value);
			return ret;
		}
		i++;
	}

	return ret;
}

/*
 * read_i2c - Read I2C data
 */
static int read_i2c_reg(u8* const address_list, u8 reg, u8* data, u32 data_len)
{
	int ret = -1;
	int i = 0;

	if (!address_list || !data_len)
		return ret;

	/* Read reg for each address in list */
	while (address_list[i] != 0) {
		if (i >= data_len)
			return -1;

		if ((ret = i2c_read(address_list[i], reg, 1, &data[i], 1)) != 0) {
			printf("i2c_read failed: addr=0x%x val=0x%x\n", address_list[i],
					data[i]);
			return ret;
		}

		i++;
	}

	return ret;
}

/*
 * send_i2c_cmd_no_ack - Send I2C command, expect no ack and reset the controller
 */
static void send_i2c_cmd_no_ack(u8* address_list, u8 reg, u8 value)
{
	int i = 0;

	if (!address_list)
		return;

	while (address_list[i] != 0) {
		// Ignore errors
		i2c_write(address_list[i], reg, 1, &value, 1);
		i++;
	}
}
/*
 * gpio_toggle - Toggle gpio
 */
static void gpio_toggle(struct gpio_data* const gp_handle, u32 hold_time_us)
{
	u8 active_high = 1;

	if (NULL == gp_handle)
		return;

	if (gp_handle->gpio_num) {
		if (gp_handle->active_low)
			active_high = 0;
	}

	gpio_direction_output(gp_handle->gpio_num, active_high ? 0 : 1);
	udelay(hold_time_us);
	gpio_direction_output(gp_handle->gpio_num, active_high ? 1 : 0);
}

/*
 * lp5523_reset - Manually reset each controller and re-initialize the I2C bus
 */
static void lp5523_reset(struct led_control* const handle)
{
	int i = 0;
	u8 address[] = { 0, 0 };
	int size = ARRAY_SIZE(handle->driver_info.i2c_info.address_list);

	for (i = 0; i < size; i++) {
		address[0] = handle->driver_info.i2c_info.address_list[i];
		/* Soft reset -  Send reset i2c command */
		// abc123: Avoid reset
		send_i2c_cmd_no_ack(address, LP5523_RESET_REG, LP52232_RESET);

		/* Toggle the enable line */
		udelay(20000); /* wait ~20ms */
		gpio_toggle(&handle->driver_info.gpio_info, DEFAULT_GPIO_HOLD_TIME);
		udelay(2000); /* wait 2ms */

		/* lp5523 will not respond with an ACK after a reset.
		 * HACK: Re-initialize the I2C controller to handle other transactions
		 */
		i2c_initialize(&handle->driver_info.i2c_info);
	}
}

static int is_pattern_active(struct led_control* const handle)
{
	u32 read_data_size = ARRAY_SIZE(handle->driver_info.i2c_info.address_list);
	u8 reg_data[read_data_size];
	u32 i = 0;

	if (read_i2c_reg(handle->driver_info.i2c_info.address_list,
			LP5523_REG_OP_MODE, reg_data, read_data_size))
		return 0;

	for (i = 0; i < read_data_size; ++i) {
		if (!(reg_data[i] & LP5523_CMD_RUN))
			return 0;
	}

	/* All engines are running */
	return 1;
}

/*
 * lp5523_init - Initialize the lp5523 led driver
 */
static int lp5523_init(struct led_control* const handle)
{
	int ret = 0;
	int led_count = 0;

	// initialize I2C
	i2c_initialize(&handle->driver_info.i2c_info);

	// Check if we already have a pattern running
	if (is_pattern_active(handle)) {
		printf("pattern is active\n");
		return 1;
	}

	/* Enable line */
	gpio_toggle(&handle->driver_info.gpio_info, DEFAULT_GPIO_HOLD_TIME);
	udelay(2000); /* wait 2ms */

	/* Send soft reset commands */
	lp5523_reset(handle);

	ret = send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_ENABLE, LP5523_ENABLE);
	udelay(2000); /* wait 2ms */

	ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_CONFIG,
			LP5523_AUTO_INC | LP5523_PWR_SAVE | LP5523_CP_AUTO | LP5523_AUTO_CLK
					| LP5523_PWM_PWR_SAVE);
	/* Set the default led current */
	for (led_count = 0; led_count < LP5523_LEDS; ++led_count) {
		ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list,
				LP5523_REG_LED_CURRENT_BASE + led_count, LED_CURRENT_DEFAULT);
	}

	/* Set INT to GPIO and high */
	ret = send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_GPO, LP5523_GPIO_HIGH);

	return ret;
}

/*
 * lp5523_load_pattern - Load pattern data to LP5523 SRAM
 */
static int lp5523_load_pattern(struct pattern_info *pattern)
{
	u8 prog_page = 0;
	int offset = 0;
	int i = 0;
	int ret = 0;
	int pattern_size = pattern->size;
	u8 *address_list = pattern->address_list;
	// Disable the engine
	ret = send_i2c_cmd(address_list, LP5523_REG_OP_MODE, LP5523_CMD_DISABLED);

	// Set the program start
	ret |= send_i2c_cmd(address_list, LP5523_REG_CH1_PROG_START,
			pattern->engine1_start);
	ret |= send_i2c_cmd(address_list, LP5523_REG_CH2_PROG_START,
			pattern->engine2_start);
	ret |= send_i2c_cmd(address_list, LP5523_REG_CH3_PROG_START,
			pattern->engine3_start);

	// Set the engine mode to load
	ret |= send_i2c_cmd(address_list, LP5523_REG_OP_MODE, LP5523_CMD_LOAD);

	// Send pattern sequence
	for (i = 0; i < pattern_size && (ret == 0); i++) {
		// Write the page select
		if (!(i % LP5523_PAGE_SIZE)) {
			ret |= send_i2c_cmd(address_list, LP5523_REG_PROG_PAGE_SEL,
					prog_page);
			prog_page++;
			offset = 0;
		}
		// Write data to selected page
		ret |= send_i2c_cmd(address_list, LP5523_REG_PROG_MEM + offset,
				pattern->data[i]);
		offset++;
	}

	return ret;
}

/**
 * Trigger pattern start
 */
static int run_pattern(struct led_control* const handle)
{
	int ret = 0;
	// Run the pattern
	ret = send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_ENABLE, (LP5523_CMD_RUN | LP5523_ENABLE));

	ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_OP_MODE, LP5523_CMD_RUN);

	return ret;
}

/*
 * lp5523_pattern - Display pattern for lp5523 led driver
 */
static int lp5523_pattern(struct led_control* const handle, LED_PATTERN_T color)
{
	int i = 0;
	int num_leds = LP5523_LEDS; /* Max = LP5523_LEDS */
	u8 brightness = 0xFF; /* Max intensity */
	u8 channels[LP5523_LEDS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 }; /* LED Channels */
	int ret = 0;

	if (NULL == handle) {
		printf("invalid led handle\n");
		return -1;
	}

	switch (color) {
	case LED_PATTERN_SOLID_RED:
		channels[0] = 6;
		channels[1] = 7;
		channels[2] = 8;
		num_leds = 3; /* 3 R-channels */
		break;
	case LED_PATTERN_SOLID_BLUE:
		channels[0] = 1;
		channels[1] = 3;
		channels[2] = 5;
		num_leds = 3; /* 3 B-channels */
		break;
	case LED_PATTERN_SOLID_GREEN:
		channels[0] = 0;
		channels[1] = 2;
		channels[2] = 4;
		num_leds = 3; /* 3 G-channels */
		break;
	case LED_PATTERN_SOLID_WHITE:
		num_leds = LP5523_LEDS;
		break;
	case LED_PATTERN_NONE:
		num_leds = LP5523_LEDS;
		brightness = 0;
		break;
	default:
		return -1;
	}
	/* Enable all leds */
	ret = send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_ENABLE_LEDS_MSB, 0x1);
	ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list,
			LP5523_REG_ENABLE_LEDS_LSB, 0xFF);
	/* Drive appropriate pwm per color channel */
	for (i = 0; i < num_leds && (ret == 0); i++) {
		ret |= send_i2c_cmd(handle->driver_info.i2c_info.address_list,
				LP5523_REG_LED_PWM_BASE + channels[i], brightness);
	}

	return ret;
}

int lp5523_register_control(struct led_control * cntrl)
{
	if (!cntrl)
		return -1;

	g_master_handle = cntrl;

	if (lp5523_init(g_master_handle) != 0) {
		printf("lp5523 init failed\n");
		return -1;
	}

	return 0;
}

int lp5523_download_animation(struct pattern_info * pattern)
{
	if (!g_master_handle || !pattern)
		return -1;
	return lp5523_load_pattern(pattern);
}

int lp5523_enable_animation()
{
	if (!g_master_handle)
		return -1;
	return run_pattern(g_master_handle);
}

int lp5523_pattern_active(void)
{
	if (!g_master_handle)
		return -1;
	return is_pattern_active(g_master_handle);
}

int lp5523_show_pattern(LED_PATTERN_T color)
{
	if (!g_master_handle)
		return -1;

	return lp5523_pattern(g_master_handle, color);
}



