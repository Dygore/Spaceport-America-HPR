/*
 * Copyright © 2012 Keith Packard <keithp@keithp.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _AO_LCO_H_
#define _AO_LCO_H_

#include <ao_lco_func.h>

#ifndef AO_LCO_DRAG
#define AO_LCO_DRAG	1
#endif

#define DEBUG	1

#if DEBUG
extern uint8_t	ao_lco_debug;
#define PRINTD(...) do { if (!ao_lco_debug) break; printf ("\r%5lu %s: ", (unsigned long) ao_tick_count, __func__); printf(__VA_ARGS__); flush(); } while(0)
#else
#define PRINTD(...) 
#endif

#if AO_LCO_DRAG
extern uint8_t	ao_lco_drag_race;	/* true when drag race mode enabled */
#endif

extern uint8_t	ao_lco_pad;		/* Currently selected pad */
extern uint16_t	ao_lco_box;		/* Currently selected box */

extern uint8_t	ao_lco_armed;		/* armed mode active */
extern uint8_t	ao_lco_firing;		/* fire button pressed */

extern struct ao_pad_query	ao_pad_query;	/* Last received QUERY from pad */

#define AO_LCO_PAD_VOLTAGE	0		/* Pad number to show box voltage */

extern uint16_t	ao_lco_min_box, ao_lco_max_box;

#define AO_LCO_MASK_SIZE(n)	(((n) + 7) >> 3)
#define AO_LCO_MASK_ID(n)	((n) >> 3)
#define AO_LCO_MASK_SHIFT(n)	((n) & 7)

extern uint8_t	ao_lco_box_mask[AO_LCO_MASK_SIZE(AO_PAD_MAX_BOXES)];

/*
 * Shared functions
 */

void
ao_lco_igniter_status(void);

void
ao_lco_update(void);

uint8_t
ao_lco_pad_present(uint16_t box, uint8_t pad);

uint8_t
ao_lco_pad_first(uint16_t box);

void
ao_lco_set_pad(uint8_t new_pad);

void
ao_lco_step_pad(int8_t dir);

void
ao_lco_set_box(uint16_t new_box);

void
ao_lco_set_armed(uint8_t armed);

void
ao_lco_set_firing(uint8_t firing);

void
ao_lco_toggle_drag(void);

void
ao_lco_search(void);

void
ao_lco_monitor(void);

extern uint8_t			ao_lco_drag_beep_count;

/* enable drag race mode */
void
ao_lco_drag_enable(void);

/* disable drag race mode */
void
ao_lco_drag_disable(void);

/* Handle drag beeps, return new delay */
AO_TICK_TYPE
ao_lco_drag_beep_check(AO_TICK_TYPE now, AO_TICK_TYPE delay);

/* Check if it's time to beep during drag race. Return new delay */
AO_TICK_TYPE
ao_lco_drag_warn_check(AO_TICK_TYPE now, AO_TICK_TYPE delay);

/* Request 'beeps' additional drag race beeps */
void
ao_lco_drag_add_beeps(uint8_t beeps);

/* task function for beeping while arm is active */
void
ao_lco_arm_warn(void);

/*
 * Provided by the hw-specific driver code
 */

void
ao_lco_show_pad(uint8_t pad);

void
ao_lco_show_box(uint16_t box);

void
ao_lco_show(void);

void
ao_lco_init(void);

uint8_t
ao_lco_box_present(uint16_t box);

#endif /* _AO_LCO_H_ */
