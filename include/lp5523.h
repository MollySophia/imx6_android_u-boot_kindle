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
#ifndef __LP5523_H__
#define __LP5523_H__
/*
 * Enums
 */
typedef enum led_colors {
	LED_PATTERN_INVALID,
	LED_PATTERN_NONE,
	LED_PATTERN_SOLID_RED,
	LED_PATTERN_SOLID_BLUE,
	LED_PATTERN_SOLID_GREEN,
	LED_PATTERN_SOLID_WHITE
} LED_PATTERN_T;

enum led_cmd {
	LED_ON,
	LED_OFF
};

/*
 * Structs
 */
struct gpio_data {
	u16 gpio_num;
	u16 active_low;
};

struct i2c_data {
	u32 bus_num;
	u8* address_list;
};

struct control_data {
	struct gpio_data gpio_info;
	struct i2c_data i2c_info;
};

struct led_control {
	u16 board_rev;
	struct control_data driver_info;
	struct control_data power_led_info;
	void (*enable_power_led)(struct led_control*);
	void (*show_pattern)(struct led_control*, LED_PATTERN_T color);
};

struct pattern_info {
	u8 address_list[2];
	u8* data;
	int size;
	u8 engine1_start;
	u8 engine2_start;
	u8 engine3_start;
};

/**
 * Registeration for led controller
 */
int lp5523_register_control(struct led_control * cntrl);

/**
 * Download a pattern to LP5523 SRAM
 */
int lp5523_download_animation(struct pattern_info * pattern);

/**
 * Enable pattern - Start animation engines
 */
int lp5523_enable_animation(void);

/**
 * lp5523_pattern_active - Determine if patter is active on all controllers
 */
int lp5523_pattern_active(void);

/**
 * Show LED pattern
 */
int lp5523_show_pattern(LED_PATTERN_T color);

#endif //__LP5523_H__