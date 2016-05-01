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

#include "common.h"


#define PSP_ALL_BUTTON_MASK 0xFFFF

#define BUTTON_REPEAT_START    200000
#define BUTTON_REPEAT_CONTINUE 50000

#define PSP_CTRL_ANALOG_UP    (1 << 28)
#define PSP_CTRL_ANALOG_DOWN  (1 << 29)
#define PSP_CTRL_ANALOG_LEFT  (1 << 30)
#define PSP_CTRL_ANALOG_RIGHT (1 << 31)


static int (*__sceCtrlPeekBufferPositive)(SceCtrlData *pad_data, int count);
static int (*__sceCtrlReadBufferPositive)(SceCtrlData *pad_data, int count);

u32 key = 0;

u32 ALIGN_DATA gamepad_config_map[16] =
{
  BUTTON_ID_FASTFORWARD,		//0 BUTTON_ID_MENU
  BUTTON_ID_A,		//1
  BUTTON_ID_B,		//2
  BUTTON_ID_FPS,	//3 BUTTON_ID_START
  BUTTON_ID_L,		//4
  BUTTON_ID_R,		//5
  BUTTON_ID_DOWN,		//6
  BUTTON_ID_LEFT,		//7
  BUTTON_ID_UP,		//8
  BUTTON_ID_RIGHT,	//9
  BUTTON_ID_SELECT,	//10
  BUTTON_ID_START,	//11
  BUTTON_ID_UP,		//12
  BUTTON_ID_DOWN,		//13
  BUTTON_ID_LEFT,		//14
  BUTTON_ID_RIGHT		//15
//  BUTTON_ID_LOADSTATE
//  BUTTON_ID_SAVESTATE
//  BUTTON_ID_FASTFORWARD
//  BUTTON_ID_FPS
};

const u32 ALIGN_DATA button_psp_mask_to_config[] =
{
  PSP_CTRL_TRIANGLE,		//0
  PSP_CTRL_CIRCLE,		//1
  PSP_CTRL_CROSS,			//2
  PSP_CTRL_SQUARE,		//3
  PSP_CTRL_LTRIGGER,		//4
  PSP_CTRL_RTRIGGER,		//5
  PSP_CTRL_DOWN,			//6
  PSP_CTRL_LEFT,			//7
  PSP_CTRL_UP,			//8
  PSP_CTRL_RIGHT,			//9
  PSP_CTRL_SELECT,		//10
  PSP_CTRL_START,			//11
  PSP_CTRL_ANALOG_UP,		//12
  PSP_CTRL_ANALOG_DOWN,	//13
  PSP_CTRL_ANALOG_LEFT,	//14
  PSP_CTRL_ANALOG_RIGHT	//15
};

const u32 ALIGN_DATA button_id_to_gba_mask[] =
{
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_A,
  BUTTON_B,
  BUTTON_L,
  BUTTON_R,
  BUTTON_START,
  BUTTON_SELECT,
  BUTTON_NONE,
  BUTTON_NONE,
  BUTTON_NONE,
  BUTTON_NONE
};

typedef enum
{
  BUTTON_NOT_HELD,
  BUTTON_HELD_INITIAL,
  BUTTON_HELD_REPEAT
} BUTTON_REPEAT_STATE_TYPE;

BUTTON_REPEAT_STATE_TYPE button_repeat_state = BUTTON_NOT_HELD;
u32 button_repeat = 0;
GUI_ACTION_TYPE cursor_repeat = CURSOR_NONE;

#define BUTTON_REPEAT_MASK \
 (PSP_CTRL_UP | PSP_CTRL_RIGHT | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER)

u32 last_buttons = 0;
u32 button_repeat_timestamp;

u32 rapidfire_flag = 1;

u32 enable_tilt_sensor = 0;
u32 tilt_sensorX = 0x800;
u32 tilt_sensorY = 0x800;

static void trigger_key(u32 key);


static void trigger_key(u32 key)
{
  u16 p1_cnt = pIO_REG(REG_P1CNT);

  if (((p1_cnt & 0x4000) != 0) || (reg[CPU_HALT_STATE] == CPU_STOP))
  {
    u16 key_intersection = (p1_cnt & key) & 0x3FF;

    if ((p1_cnt & 0x8000) != 0)
    {
      if (key_intersection == (p1_cnt & 0x3FF))
        pIO_REG(REG_IF) |= IRQ_KEYPAD;
    }
    else
    {
      if (key_intersection != 0)
        pIO_REG(REG_IF) |= IRQ_KEYPAD;
    }
  }
}


u32 get_pad_input(u32 mask)
{
  if (__sceCtrlPeekBufferPositive)
  {
    SceCtrlData ctrl_data;
    (*__sceCtrlPeekBufferPositive)(&ctrl_data, 1);
    return ctrl_data.Buttons & mask;
  }

  return 0;
}


GUI_ACTION_TYPE get_gui_input(void)
{
  SceCtrlData ctrl_data;
  GUI_ACTION_TYPE new_button = CURSOR_NONE;

  u32 new_buttons;
  u32 analog_sensitivity = 20 + (option_analog_sensitivity * 10);
  u32 inv_analog_sensitivity = 255 - analog_sensitivity;

  sceCtrlReadBufferPositive(&ctrl_data, 1);

  if ((option_enable_analog != 0) && ((ctrl_data.Buttons & PSP_CTRL_HOLD) == 0))
  {
    if (ctrl_data.Lx < analog_sensitivity)
      ctrl_data.Buttons |= PSP_CTRL_LEFT;

    if (ctrl_data.Lx > inv_analog_sensitivity)
      ctrl_data.Buttons |= PSP_CTRL_RIGHT;

    if (ctrl_data.Ly < analog_sensitivity)
      ctrl_data.Buttons |= PSP_CTRL_UP;

    if (ctrl_data.Ly > inv_analog_sensitivity)
      ctrl_data.Buttons |= PSP_CTRL_DOWN;
  }

  ctrl_data.Buttons &= PSP_ALL_BUTTON_MASK;

  new_buttons = (last_buttons ^ ctrl_data.Buttons) & ctrl_data.Buttons;
  last_buttons = ctrl_data.Buttons;

  if ((new_buttons & PSP_CTRL_RTRIGGER) != 0)
    new_button = CURSOR_RTRIGGER;

  if ((new_buttons & PSP_CTRL_LTRIGGER) != 0)
    new_button = CURSOR_LTRIGGER;

  if ((new_buttons & PSP_CTRL_LEFT) != 0)
    new_button = CURSOR_LEFT;

  if ((new_buttons & PSP_CTRL_RIGHT) != 0)
    new_button = CURSOR_RIGHT;

  if ((new_buttons & PSP_CTRL_UP) != 0)
    new_button = CURSOR_UP;

  if ((new_buttons & PSP_CTRL_DOWN) != 0)
    new_button = CURSOR_DOWN;

  if ((new_buttons & PSP_CTRL_START) != 0)
    new_button = CURSOR_SELECT;

  if ((new_buttons & PSP_CTRL_SELECT) != 0)
    new_button = CURSOR_DEFAULT;

  if ((new_buttons & PSP_CTRL_CIRCLE) != 0)
    new_button = CURSOR_SELECT;

  if ((new_buttons & PSP_CTRL_CROSS) != 0)
    new_button = CURSOR_EXIT;

  if ((new_buttons & PSP_CTRL_SQUARE) != 0)
    new_button = CURSOR_BACK;

  if (new_button != CURSOR_NONE)
  {
    button_repeat_timestamp = ctrl_data.TimeStamp;
    button_repeat_state = BUTTON_HELD_INITIAL;
    button_repeat = new_buttons & BUTTON_REPEAT_MASK;
    cursor_repeat = new_button;
  }
  else
  {
    if ((ctrl_data.Buttons & button_repeat) != 0)
    {
      u32 new_ticks = ctrl_data.TimeStamp;

      if (button_repeat_state == BUTTON_HELD_INITIAL)
      {
        if ((new_ticks - button_repeat_timestamp) > BUTTON_REPEAT_START)
        {
          new_button = cursor_repeat;
          button_repeat_timestamp = new_ticks;
          button_repeat_state = BUTTON_HELD_REPEAT;
        }
      }

      if (button_repeat_state == BUTTON_HELD_REPEAT)
      {
        if ((new_ticks - button_repeat_timestamp) > BUTTON_REPEAT_CONTINUE)
        {
          new_button = cursor_repeat;
          button_repeat_timestamp = new_ticks;
        }
      }
    }
  }

  return new_button;
}

GUI_ACTION_TYPE get_gui_input_fs_hold(u32 button_id)
{
  GUI_ACTION_TYPE new_button = get_gui_input();

  if ((last_buttons & button_psp_mask_to_config[button_id]) == 0)
    return CURSOR_BACK;

  return new_button;
}

u32 update_input(void)
{
  SceCtrlData ctrl_data = { 0, };
  u32 buttons;
  u32 non_repeat_buttons;
  u32 button_id = 0;
  u32 i;
  u32 new_key = 0;
  u32 analog_sensitivity = 20 + (option_analog_sensitivity * 10);
  u32 inv_analog_sensitivity = 255 - analog_sensitivity;

  tilt_sensorX = 0x800;
  tilt_sensorY = 0x800;

  if (__sceCtrlPeekBufferPositive)
    (*__sceCtrlPeekBufferPositive)(&ctrl_data, 1);

  buttons = ctrl_data.Buttons & 0x0003FFFF;

  if ((option_enable_analog != 0) && ((ctrl_data.Buttons & PSP_CTRL_HOLD) == 0))
  {
    if (enable_tilt_sensor != 0)
    {
      if ((ctrl_data.Lx < analog_sensitivity) || (ctrl_data.Lx > inv_analog_sensitivity))
      {
        tilt_sensorX = (0xFF - ctrl_data.Lx) << 4;
      }

      if ((ctrl_data.Ly < analog_sensitivity) || (ctrl_data.Ly > inv_analog_sensitivity))
      {
        tilt_sensorY = (0xFF - ctrl_data.Ly) << 4;
      }
    }
    else
    {
      if (ctrl_data.Lx < analog_sensitivity)
        buttons |= PSP_CTRL_ANALOG_LEFT;

      if (ctrl_data.Lx > inv_analog_sensitivity)
        buttons |= PSP_CTRL_ANALOG_RIGHT;

      if (ctrl_data.Ly < analog_sensitivity)
        buttons |= PSP_CTRL_ANALOG_UP;

      if (ctrl_data.Ly > inv_analog_sensitivity)
        buttons |= PSP_CTRL_ANALOG_DOWN;
    }
  }

  non_repeat_buttons = (last_buttons ^ buttons) & buttons;
  last_buttons = buttons;

  if ((enable_home_menu != 0) && ((non_repeat_buttons & PSP_CTRL_HOME) != 0))
    return menu();

  for (i = 0; i < 16; i++)
  {
    if ((non_repeat_buttons & button_psp_mask_to_config[i]) != 0)
    {
      switch (gamepad_config_map[i])
      {
        case BUTTON_ID_MENU:
          return menu();

        case BUTTON_ID_LOADSTATE:
          action_loadstate();
          return 1;

        case BUTTON_ID_SAVESTATE:
          action_savestate();
          return 0;

        case BUTTON_ID_FASTFORWARD:
          synchronize_flag ^= 1;
          return 0;

        case BUTTON_ID_FPS:
          psp_fps_debug ^= 1;
          return 0;
      }
    }

    if ((buttons & button_psp_mask_to_config[i]) != 0)
    {
      button_id = gamepad_config_map[i];

      if (button_id < BUTTON_ID_MENU)
      {
        new_key |= button_id_to_gba_mask[button_id];
      }
      else
      {
        if ((button_id >= BUTTON_ID_RAPIDFIRE_A) && (button_id <= BUTTON_ID_RAPIDFIRE_R))
        {
          rapidfire_flag ^= 1;

          if (rapidfire_flag != 0)
          {
            new_key |= button_id_to_gba_mask[button_id - BUTTON_ID_RAPIDFIRE_A + BUTTON_ID_A];
          }
          else
          {
            new_key &= ~button_id_to_gba_mask[button_id - BUTTON_ID_RAPIDFIRE_A + BUTTON_ID_A];
          }
        }
      }
    }
  }

  if ((new_key | key) != key)
    trigger_key(new_key);

  key = new_key;
  pIO_REG(REG_P1) = (~key) & 0x3FF;

  return 0;
}


void init_input(void)
{
  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  __sceCtrlPeekBufferPositive = sceCtrlPeekBufferPositive;
  __sceCtrlReadBufferPositive = sceCtrlReadBufferPositive;
}

void init_input_kernel(void)
{
  kuCtrlSetSamplingCycle(0);
  kuCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  __sceCtrlPeekBufferPositive = kuCtrlPeekBufferPositive;
  __sceCtrlReadBufferPositive = kuCtrlReadBufferPositive;
}


// type = READ / WRITE_MEM
#define INPUT_SAVESTATE_BODY(type)                                            \
{                                                                             \
  FILE_##type##_VARIABLE(savestate_file, key);                                \
}                                                                             \

void input_read_savestate(SceUID savestate_file)
{
  INPUT_SAVESTATE_BODY(READ);
}

void input_write_mem_savestate(SceUID savestate_file)
{
  INPUT_SAVESTATE_BODY(WRITE_MEM);
}

