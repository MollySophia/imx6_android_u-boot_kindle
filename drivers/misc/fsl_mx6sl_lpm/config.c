/*
 * Copyright (C) 2010-2012 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "config.h"

void update_config(config_t * config, unsigned int new_parameter0)
	__attribute__ ((section (".low_power_code")));
void update_config(config_t * config, unsigned int new_parameter0)
{
	int (*update_function) (unsigned int);

	if (config->parameter0 != new_parameter0) {
		if (((unsigned int)config->update_call) != 0) {
			// Call Private Function to Update Configuration
			update_function = config->update_call;
			if (update_function(new_parameter0)) {
				//printf("Configuration Update Failed, Update Function Return Non-Zero\n");
			} else {
				// printf("Update Configuration Function Return Zero, Configuration Updated\n");
				config->parameter0 = new_parameter0;
				//printf("Configuration Updated\n");
			}
		} else {
			config->parameter0 = new_parameter0;
		}
	}
}
