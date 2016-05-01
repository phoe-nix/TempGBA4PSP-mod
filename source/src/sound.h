/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SOUND_H
#define SOUND_H


#define SOUND_SAMPLES   (800)
#define SOUND_FREQUENCY (48000)

// #define SOUND_SAMPLES   (736)
// #define SOUND_FREQUENCY (44100)

extern u32 sound_pause;

extern u32 gbc_sound_update;
void update_gbc_sound(u32 cpu_ticks);

void sound_timer(FIXED08_24 frequency_step, u8 channel);
void adjust_direct_sound_buffer(u8 channel, u32 cpu_ticks);

void sound_timer_queue(u8 channel);

void gbc_sound_tone_control_low(u8 channel, u32 value);
void gbc_sound_tone_control_high(u8 channel, u32 value);

void gbc_sound_tone_control_sweep(u32 value);

void gbc_sound_wave_control(u32 value);
void gbc_sound_tone_control_low_wave(u32 value);
void gbc_sound_tone_control_high_wave(u32 value);
void gbc_sound_wave_pattern_ram16(u32 address, u32 value);

void gbc_sound_noise_control(u32 value);

void sound_control_low(u32 value);
void sound_control_high(u32 value);
void sound_control_x(u32 value);


void init_sound(void);
void reset_sound(void);
void sound_term(void);

void set_sound_volume(void);
int psp_sound_frequency(u16 sample_count, u16 freq);

void synchronize_sound(void);

void sound_write_mem_savestate(SceUID savestate_file);
void sound_read_savestate(SceUID savestate_file);


#endif /* SOUND_H */
