/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
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

#define GPSP_CONFIG_FILENAME  "tempgba.cfg"
#define GPSP_CONFIG_NUM       (15 + 16) // options + game pad config
#define GPSP_GAME_CONFIG_NUM  (7 + 16)

#define COLOR_BG            COLOR15( 3,  5,  8)
#define COLOR_ROM_INFO      COLOR15(22, 18, 26)
#define COLOR_ACTIVE_ITEM   COLOR15(31, 31, 31)
#define COLOR_INACTIVE_ITEM COLOR15(13, 20, 18)

#define COLOR_HELP_TEXT     COLOR15(16, 20, 24)
#define COLOR_INACTIVE_DIR  COLOR15(13, 22, 22)
#define COLOR_SCROLL_BAR    COLOR15( 7,  9, 11)

#define COLOR_BATT_NORMAL   COLOR_HELP_TEXT
#define COLOR_BATT_LOW      COLOR15_YELLOW
#define COLOR_BATT_CHARG    COLOR15_GREEN

#define FILE_LIST_ROWS      (20)
#define FILE_LIST_POS_X     (18)
#define FILE_LIST_POS_Y     (16)
#define DIR_LIST_POS_X      (360)
#define PAGE_SCROLL_NUM     (10)

#define MENU_LIST_POS_X     (18)

#define SCREEN_IMAGE_POS_X  (228)
#define SCREEN_IMAGE_POS_Y  (44)

#define BATT_STATUS_POS_X   (PSP_SCREEN_WIDTH - (FONTWIDTH * 14))  // 396
#define TIME_STATUS_POS_X   (BATT_STATUS_POS_X - (FONTWIDTH * 22)) // 264
#define DIR_NAME_LENGTH     ((TIME_STATUS_POS_X / FONTWIDTH) - 2)  // 42

// scroll bar
#define SBAR_X1  (2)
#define SBAR_X2  (12)
#define SBAR_Y1  (16)
#define SBAR_Y2  (255)

#define SBAR_T   (SBAR_Y1 + 2)
#define SBAR_B   (SBAR_Y2 - 2)
#define SBAR_H   (SBAR_B - SBAR_T)
#define SBAR_X1I (SBAR_X1 + 2)
#define SBAR_X2I (SBAR_X2 - 2)
#define SBAR_Y1I (SBAR_H * scroll_value[0] / num[0] + SBAR_T)
#define SBAR_Y2I (SBAR_H * (scroll_value[0] + FILE_LIST_ROWS) / num[0] + SBAR_T)


typedef enum
{
  NUMBER_SELECTION_OPTION = 0x01,
  STRING_SELECTION_OPTION = 0x02,
  SUBMENU_OPTION          = 0x04,
  ACTION_OPTION           = 0x08
} MENU_OPTION_TYPE_ENUM;

struct _MenuType
{
  void (*init_function)(void);
  void (*passive_function)(void);
  struct _MenuOptionType *options;
  u32 num_options;
};

struct _MenuOptionType
{
  void (*action_function)(void);
  void (*passive_function)(void);
  struct _MenuType *sub_menu;
  const char *display_string;
  void *options;
  u32 *current_option;
  u32 num_options;
  u32 help_string;
  u32 line_number;
  MENU_OPTION_TYPE_ENUM option_type;
};

typedef struct _MenuOptionType MenuOptionType;
typedef struct _MenuType MenuType;

#define MAKE_MENU(name, init_function, passive_function)                      \
  MenuType name##_menu =                                                      \
  {                                                                           \
    init_function,                                                            \
    passive_function,                                                         \
    name##_options,                                                           \
    sizeof(name##_options) / sizeof(MenuOptionType)                           \
  }                                                                           \

#define GAMEPAD_CONFIG_OPTION(display_string, number)                         \
{                                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  NULL,                                                                       \
  display_string,                                                             \
  gamepad_config_buttons,                                                     \
  gamepad_config_map + gamepad_config_line_to_button[number],                 \
  sizeof(gamepad_config_buttons) / sizeof(gamepad_config_buttons[0]),         \
  MSG_PAD_MENU_HELP_0,                                                        \
  number,                                                                     \
  STRING_SELECTION_OPTION                                                     \
}                                                                             \

#define ANALOG_CONFIG_OPTION(display_string, number)                          \
{                                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  NULL,                                                                       \
  display_string,                                                             \
  gamepad_config_buttons,                                                     \
  gamepad_config_map + number + 12,                                           \
  sizeof(gamepad_config_buttons) / sizeof(gamepad_config_buttons[0]),         \
  MSG_PAD_MENU_HELP_0,                                                        \
  number,                                                                     \
  STRING_SELECTION_OPTION                                                     \
}                                                                             \

#define CHEAT_OPTION(number)                                                  \
{                                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  NULL,                                                                       \
  cheat_format_str[number],                                                   \
  enable_disable_options,                                                     \
  &(cheats[number].cheat_active),                                             \
  2,                                                                          \
  MSG_CHEAT_MENU_HELP_0,                                                      \
  (number) % 10,                                                              \
  STRING_SELECTION_OPTION                                                     \
}                                                                             \

#define SAVESTATE_OPTION(number)                                              \
{                                                                             \
  menu_select_savestate,                                                      \
  NULL,                                                                       \
  NULL,                                                                       \
  savestate_timestamps[number],                                               \
  NULL,                                                                       \
  &savestate_action,                                                          \
  2,                                                                          \
  MSG_STATE_MENU_HELP_0,                                                      \
  number,                                                                     \
  NUMBER_SELECTION_OPTION | ACTION_OPTION                                     \
}                                                                             \

#define ACTION_OPTION(action_function, passive_function, display_string, help_string, line_number) \
{                                                                             \
  action_function,                                                            \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  0,                                                                          \
  help_string,                                                                \
  line_number,                                                                \
  ACTION_OPTION                                                               \
}                                                                             \

#define SUBMENU_OPTION(sub_menu, display_string, help_string, line_number)    \
{                                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  sub_menu,                                                                   \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  sizeof(sub_menu) / sizeof(MenuOptionType),                                  \
  help_string,                                                                \
  line_number,                                                                \
  SUBMENU_OPTION                                                              \
}                                                                             \

#define ACTION_SUBMENU_OPTION(sub_menu, action_function, display_string, help_string, line_number) \
{                                                                             \
  action_function,                                                            \
  NULL,                                                                       \
  sub_menu,                                                                   \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  sizeof(sub_menu) / sizeof(MenuOptionType),                                  \
  help_string,                                                                \
  line_number,                                                                \
  SUBMENU_OPTION | ACTION_OPTION                                              \
}                                                                             \

#define SELECTION_OPTION(passive_function, display_string, options, option_ptr, num_options, help_string, line_number, type) \
{                                                                             \
  NULL,                                                                       \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  type                                                                        \
}                                                                             \

#define ACTION_SELECTION_OPTION(action_function, passive_function, display_string, options, option_ptr, num_options, help_string, line_number, type) \
{                                                                             \
  action_function,                                                            \
  passive_function,                                                           \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  type | ACTION_OPTION                                                        \
}                                                                             \


#define STRING_SELECTION_OPTION(passive_function, display_string, options, option_ptr, num_options, help_string, line_number) \
  SELECTION_OPTION(passive_function, display_string, options, option_ptr, num_options, help_string, line_number, STRING_SELECTION_OPTION)

#define NUMERIC_SELECTION_OPTION(passive_function, display_string, option_ptr, num_options, help_string, line_number) \
  SELECTION_OPTION(passive_function, display_string, NULL, option_ptr, num_options, help_string, line_number, NUMBER_SELECTION_OPTION)

#define STRING_SELECTION_ACTION_OPTION(action_function, passive_function, display_string, options, option_ptr, num_options, help_string, line_number) \
  ACTION_SELECTION_OPTION(action_function, passive_function, display_string,  options, option_ptr, num_options, help_string, line_number, STRING_SELECTION_OPTION)

#define NUMERIC_SELECTION_ACTION_OPTION(action_function, passive_function, display_string, option_ptr, num_options, help_string, line_number) \
  ACTION_SELECTION_OPTION(action_function, passive_function, display_string,  NULL, option_ptr, num_options, help_string, line_number, NUMBER_SELECTION_OPTION)

#define NUMERIC_SELECTION_ACTION_HIDE_OPTION(action_function, passive_function, display_string, option_ptr, num_options, help_string, line_number) \
  ACTION_SELECTION_OPTION(action_function, passive_function, display_string, NULL, option_ptr, num_options, help_string, line_number, NUMBER_SELECTION_OPTION)


char dir_roms[MAX_PATH];
char dir_save[MAX_PATH];
char dir_state[MAX_PATH];
char dir_cfg[MAX_PATH];
char dir_snap[MAX_PATH];
char dir_cheat[MAX_PATH];//cheat

u32 menu_cheat_page = 0;
u32 ALIGN_DATA gamepad_config_line_to_button[] =
{
 8,
 6,
 7,
 9,
 1,
 2,
 3,
 0,
 4,
 5,
 11,
 10,
};

u32 savestate_slot = 0;


void _flush_cache(void);

static int sort_function(const void *dest_str_ptr, const void *src_str_ptr);

static s32 save_game_config_file(void);

static void update_status_string(char *time_str, char *batt_str, u16 *color_batt);
static void update_status_string_gbk(char *time_str, char *batt_str, u16 *color_batt);
static void get_timestamp_string(char *buffer, u16 msg_id, pspTime *msg_time, int day_of_week);

static void get_savestate_info(char *filename, u16 *snapshot, char *timestamp);
static void get_savestate_filename(u32 slot, char *name_buffer);

static void get_snapshot_filename(char *name, const char *ext);
static void save_bmp(const char *path, u16 *screen_image);


void _flush_cache(void)
{
  invalidate_all_cache();
}


static int sort_function(const void *dest_str_ptr, const void *src_str_ptr)
{
  char *dest_str = *((char **)dest_str_ptr);
  char *src_str  = *((char **)src_str_ptr);

  if (src_str[0] == '.')
    return 1;

  if (dest_str[0] == '.')
    return -1;

  if ((isalpha((int)src_str[0]) != 0) || (isalpha((int)dest_str[0]) != 0))
    return strcasecmp(dest_str, src_str);

  return strcmp(dest_str, src_str);
}

s32 load_file(const char **wildcards, char *result, char *default_dir_name)
{
  char current_dir_name[MAX_PATH];
  char current_dir_short[81];
  u32  current_dir_length;

  SceUID current_dir;
  SceIoDirent current_file;

  u32 total_file_names_allocated;
  u32 total_dir_names_allocated;
  char **file_list;
  char **dir_list;
  char *ext_pos;

  #define FILE_LIST (0)
  #define DIR_LIST  (1)

  u32 column;
  u32 num[2];
  u32 selection[2];
  u32 scroll_value[2];
  u32 in_scroll[2];

  u32 current_file_number, current_dir_number;
  u16 current_line_color;

  u32 i;
  s32 return_value = 1;
  s32 repeat;

  GUI_ACTION_TYPE gui_action;

  char time_str[40];
  char batt_str[40];
  u16 color_batt_life = COLOR_BATT_NORMAL;
  u32 counter = 0;

  auto void filelist_term(void);
  auto void malloc_error(void);


  void filelist_term(void)
  {
    for (i = 0; i < num[FILE_LIST]; i++)
    {
      free(file_list[i]);
    }

    free(file_list);

    for (i = 0; i < num[DIR_LIST]; i++)
    {
      free(dir_list[i]);
    }

    free(dir_list);
  }

  void malloc_error(void)
  {
    clear_screen(COLOR32_BLACK);
    error_msg(MSG[MSG_ERR_MALLOC], CONFIRMATION_QUIT);
    quit();
  }

  #define CHECK_MEM_ALLOCATE(mem_block)                                       \
  {                                                                           \
    if (mem_block == NULL)                                                    \
      malloc_error();                                                         \
  }                                                                           \


  if (default_dir_name != NULL)
  {
    chdir(default_dir_name);
  }

  while (return_value == 1)
  {
    column = FILE_LIST;

    selection[FILE_LIST]    = 0;
    scroll_value[FILE_LIST] = 0;
    in_scroll[FILE_LIST]    = 0;

    selection[DIR_LIST]     = 0;
    scroll_value[DIR_LIST]  = 0;
    in_scroll[DIR_LIST]     = 0;

    memset(&current_file, 0, sizeof(current_file));

    total_file_names_allocated = 32;
    total_dir_names_allocated  = 32;

    file_list = (char **)safe_malloc(sizeof(char *) * total_file_names_allocated);
    dir_list  = (char **)safe_malloc(sizeof(char *) * total_dir_names_allocated);

    memset(file_list, 0, sizeof(char *) * total_file_names_allocated);
    memset(dir_list,  0, sizeof(char *) * total_dir_names_allocated);

    num[FILE_LIST] = 0;
    num[DIR_LIST]  = 0;

    getcwd(current_dir_name, MAX_PATH);
    strcat(current_dir_name, "/");

    if (strlen(strstr(current_dir_name, ":/")) != 2)
    {
      dir_list[num[DIR_LIST]] = (char *)safe_malloc(strlen("..") + 1);

      sprintf(dir_list[num[DIR_LIST]], "%s", "..");
      num[DIR_LIST]++;
    }

    scePowerLock(0);
    current_dir = sceIoDopen(current_dir_name);

    while (sceIoDread(current_dir, &current_file) > 0)
    {
      if (current_file.d_name[0] == '.')
        continue;

      if (FIO_S_ISDIR(current_file.d_stat.st_mode) != 0)
      {
        dir_list[num[DIR_LIST]] = (char *)safe_malloc(strlen(current_file.d_name) + 1);

        sprintf(dir_list[num[DIR_LIST]], "%s", current_file.d_name);
        num[DIR_LIST]++;
      }
      else
      {
        if ((ext_pos = strrchr(current_file.d_name, '.')) != NULL)
        {
          for (i = 0; wildcards[i] != NULL; i++)
          {
            if (strcasecmp(ext_pos, wildcards[i]) == 0)
            {
              file_list[num[FILE_LIST]] = (char *)safe_malloc(strlen(current_file.d_name) + 1);

              sprintf(file_list[num[FILE_LIST]], "%s", current_file.d_name);
              num[FILE_LIST]++;
              break;
            }
          }
        }
      }

      if (num[FILE_LIST] == total_file_names_allocated)
      {
        file_list = (char **)realloc(file_list, sizeof(char *) * (total_file_names_allocated << 1));
        CHECK_MEM_ALLOCATE(file_list);
        memset(file_list + total_file_names_allocated, 0, sizeof(char *) * total_file_names_allocated);

        total_file_names_allocated <<= 1;
      }

      if (num[DIR_LIST] == total_dir_names_allocated)
      {
        dir_list = (char **)realloc(dir_list, sizeof(char *) * (total_dir_names_allocated << 1));
        CHECK_MEM_ALLOCATE(dir_list);
        memset(dir_list + total_dir_names_allocated, 0, sizeof(char *) * total_dir_names_allocated);

        total_dir_names_allocated <<= 1;
      }

    } /* end of while */

    sceIoDclose(current_dir);
    scePowerUnlock(0);

    qsort((void *)file_list, num[FILE_LIST], sizeof(char *), sort_function);
    qsort((void *)dir_list,  num[DIR_LIST],  sizeof(char *), sort_function);


    current_dir_length = strlen(current_dir_name);

    if (current_dir_length > DIR_NAME_LENGTH)
    {
      memcpy(current_dir_short, "...", 3);
      memcpy(current_dir_short + 3, current_dir_name + (current_dir_length - (DIR_NAME_LENGTH - 3)), DIR_NAME_LENGTH - 3);
      current_dir_short[DIR_NAME_LENGTH] = 0;
    }
    else
    {
      memcpy(current_dir_short, current_dir_name, current_dir_length + 1);
    }


    repeat = 1;

    if (num[FILE_LIST] == 0)
    {
      column = DIR_LIST;
    }

    while (repeat)
    {
      clear_screen(COLOR15_TO_32(COLOR_BG));
		if (option_language == 0)
			print_string(current_dir_short, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);
		else
			print_string_gbk(current_dir_short, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);
      if ((counter % 30) == 0)
	  {
	    if (option_language == 0)
        update_status_string(time_str, batt_str, &color_batt_life);
		else
        update_status_string_gbk(time_str, batt_str, &color_batt_life);
	  }
      counter++;
	  if (option_language == 0)
	  print_string(time_str, TIME_STATUS_POS_X, 2, COLOR_HELP_TEXT, BG_NO_FILL);
	  else
	  print_string_gbk(time_str, TIME_STATUS_POS_X, 2, COLOR_HELP_TEXT, BG_NO_FILL);
 
      if (option_language == 0)
		print_string(batt_str, BATT_STATUS_POS_X, 2, color_batt_life, BG_NO_FILL);
	  else
		print_string_gbk(batt_str, BATT_STATUS_POS_X, 2, color_batt_life, BG_NO_FILL);

		if (option_language == 0)
			print_string(MSG[MSG_BROWSER_HELP], 30, 258, COLOR_HELP_TEXT, BG_NO_FILL);
		else
			print_string_gbk(MSG[MSG_BROWSER_HELP], 30, 258, COLOR_HELP_TEXT, BG_NO_FILL);

      char str_buffer_size[32];
      sprintf(str_buffer_size, MSG[MSG_BUFFER], gamepak_ram_buffer_size >> 20);
		if (option_language == 0)
			print_string(str_buffer_size, 384, 258, COLOR_HELP_TEXT, BG_NO_FILL);
		else
			print_string_gbk(str_buffer_size, 384, 258, COLOR_HELP_TEXT, BG_NO_FILL);

      // PSP controller - hold
      if (get_pad_input(PSP_CTRL_HOLD) != 0)
		{
		if (option_language == 0)
			print_string(FONT_KEY_ICON_GBK, 6, 258, COLOR15_YELLOW, BG_NO_FILL);
		else
			print_string_gbk(FONT_KEY_ICON, 6, 258, COLOR15_YELLOW, BG_NO_FILL);
		}
      // draw scroll bar
      if (num[FILE_LIST] > FILE_LIST_ROWS)
      {
        draw_box_line(SBAR_X1,  SBAR_Y1,  SBAR_X2,  SBAR_Y2,  COLOR_SCROLL_BAR);
        draw_box_fill(SBAR_X1I, SBAR_Y1I, SBAR_X2I, SBAR_Y2I, COLOR_SCROLL_BAR);
      }

      for (i = 0; i < FILE_LIST_ROWS; i++)
      {
        current_file_number = i + scroll_value[FILE_LIST];

        if (current_file_number < num[FILE_LIST])
        {
          if ((current_file_number == selection[FILE_LIST]) && (column == FILE_LIST))
            current_line_color = COLOR_ACTIVE_ITEM;
          else
            current_line_color = COLOR_INACTIVE_ITEM;

          print_string(file_list[current_file_number], FILE_LIST_POS_X, FILE_LIST_POS_Y + (i * FONTHEIGHT), current_line_color, BG_NO_FILL);
        }
      }

      for (i = 0; i < FILE_LIST_ROWS; i++)
      {
        current_dir_number = i + scroll_value[DIR_LIST];

        if (current_dir_number < num[DIR_LIST])
        {
          if ((current_dir_number == selection[DIR_LIST]) && (column == DIR_LIST))
            current_line_color = COLOR_ACTIVE_ITEM;
          else
            current_line_color = COLOR_INACTIVE_DIR;

          print_string(dir_list[current_dir_number], DIR_LIST_POS_X, FILE_LIST_POS_Y + (i * FONTHEIGHT), current_line_color, COLOR_BG);
        }
      }

      if (num[DIR_LIST] > FILE_LIST_ROWS)
      {
        if (scroll_value[DIR_LIST] != 0)
          print_string(FONT_CURSOR_UP_FILL, PSP_SCREEN_WIDTH - (FONTWIDTH * 2), FILE_LIST_POS_Y, COLOR_SCROLL_BAR, COLOR_BG);

        if (num[DIR_LIST] > (scroll_value[DIR_LIST] + FILE_LIST_ROWS))
          print_string(FONT_CURSOR_DOWN_FILL, PSP_SCREEN_WIDTH - (FONTWIDTH * 2), FILE_LIST_POS_Y + ((FILE_LIST_ROWS - 1) * FONTHEIGHT), COLOR_SCROLL_BAR, COLOR_BG);
      }

      __draw_volume_status(1);
      flip_screen(1);


      gui_action = get_gui_input();

      switch (gui_action)
      {
        case CURSOR_DOWN:
          if (selection[column] < (num[column] - 1))
          {
            selection[column]++;

            if (in_scroll[column] == (FILE_LIST_ROWS - 1))
              scroll_value[column]++;
            else
              in_scroll[column]++;
          }
          break;

        case CURSOR_RTRIGGER:
          if (num[column] > PAGE_SCROLL_NUM)
          {
            if (selection[column] < (num[column] - PAGE_SCROLL_NUM))
            {
              selection[column] += PAGE_SCROLL_NUM;

              if (in_scroll[column] >= (FILE_LIST_ROWS - PAGE_SCROLL_NUM))
              {
                scroll_value[column] += PAGE_SCROLL_NUM;

                if (scroll_value[column] > (num[column] - FILE_LIST_ROWS))
                {
                  scroll_value[column] = num[column] - FILE_LIST_ROWS;
                  in_scroll[column] = selection[column] - scroll_value[column];
                }
              }
              else
              {
                in_scroll[column] += PAGE_SCROLL_NUM;
              }
            }
            else
            {
              selection[column] = num[column] - 1;
              in_scroll[column] += PAGE_SCROLL_NUM;

              if (in_scroll[column] >= (FILE_LIST_ROWS - 1))
              {
                if (num[column] > (FILE_LIST_ROWS - 1))
                {
                  in_scroll[column] = FILE_LIST_ROWS - 1;
                  scroll_value[column] = num[column] - FILE_LIST_ROWS;
                }
                else
                {
                  in_scroll[column] = num[column] - 1;
                }
              }
            }
          }
          else
          {
            selection[column] = num[column] - 1;
            in_scroll[column] = num[column] - 1;
          }
          break;

        case CURSOR_UP:
          if (selection[column] != 0)
          {
            selection[column]--;

            if (in_scroll[column] == 0)
              scroll_value[column]--;
            else
              in_scroll[column]--;
          }
          break;

        case CURSOR_LTRIGGER:
          if (selection[column] >= PAGE_SCROLL_NUM)
          {
            selection[column] -= PAGE_SCROLL_NUM;

            if (in_scroll[column] < PAGE_SCROLL_NUM)
            {
              if (scroll_value[column] >= PAGE_SCROLL_NUM)
              {
                scroll_value[column] -= PAGE_SCROLL_NUM;
              }
              else
              {
                scroll_value[column] = 0;
                in_scroll[column] = selection[column];
              }
            }
            else
            {
              in_scroll[column] -= PAGE_SCROLL_NUM;
            }
          }
          else
          {
            selection[column] = 0;
            in_scroll[column] = 0;
            scroll_value[column] = 0;
          }
          break;

        case CURSOR_RIGHT:
          if (column == FILE_LIST)
          {
            if (num[DIR_LIST] != 0)
              column = DIR_LIST;
          }
          break;

        case CURSOR_LEFT:
          if (column == DIR_LIST)
          {
            if (num[FILE_LIST] != 0)
              column = FILE_LIST;
          }
          break;

        case CURSOR_SELECT:
          if (column == DIR_LIST)
          {
            repeat = 0;
            chdir(dir_list[selection[DIR_LIST]]);
          }
          else
          {
            if (num[FILE_LIST] != 0)
            {
              repeat = 0;
              return_value = 0;
              strcpy(result, file_list[selection[FILE_LIST]]);
            }
          }
          break;

        case CURSOR_BACK:
          // ROOT
          if (strlen(strstr(current_dir_name, ":/")) == 2)
            break;

          repeat = 0;
          chdir("..");
          break;

        case CURSOR_EXIT:
          return_value = -1;
          repeat = 0;
          break;

        case CURSOR_DEFAULT:
          break;

        case CURSOR_NONE:
          break;
      }

    } /* end while (repeat) */

    filelist_term();

  } /* end while (return_value == 1) */

  return return_value;
}


static void get_savestate_info(char *filename, u16 *snapshot, char *timestamp)
{
  SceUID savestate_file;
  char savestate_path[MAX_PATH];

  sprintf(savestate_path, "%s%s", dir_state, filename);

  scePowerLock(0);

  FILE_OPEN(savestate_file, savestate_path, READ);

  if (FILE_CHECK_VALID(savestate_file))
  {
    u64 savestate_tick_utc;
    u64 savestate_tick_local;

    pspTime savestate_time = { 0 };

    if (snapshot != NULL)
      FILE_READ(savestate_file, snapshot, GBA_SCREEN_SIZE);
    else
      FILE_SEEK(savestate_file, GBA_SCREEN_SIZE, SEEK_SET);

    FILE_READ_VARIABLE(savestate_file, savestate_tick_utc);

    FILE_CLOSE(savestate_file);

    sceRtcConvertUtcToLocalTime(&savestate_tick_utc, &savestate_tick_local);
    sceRtcSetTick(&savestate_time, &savestate_tick_local);
    int day_of_week = sceRtcGetDayOfWeek(savestate_time.year, savestate_time.month, savestate_time.day);

    get_timestamp_string(timestamp, MSG_STATE_MENU_DATE_FMT_0, &savestate_time, day_of_week);
  }
  else
  {
    if (snapshot != NULL)
    {
      memset(snapshot, 0, GBA_SCREEN_SIZE);
		if (option_language == 0)
			print_string_ext(MSG[MSG_STATE_MENU_STATE_NONE], X_POS_CENTER, 74, COLOR15_WHITE, BG_NO_FILL, snapshot, GBA_SCREEN_WIDTH);
		else
			print_string_ext_gbk(MSG[MSG_STATE_MENU_STATE_NONE], X_POS_CENTER, 74, COLOR15_WHITE, BG_NO_FILL, snapshot, GBA_SCREEN_WIDTH);
    }

    sprintf(timestamp, "%s", MSG[(date_format == 0) ? MSG_STATE_MENU_DATE_NONE_0 : MSG_STATE_MENU_DATE_NONE_1]);
  }

  scePowerUnlock(0);
}

static void get_savestate_filename(u32 slot, char *name_buffer)
{
  char savestate_ext[16];

  sprintf(savestate_ext, "_%d.svs", (int)slot);
  change_ext(gamepak_filename, name_buffer, savestate_ext);
}


void action_loadstate(void)
{
  char savestate_filename[MAX_FILE];

  get_savestate_filename(savestate_slot, savestate_filename);
  load_state(savestate_filename);
}

void action_savestate(void)
{
  char savestate_filename[MAX_FILE];
  u16 *current_screen;

  current_screen = copy_screen();

  get_savestate_filename(savestate_slot, savestate_filename);
  save_state(savestate_filename, current_screen);

  free(current_screen);
}


u32 menu(void)
{
	int id_language;
  u32 i;

  u32 repeat = 1;
  u32 return_value = 0;

  u32 first_load = 0;

  GUI_ACTION_TYPE gui_action;
  SceCtrlData ctrl_data;

  char game_title[MAX_FILE];
  //char backup_id[16];
  u16 *screen_image_ptr = NULL;
  u16 *current_screen   = NULL;
  u16 *savestate_screen = NULL;

  u32 savestate_action = 0;
  char savestate_timestamps[10][40];

  char time_str[40];
  char batt_str[40];
  u16 color_batt_life = COLOR_BATT_NORMAL;
  u32 counter = 0;

  char filename_buffer[MAX_PATH];

  char line_buffer[80];

  char cheat_format_str[MAX_CHEATS][25*4];// gpsp kai 41*4

  MenuType *current_menu;
  MenuOptionType *current_option;
  MenuOptionType *display_option;

  u32 menu_init_flag = 0;

  u32 current_option_num = 0;
  u32 menu_main_option_num = 0;

  const char *yes_no_options[] =
  {
    MSG[MSG_NO],
    MSG[MSG_YES]
  };

  const char *enable_disable_options[] =
  {
    MSG[MSG_DISABLED],
    MSG[MSG_ENABLED]
  };

  const char *on_off_options[] =
  {
    MSG[MSG_OFF],
    MSG[MSG_ON]
  };

  const char *scale_options[] =
  {
    MSG[MSG_SCN_SCALED_NONE],
    MSG[MSG_SCN_SCALED_X15_GU],
    MSG[MSG_SCN_SCALED_X15_SW],
    MSG[MSG_SCN_SCALED_USER]
  };

  const char *frameskip_options[] =
  {
    MSG[MSG_AUTO],
    MSG[MSG_MANUAL],
    MSG[MSG_OFF]
  };

  const char *stack_optimize_options[] =
  {
    MSG[MSG_OFF],
    MSG[MSG_AUTO]
  };

  const char *update_backup_options[] =
  {
    MSG[MSG_EXITONLY],
    MSG[MSG_AUTO]
  };

  const char *language_option[] =
  {
    MSG[MSG_LANG_JAPANESE],
    MSG[MSG_LANG_ENGLISH],
    MSG[MSG_LANG_CHS],
    MSG[MSG_LANG_CHT]
  };

  const char *sound_volume_options[] =
  {
    "0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"
  };

  const char *clock_speed_options[] =
  {
    "222MHz", "266MHz", "300MHz", "333MHz"
  };

  const char *image_format_options[] =
  {
    "PNG", "BMP"
  };

  const char *gamepad_config_buttons[] =
  {
    MSG[MSG_PAD_MENU_CFG_0],
    MSG[MSG_PAD_MENU_CFG_1],
    MSG[MSG_PAD_MENU_CFG_2],
    MSG[MSG_PAD_MENU_CFG_3],
    MSG[MSG_PAD_MENU_CFG_4],
    MSG[MSG_PAD_MENU_CFG_5],
    MSG[MSG_PAD_MENU_CFG_6],
    MSG[MSG_PAD_MENU_CFG_7],
    MSG[MSG_PAD_MENU_CFG_8],
    MSG[MSG_PAD_MENU_CFG_9],
    MSG[MSG_PAD_MENU_CFG_10],
    MSG[MSG_PAD_MENU_CFG_11],
    MSG[MSG_PAD_MENU_CFG_12],
    MSG[MSG_PAD_MENU_CFG_13],
    MSG[MSG_PAD_MENU_CFG_14],
    MSG[MSG_PAD_MENU_CFG_15],
    MSG[MSG_PAD_MENU_CFG_16],
    MSG[MSG_PAD_MENU_CFG_17],
    MSG[MSG_PAD_MENU_CFG_18],
    MSG[MSG_PAD_MENU_CFG_19]
  };

  auto void choose_menu(MenuType *new_menu);

  auto void menu_init(void);
  auto void menu_term(void);
  auto void menu_exit(void);
  auto void menu_quit(void);
  auto void menu_reset(void);
  auto void menu_suspend(void);

  auto void menu_screen_capture(void);

  auto void menu_change_state(void);
  auto void menu_select_savestate(void);
  auto void menu_save_state(void);
  auto void menu_load_state(void);
  auto void menu_load_state_file(void);

  auto void menu_default(void);
  auto void menu_load_cheat_file(void);
  auto void submenu_cheats_misc(void);

  auto void menu_load_file(void);

  auto void submenu_emulator(void);
  auto void submenu_gamepad(void);
  auto void submenu_analog(void);
  auto void submenu_savestate(void);
  auto void submenu_main(void);
  auto void reload_cheats_page(void);

  auto void draw_analog_pad_range(void);
  auto void load_savestate_timestamps(void);

  auto void gamepak_file_none(void);
  auto void gamepak_file_reopen(void);

  void menu_init(void)
  {
    menu_init_flag = 1;
  }

  void menu_term(void)
  {
    screen_image_ptr = NULL;

    if (savestate_screen != NULL)
    {
      free(savestate_screen);
      savestate_screen = NULL;
    }

    if (current_screen != NULL)
    {
      free(current_screen);
      current_screen = NULL;
    }
  }

  void menu_exit(void)
  {
    if (!first_load)
      repeat = 0;
  }

  void menu_quit(void)
  {
    menu_term();
    quit();
  }

  void menu_suspend(void)
  {
    save_game_config_file();

    if (!first_load)
      update_backup_immediately();

    scePowerTick(0);
    scePowerRequestSuspend();
  }

  void menu_load_file(void)
  {
    const char *file_ext[] = { ".zip", ".gba", ".bin", ".agb", ".gbz", NULL };

    save_game_config_file();

    if (!first_load)
      update_backup_immediately();

    if (load_file(file_ext, filename_buffer, dir_roms) == 0)
    {
      if (load_gamepak(filename_buffer) < 0)
      {
        clear_screen(COLOR32_BLACK);
        error_msg(MSG[MSG_ERR_LOAD_GAMEPACK], CONFIRMATION_CONT);

        gamepak_file_none();

        menu_init();
        choose_menu(current_menu);
        counter = 0;

        return;
      }

      reset_gba();
      reg[CHANGED_PC_STATUS] = 1;

      return_value = 1;
      repeat = 0;
    }
    else
    {
      menu_init();
      choose_menu(current_menu);
      counter = 0;
    }
  }

  void menu_reset(void)
  {
    if (!first_load)
    {
      reset_gba();
      reg[CHANGED_PC_STATUS] = 1;

      return_value = 1;
      repeat = 0;
    }
  }

  void menu_screen_capture(void)
  {
    if (!first_load)
    {
      scePowerLock(0);
      set_cpu_clock(PSP_CLOCK_333);

      if (option_screen_capture_format != 0)
      {
        get_snapshot_filename(filename_buffer, "bmp");
        save_bmp(filename_buffer, current_screen);
      }
      else
      {
        get_snapshot_filename(filename_buffer, "png");
        save_png(filename_buffer, current_screen);
      }

      set_cpu_clock(PSP_CLOCK_222);
      scePowerUnlock(0);
    }
  }

  void menu_change_state(void)
  {
    get_savestate_filename(savestate_slot, filename_buffer);
    get_savestate_info(filename_buffer, savestate_screen, line_buffer);
    sprintf(savestate_timestamps[savestate_slot], "%d: %s", (int)savestate_slot, line_buffer);

    screen_image_ptr = savestate_screen;
  }

  void menu_save_state(void)
  {
    if (!first_load)
    {
      get_savestate_filename(savestate_slot, filename_buffer);
      save_state(filename_buffer, current_screen);

      get_savestate_info(filename_buffer, savestate_screen, line_buffer);
      sprintf(savestate_timestamps[savestate_slot], "%d: %s", (int)savestate_slot, line_buffer);
    }
  }

  void menu_load_state(void)
  {
    if (!first_load)
    {
      get_savestate_filename(savestate_slot, filename_buffer);
      load_state(filename_buffer);

      return_value = 1;
      repeat = 0;
    }
  }

  void menu_select_savestate(void)
  {
    if (savestate_action != 0)
      menu_save_state();
    else
      menu_load_state();
  }

  void menu_load_state_file(void)
  {
    const char *file_ext[] = { ".svs", NULL };

    if ((load_file(file_ext, filename_buffer, dir_state) == 0) && !first_load)
    {
      load_state(filename_buffer);
      return_value = 1;
      repeat = 0;
    }
    else
    {
      menu_init();
      choose_menu(current_menu);
      counter = 0;
    }
  }

  void menu_default(void)
  {
	option_screen_scale = SCALED_X15_GU;
	option_screen_mag = 170;
	option_screen_filter = FILTER_BILINEAR;
	psp_fps_debug = 0;
	option_frameskip_type = FRAMESKIP_AUTO;
	option_frameskip_value = 9;
	option_clock_speed = PSP_CLOCK_333;
	option_sound_volume = 10;
	option_stack_optimize = 1;
	option_boot_mode = 0;
	option_update_backup = 1;		//auto
	option_screen_capture_format = 0;
	option_enable_analog = 0;
	option_analog_sensitivity = 4;
	//int id_language;
	sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id_language);
	if (id_language == PSP_SYSTEMPARAM_LANGUAGE_JAPANESE)
		option_language = 0;
	else if (id_language == PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED)
		option_language = 2;
	else if (id_language == PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL)
		option_language = 3;
	else
		option_language = 1;

  }

  void menu_load_cheat_file(void)
  {
    const char *file_ext[] = { ".cht", NULL };
    char load_filename[MAX_FILE];

    if(load_file(file_ext, load_filename, dir_cheat) != -1)
    {

	  u32 i,j;
      for(j = 0; j < MAX_CHEATS; j++)
      {
        cheats[j].cheat_active = 0;
        cheats[j].cheat_name[0] = '\0';
      }

      add_cheats(load_filename);
      for(i = 0; i < MAX_CHEATS; i++)
      {

        if(i >= num_cheats)
        {
          sprintf(cheat_format_str[i], MSG[MSG_CHEAT_MENU_NON_LOAD], i);
        }
        else
        {
          sprintf(cheat_format_str[i], MSG[MSG_CHEAT_MENU_0], i, cheats[i].cheat_name);
        }
      }
	  //menu_cheat_page = 0;
      choose_menu(current_menu);
    }
    else
    {
      choose_menu(current_menu);
    }
  }

  #define DRAW_TITLE(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_GBA_ICON, MSG[title]);                  \
   print_string(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  #define DRAW_TITLE_PSP(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_PSP_ICON, MSG[title]);                  \
   print_string(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  #define DRAW_TITLE_SAVESTATE(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_MSC_ICON, MSG[title]);                  \
   print_string(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \


  #define DRAW_TITLE_GBK(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_GBA_ICON_GBK, MSG[title]);                  \
   print_string_gbk(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  #define DRAW_TITLE_PSP_GBK(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_PSP_ICON_GBK, MSG[title]);                  \
   print_string_gbk(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  #define DRAW_TITLE_SAVESTATE_GBK(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_MSC_ICON_GBK, MSG[title]);                  \
   print_string_gbk(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  #define DRAW_TITLE_OPT_GBK(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_OPT_ICON_GBK, MSG[title]);                  \
   print_string_gbk(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  #define DRAW_TITLE_PAD_GBK(title)                                                   \
   sprintf(line_buffer, "%s %s", FONT_PAD_ICON_GBK, MSG[title]);                  \
   print_string_gbk(line_buffer, 6, 2, COLOR_HELP_TEXT, BG_NO_FILL);              \

  void submenu_emulator(void)
  {
    DRAW_TITLE_OPT_GBK(MSG_OPTION_MENU_TITLE);
  }


  void submenu_cheats_misc(void)
  {
    DRAW_TITLE_PSP_GBK(MSG_CHEAT_MENU_TITLE);
  }

  void submenu_gamepad(void)
  {
    DRAW_TITLE_PAD_GBK(MSG_PAD_MENU_TITLE);
  }

  void submenu_analog(void)
  {
    DRAW_TITLE_PAD_GBK(MSG_A_PAD_MENU_TITLE);

    draw_analog_pad_range();
  }

  void submenu_savestate(void)
  {
    DRAW_TITLE_SAVESTATE_GBK(MSG_STATE_MENU_TITLE);

    if (menu_init_flag != 0)
    {
      savestate_action = 0;
      menu_change_state();

      current_option_num = savestate_slot;
      current_option = current_menu->options + current_option_num;

      menu_init_flag = 0;
    }

    if (current_option_num < 10)
    {
      if (savestate_slot != current_option_num)
      {
        savestate_slot = current_option_num;
        menu_change_state();
      }
	if (option_language == 0)
      print_string(MSG[savestate_action ? MSG_SAVE : MSG_LOAD], MENU_LIST_POS_X + ((strlen(savestate_timestamps[0]) + 1) * FONTWIDTH), (current_option_num * FONTHEIGHT) + 28, COLOR_ACTIVE_ITEM, BG_NO_FILL);
	else
      print_string_gbk(MSG[savestate_action ? MSG_SAVE : MSG_LOAD], MENU_LIST_POS_X + ((strlen(savestate_timestamps[0]) + 1) * FONTWIDTH), (current_option_num * FONTHEIGHT) + 28, COLOR_ACTIVE_ITEM, BG_NO_FILL);
    }
  }

  void submenu_main(void)
  {
    DRAW_TITLE_GBK(MSG_MAIN_MENU_TITLE);

    if (menu_init_flag != 0)
    {
      screen_image_ptr = current_screen;

      current_option_num = menu_main_option_num;
      current_option = current_menu->options + current_option_num;

      menu_init_flag = 0;
    }

    if (menu_main_option_num != current_option_num)
      menu_main_option_num = current_option_num;
  }

  void draw_analog_pad_range(void)
  {
    char print_buffer[40];
    u32 lx, ly;
    u32 analog_sensitivity, inv_analog_sensitivity;

    #define PAD_RANGE (255 >> 1)
    #define BASE_X (SCREEN_IMAGE_POS_X + ((GBA_SCREEN_WIDTH  - PAD_RANGE) >> 1))
    #define BASE_Y (SCREEN_IMAGE_POS_Y + ((GBA_SCREEN_HEIGHT - PAD_RANGE) >> 1))

    sceCtrlPeekBufferPositive(&ctrl_data, 1);
    lx = ctrl_data.Lx;
    ly = ctrl_data.Ly;

    analog_sensitivity = 20 + (option_analog_sensitivity * 10);
    inv_analog_sensitivity = 255 - analog_sensitivity;

    draw_box_alpha(SCREEN_IMAGE_POS_X, SCREEN_IMAGE_POS_Y, SCREEN_IMAGE_POS_X + GBA_SCREEN_WIDTH - 1, SCREEN_IMAGE_POS_Y + GBA_SCREEN_HEIGHT - 1, 0xBF000000);

    sprintf(print_buffer, "Lx:%3d Ly:%3d", (int)lx, (int)ly);
    print_string(print_buffer, SCREEN_IMAGE_POS_X + 6, SCREEN_IMAGE_POS_Y + 2, COLOR15_WHITE, BG_NO_FILL);

    if (lx < analog_sensitivity)
      print_string(FONT_CURSOR_LEFT, BASE_X - (FONTWIDTH << 1), BASE_Y + ((PAD_RANGE - FONTHEIGHT) >> 1), COLOR15_WHITE, BG_NO_FILL);

    if (lx > inv_analog_sensitivity)
      print_string(FONT_CURSOR_RIGHT, BASE_X + PAD_RANGE, BASE_Y + ((PAD_RANGE - FONTHEIGHT) >> 1), COLOR15_WHITE, BG_NO_FILL);

    if (ly < analog_sensitivity)
      print_string(FONT_CURSOR_UP, BASE_X + (PAD_RANGE >> 1) - FONTWIDTH, BASE_Y - FONTHEIGHT, COLOR15_WHITE, BG_NO_FILL);

    if (ly > inv_analog_sensitivity)
      print_string(FONT_CURSOR_DOWN, BASE_X + (PAD_RANGE >> 1) - FONTWIDTH, BASE_Y + PAD_RANGE, COLOR15_WHITE, BG_NO_FILL);

    lx >>= 1;
    ly >>= 1;
    analog_sensitivity >>= 1;
    inv_analog_sensitivity >>= 1;

    draw_box_line(BASE_X, BASE_Y, BASE_X + PAD_RANGE, BASE_Y + PAD_RANGE, COLOR15_WHITE);

    // dead zone
    draw_box_alpha(BASE_X + analog_sensitivity, BASE_Y + analog_sensitivity, BASE_X + inv_analog_sensitivity, BASE_Y + inv_analog_sensitivity, 0x5F000000 | 255);
    draw_box_line(BASE_X + analog_sensitivity, BASE_Y + analog_sensitivity, BASE_X + inv_analog_sensitivity, BASE_Y + inv_analog_sensitivity, COLOR15_RED);

    // pointer
    draw_box_line(BASE_X + lx - 2, BASE_Y + ly - 2, BASE_X + lx + 2, BASE_Y + ly + 2, COLOR15_WHITE);
    draw_hline(BASE_X + lx - 5, BASE_X + lx + 5, BASE_Y + ly, COLOR15_WHITE);
    draw_vline(BASE_X + lx, BASE_Y + ly - 5, BASE_Y + ly + 5, COLOR15_WHITE);
  }

  void load_savestate_timestamps(void)
  {
    for (i = 0; i < 10; i++)
    {
      get_savestate_filename(i, filename_buffer);
      get_savestate_info(filename_buffer, NULL, line_buffer);
      sprintf(savestate_timestamps[i], "%d: %s", i, line_buffer);
    }
  }

  void gamepak_file_none(void)
  {
    gamepak_filename[0] = 0;
    game_title[0] = 0;

    first_load = 1;

    memset(current_screen, 0x00, GBA_SCREEN_SIZE);
	if (option_language == 0)
    print_string_ext(MSG[MSG_NON_LOAD_GAME], X_POS_CENTER, 74, COLOR15_WHITE, BG_NO_FILL, current_screen, GBA_SCREEN_WIDTH);
	else
    print_string_ext_gbk(MSG[MSG_NON_LOAD_GAME], X_POS_CENTER, 74, COLOR15_WHITE, BG_NO_FILL, current_screen, GBA_SCREEN_WIDTH);
  }

  void gamepak_file_reopen(void)
  {
    for (i = 0; i < 5; i++)
    {
      FILE_OPEN(gamepak_file_large, gamepak_filename_raw, READ);

      if (FILE_CHECK_VALID(gamepak_file_large))
        return;

      sceKernelDelayThread(500 * 1000);
    }

    clear_screen(COLOR32_BLACK);
    error_msg(MSG[MSG_ERR_OPEN_GAMEPACK], CONFIRMATION_QUIT);
    quit();
  }


  // Marker for help information, don't go past this mark (except \n)------*
  MenuOptionType emulator_options[] =
  {
    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_0], scale_options, &option_screen_scale, 4, MSG_OPTION_MENU_HELP_0, 0),

    NUMERIC_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_1], &option_screen_mag, 201, MSG_OPTION_MENU_HELP_1, 1),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_2], on_off_options, &option_screen_filter, 2, MSG_OPTION_MENU_HELP_2, 2),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_SHOW_FPS], on_off_options, &psp_fps_debug, 2, MSG_OPTION_MENU_HELP_SHOW_FPS, 3),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_3], frameskip_options, &option_frameskip_type, 3, MSG_OPTION_MENU_HELP_3, 5),

    NUMERIC_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_4], &option_frameskip_value, 10, MSG_OPTION_MENU_HELP_4, 6),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_5], clock_speed_options, &option_clock_speed, 4, MSG_OPTION_MENU_HELP_5, 7), 

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_6], sound_volume_options, &option_sound_volume, 11, MSG_OPTION_MENU_HELP_6, 8),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_7], stack_optimize_options, &option_stack_optimize, 2, MSG_OPTION_MENU_HELP_7, 10),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_8], yes_no_options, &option_boot_mode, 2, MSG_OPTION_MENU_HELP_8, 11),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_OPTION_MENU_9], update_backup_options, &option_update_backup, 2, MSG_OPTION_MENU_HELP_9, 12), 

    STRING_SELECTION_ACTION_OPTION(menu_exit, NULL, MSG[MSG_OPTION_MENU_10], language_option, &option_language, 4, MSG_OPTION_MENU_HELP_10, 14), 

    ACTION_OPTION(menu_default, NULL, MSG[MSG_OPTION_MENU_DEFAULT], MSG_OPTION_MENU_HELP_DEFAULT, 16),

    ACTION_SUBMENU_OPTION(NULL, menu_init, MSG[MSG_OPTION_MENU_11], MSG_OPTION_MENU_HELP_11, 17)
  };

  MAKE_MENU(emulator, submenu_emulator, NULL);


  MenuOptionType cheats_misc_options[] =
  {
    CHEAT_OPTION((10 * menu_cheat_page) + 0),
    CHEAT_OPTION((10 * menu_cheat_page) + 1),
    CHEAT_OPTION((10 * menu_cheat_page) + 2),
    CHEAT_OPTION((10 * menu_cheat_page) + 3),
    CHEAT_OPTION((10 * menu_cheat_page) + 4),
    CHEAT_OPTION((10 * menu_cheat_page) + 5),
    CHEAT_OPTION((10 * menu_cheat_page) + 6),
    CHEAT_OPTION((10 * menu_cheat_page) + 7),
    CHEAT_OPTION((10 * menu_cheat_page) + 8),
    CHEAT_OPTION((10 * menu_cheat_page) + 9),

    NUMERIC_SELECTION_OPTION(reload_cheats_page, MSG[MSG_CHEAT_MENU_3], &menu_cheat_page, MAX_CHEATS_PAGE, MSG_CHEAT_MENU_HELP_3, 11),
    ACTION_OPTION(menu_load_cheat_file, NULL, MSG[MSG_CHEAT_MENU_1], MSG_CHEAT_MENU_HELP_1, 13),

    SUBMENU_OPTION(NULL, MSG[MSG_CHEAT_MENU_2], MSG_CHEAT_MENU_HELP_2, 15)
  };

  MAKE_MENU(cheats_misc, submenu_cheats_misc, NULL);

  MenuOptionType savestate_options[] =
  {
    SAVESTATE_OPTION(0),
    SAVESTATE_OPTION(1),
    SAVESTATE_OPTION(2),
    SAVESTATE_OPTION(3),
    SAVESTATE_OPTION(4),
    SAVESTATE_OPTION(5),
    SAVESTATE_OPTION(6),
    SAVESTATE_OPTION(7),
    SAVESTATE_OPTION(8),
    SAVESTATE_OPTION(9),

    ACTION_OPTION(menu_load_state_file, NULL, MSG[MSG_STATE_MENU_1], MSG_STATE_MENU_HELP_1, 11),

    ACTION_SUBMENU_OPTION(NULL, menu_init, MSG[MSG_STATE_MENU_2], MSG_STATE_MENU_HELP_2, 13)
  };

  MAKE_MENU(savestate, submenu_savestate, NULL);

  MenuOptionType gamepad_config_options[] =
  {
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_0], 0),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_1], 1),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_2], 2),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_3], 3),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_4], 4),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_5], 5),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_6], 6),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_7], 7),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_8], 8),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_9], 9),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_10], 10),
    GAMEPAD_CONFIG_OPTION(MSG[MSG_PAD_MENU_11], 11),

    ACTION_SUBMENU_OPTION(NULL, menu_init, MSG[MSG_PAD_MENU_12], MSG_PAD_MENU_HELP_1, 13)
  };

  MAKE_MENU(gamepad_config, submenu_gamepad, NULL);

  MenuOptionType analog_config_options[] =
  {
    ANALOG_CONFIG_OPTION(MSG[MSG_A_PAD_MENU_0], 0),
    ANALOG_CONFIG_OPTION(MSG[MSG_A_PAD_MENU_1], 1),
    ANALOG_CONFIG_OPTION(MSG[MSG_A_PAD_MENU_2], 2),
    ANALOG_CONFIG_OPTION(MSG[MSG_A_PAD_MENU_3], 3),

    STRING_SELECTION_OPTION(NULL, MSG[MSG_A_PAD_MENU_4], yes_no_options, &option_enable_analog, 2, MSG_A_PAD_MENU_HELP_0, 5),
    NUMERIC_SELECTION_OPTION(NULL, MSG[MSG_A_PAD_MENU_5], &option_analog_sensitivity, 10, MSG_A_PAD_MENU_HELP_1, 6),

    ACTION_SUBMENU_OPTION(NULL, menu_init, MSG[MSG_A_PAD_MENU_6], MSG_A_PAD_MENU_HELP_2, 8)
  };

  MAKE_MENU(analog_config, submenu_analog, NULL);

  MenuOptionType main_options[] =
  {
    NUMERIC_SELECTION_ACTION_OPTION(menu_load_state, NULL, MSG[MSG_MAIN_MENU_0], &savestate_slot, 10, MSG_MAIN_MENU_HELP_0, 0),

    NUMERIC_SELECTION_ACTION_OPTION(menu_save_state, NULL, MSG[MSG_MAIN_MENU_1], &savestate_slot, 10, MSG_MAIN_MENU_HELP_1, 1),

    ACTION_SUBMENU_OPTION(&savestate_menu, menu_init, MSG[MSG_MAIN_MENU_2], MSG_MAIN_MENU_HELP_2, 2),

    STRING_SELECTION_ACTION_OPTION(menu_screen_capture, NULL, MSG[MSG_MAIN_MENU_3], image_format_options, &option_screen_capture_format, 2, MSG_MAIN_MENU_HELP_3, 4),

    SUBMENU_OPTION(&emulator_menu, MSG[MSG_MAIN_MENU_4], MSG_MAIN_MENU_HELP_4, 6), 

    SUBMENU_OPTION(&gamepad_config_menu, MSG[MSG_MAIN_MENU_5], MSG_MAIN_MENU_HELP_5, 7),

    SUBMENU_OPTION(&analog_config_menu, MSG[MSG_MAIN_MENU_6], MSG_MAIN_MENU_HELP_6, 8),

    SUBMENU_OPTION(&cheats_misc_menu, MSG[MSG_MAIN_MENU_CHEAT], MSG_MAIN_MENU_HELP_CHEAT, 10),

    ACTION_OPTION(menu_load_file, NULL, MSG[MSG_MAIN_MENU_7], MSG_MAIN_MENU_HELP_7, 11),

    ACTION_OPTION(menu_reset, NULL, MSG[MSG_MAIN_MENU_8], MSG_MAIN_MENU_HELP_8, 13),

    ACTION_OPTION(menu_exit, NULL, MSG[MSG_MAIN_MENU_9], MSG_MAIN_MENU_HELP_9, 14),

    ACTION_OPTION(menu_suspend, NULL, MSG[MSG_MAIN_MENU_10], MSG_MAIN_MENU_HELP_10, 16),

    ACTION_OPTION(menu_quit, NULL, MSG[MSG_MAIN_MENU_11], MSG_MAIN_MENU_HELP_11, 17)
  };

  MAKE_MENU(main, submenu_main, NULL);

  void choose_menu(MenuType *new_menu)
  {
    if (new_menu == NULL)
      new_menu = &main_menu;

    current_menu = new_menu;
    current_option = new_menu->options;
    current_option_num = 0;
  }

  void reload_cheats_page()
  {
    for(i = 0; i<10; i++)
    {
      cheats_misc_options[i].display_string = cheat_format_str[(10 * menu_cheat_page) + i];
      cheats_misc_options[i].current_option = &(cheats[(10 * menu_cheat_page) + i].cheat_active);
    }
  }


  sound_pause = 1;

  current_screen = copy_screen();
  savestate_screen = (u16 *)safe_malloc(GBA_SCREEN_SIZE);

  if (gamepak_filename[0] == 0)
    gamepak_file_none();
  else
    change_ext(gamepak_filename, game_title, "");

  screen_image_ptr = current_screen;

  load_savestate_timestamps();

  if (FILE_CHECK_VALID(gamepak_file_large))
  {
    FILE_CLOSE(gamepak_file_large);
    gamepak_file_large = -2;
  }


  for(i = 0; i < MAX_CHEATS; i++)
  {
    if(i >= num_cheats)
    {
      sprintf(cheat_format_str[i], MSG[MSG_CHEAT_MENU_NON_LOAD], i);
    }
    else
    {
      sprintf(cheat_format_str[i], MSG[MSG_CHEAT_MENU_0], i, cheats[i].cheat_name);
    }
  }
  //menu_cheat_page = 0;
  reload_cheats_page();

  video_resolution_large();
  set_cpu_clock(PSP_CLOCK_222);
  choose_menu(&main_menu);

  while (repeat)
  {
    clear_screen(COLOR15_TO_32(COLOR_BG));

    if ((counter % 30) == 0)
	{
	  if (option_language == 0)
      update_status_string(time_str, batt_str, &color_batt_life);
	  else
      update_status_string_gbk(time_str, batt_str, &color_batt_life);
	}
    counter++;
	if (option_language == 0)
    print_string(time_str, TIME_STATUS_POS_X, 2, COLOR_HELP_TEXT, BG_NO_FILL);
	else
    print_string_gbk(time_str, TIME_STATUS_POS_X, 2, COLOR_HELP_TEXT, BG_NO_FILL);

	if (option_language == 0)
    print_string(batt_str, BATT_STATUS_POS_X, 2, color_batt_life, BG_NO_FILL);
	else
	print_string_gbk(batt_str, BATT_STATUS_POS_X, 2, color_batt_life, BG_NO_FILL);

    print_string(game_title, 228, 28, COLOR_INACTIVE_ITEM, BG_NO_FILL);
    blit_to_screen(screen_image_ptr, GBA_SCREEN_WIDTH, GBA_SCREEN_HEIGHT, SCREEN_IMAGE_POS_X, SCREEN_IMAGE_POS_Y);

    //print_string(backup_id, 228, 208, COLOR_INACTIVE_ITEM, BG_NO_FILL);

    if (current_menu->init_function != NULL)
    {
      current_menu->init_function();
    }

    display_option = current_menu->options;

    for (i = 0; i < current_menu->num_options; i++, display_option++)
    {
      if (display_option->option_type & NUMBER_SELECTION_OPTION)
      {
        sprintf(line_buffer, display_option->display_string, *(display_option->current_option));
      }
      else
      {
        if (display_option->option_type & STRING_SELECTION_OPTION)
          sprintf(line_buffer, display_option->display_string, ((u32 *)display_option->options)[*(display_option->current_option)]);
        else
          strcpy(line_buffer, display_option->display_string);
      }
/*file charset*/
		if (option_language == 0)
			print_string(line_buffer, MENU_LIST_POS_X, (display_option->line_number * FONTHEIGHT) + 28, (display_option == current_option) ? COLOR_ACTIVE_ITEM : COLOR_INACTIVE_ITEM, BG_NO_FILL);
		else
			print_string_gbk(line_buffer, MENU_LIST_POS_X, (display_option->line_number * FONTHEIGHT) + 28, (display_option == current_option) ? COLOR_ACTIVE_ITEM : COLOR_INACTIVE_ITEM, BG_NO_FILL);
    }

	if (option_language == 0)
		print_string(MSG[current_option->help_string], 30, 258, COLOR_HELP_TEXT, BG_NO_FILL);
	else
		print_string_gbk(MSG[current_option->help_string], 30, 258, COLOR_HELP_TEXT, BG_NO_FILL);

    // PSP controller - hold
    if (get_pad_input(PSP_CTRL_HOLD) != 0)
      print_string(FONT_KEY_ICON, 6, 258, COLOR15_YELLOW, BG_NO_FILL);

    __draw_volume_status(1);
    flip_screen(1);


    gui_action = get_gui_input();

    switch (gui_action)
    {
      case CURSOR_DOWN:
        current_option_num = (current_option_num + 1) % current_menu->num_options;

        current_option = current_menu->options + current_option_num;
        break;

      case CURSOR_UP:
        if (current_option_num != 0)
          current_option_num--;
        else
          current_option_num = current_menu->num_options - 1;

        current_option = current_menu->options + current_option_num;
        break;

      case CURSOR_RIGHT:
        if ((current_option->option_type & (NUMBER_SELECTION_OPTION | STRING_SELECTION_OPTION)) != 0)
        {
          *(current_option->current_option) = (*current_option->current_option + 1) % current_option->num_options;

          if (current_option->passive_function != NULL)
            current_option->passive_function();
        }
        break;

      case CURSOR_LEFT:
        if ((current_option->option_type & (NUMBER_SELECTION_OPTION | STRING_SELECTION_OPTION)) != 0)
        {
          u32 current_option_val = *(current_option->current_option);

          if (current_option_val != 0)
            current_option_val--;
          else
            current_option_val = current_option->num_options - 1;

          *(current_option->current_option) = current_option_val;

          if (current_option->passive_function != NULL)
            current_option->passive_function();
        }
        break;

      case CURSOR_RTRIGGER:
        if (current_menu == &main_menu)
        {
          menu_init();
          choose_menu(&savestate_menu);
        }
        break;

      case CURSOR_LTRIGGER:
        if (current_menu == &main_menu)
          menu_load_file();
        if (current_menu == &cheats_misc_menu)
          menu_load_cheat_file();
        break;

      case CURSOR_DEFAULT:
	  {
        /*if (current_menu == &emulator_menu)
		{	
			option_screen_scale = SCALED_X15_GU;
			option_screen_mag = 170;
			option_screen_filter = FILTER_BILINEAR;
			psp_fps_debug = 0;
			option_frameskip_type = FRAMESKIP_AUTO;
			option_frameskip_value = 9;
			option_clock_speed = PSP_CLOCK_333;
			option_sound_volume = 10;
			option_stack_optimize = 1;
			option_boot_mode = 0;
			option_update_backup = 0;
			option_screen_capture_format = 0;
			option_enable_analog = 0;
			option_analog_sensitivity = 4;
		}*/
	}
        break;

      case CURSOR_EXIT:
        if (current_menu == &main_menu)
        {
          menu_exit();
        }
        else
        {
          menu_init();
          choose_menu(&main_menu);
        }
        break;

      case CURSOR_SELECT:
        switch (current_option->option_type & (ACTION_OPTION | SUBMENU_OPTION))
        {
          case (ACTION_OPTION | SUBMENU_OPTION):
            current_option->action_function();
            choose_menu(current_option->sub_menu);
            break;

          case ACTION_OPTION:
            current_option->action_function();
            break;

          case SUBMENU_OPTION:
            choose_menu(current_option->sub_menu);
            break;

          default:
            break;
        }
        break;

      case CURSOR_BACK:
      case CURSOR_NONE:
        break;
    }

  } /* end while */


  scePowerLock(0);

  if (gamepak_file_large == -2)
    gamepak_file_reopen();

  while (get_pad_input(0x0001FFFF) != 0);

  menu_term();

  set_sound_volume();
  set_cpu_clock(option_clock_speed);

  sceDisplayWaitVblankStart();
  video_resolution_small();

  sound_pause = 0;
  //menu_cheat_page = 0;//

  scePowerUnlock(0);

  return return_value;
}


/*-----------------------------------------------------------------------------
  Status bar
-----------------------------------------------------------------------------*/

static void update_status_string(char *time_str, char *batt_str, u16 *color_batt)
{
  pspTime current_time = { 0 };

  u32 i = 0;
  int batt_life_per;
  int batt_life_time;

  char batt_icon[4][4] =
  {
    FONT_BATTERY0 "\0", // empty
    FONT_BATTERY1 "\0",
    FONT_BATTERY2 "\0",
    FONT_BATTERY3 "\0", // full
  };

  sceRtcGetCurrentClockLocalTime(&current_time);
  int day_of_week = sceRtcGetDayOfWeek(current_time.year, current_time.month, current_time.day);

  get_timestamp_string(time_str, MSG_MENU_DATE_FMT_0, &current_time, day_of_week);


  batt_life_per = scePowerGetBatteryLifePercent();

  if (batt_life_per < 0)
  {
    sprintf(batt_str, "%3s --%%", batt_icon[0]);
  }
  else
  {
    if (batt_life_per > 66)      i = 3;
    else if (batt_life_per > 33) i = 2;
    else if (batt_life_per >  9) i = 1;
    else                         i = 0;

    sprintf(batt_str, "%3s%3d%%", batt_icon[i], batt_life_per);
  }

  if (scePowerIsPowerOnline() == 1)
  {
    sprintf(batt_str, "%s%s", batt_str, MSG[MSG_CHARGE]);
  }
  else
  {
    batt_life_time = scePowerGetBatteryLifeTime();

    if (batt_life_time < 0)
      sprintf(batt_str, "%s%s", batt_str, "[--:--]");
    else
      sprintf(batt_str, "%s[%2d:%02d]", batt_str, (batt_life_time / 60) % 100, batt_life_time % 60);
  }

  if (scePowerIsBatteryCharging() == 1)
  {
    *color_batt = COLOR_BATT_CHARG;
  }
  else
  {
    if (scePowerIsLowBattery() == 1)
      *color_batt = COLOR_BATT_LOW;
    else
      *color_batt = COLOR_BATT_NORMAL;
  }
}

static void update_status_string_gbk(char *time_str, char *batt_str, u16 *color_batt)
{
  pspTime current_time = { 0 };

  u32 i = 0;
  int batt_life_per;
  int batt_life_time;

  char batt_icon[4][4] =
  {
    FONT_BATTERY0_GBK "\0", // empty
    FONT_BATTERY1_GBK "\0",
    FONT_BATTERY2_GBK "\0",
    FONT_BATTERY3_GBK "\0", // full
  };

  sceRtcGetCurrentClockLocalTime(&current_time);
  int day_of_week = sceRtcGetDayOfWeek(current_time.year, current_time.month, current_time.day);

  get_timestamp_string(time_str, MSG_MENU_DATE_FMT_0, &current_time, day_of_week);


  batt_life_per = scePowerGetBatteryLifePercent();

  if (batt_life_per < 0)
  {
    sprintf(batt_str, "%3s --%%", batt_icon[0]);
  }
  else
  {
    if (batt_life_per > 66)      i = 3;
    else if (batt_life_per > 33) i = 2;
    else if (batt_life_per >  9) i = 1;
    else                         i = 0;

    sprintf(batt_str, "%3s%3d%%", batt_icon[i], batt_life_per);
  }

  if (scePowerIsPowerOnline() == 1)
  {
    sprintf(batt_str, "%s%s", batt_str, MSG[MSG_CHARGE]);
  }
  else
  {
    batt_life_time = scePowerGetBatteryLifeTime();

    if (batt_life_time < 0)
      sprintf(batt_str, "%s%s", batt_str, "[--:--]");
    else
      sprintf(batt_str, "%s[%2d:%02d]", batt_str, (batt_life_time / 60) % 100, batt_life_time % 60);
  }

  if (scePowerIsBatteryCharging() == 1)
  {
    *color_batt = COLOR_BATT_CHARG;
  }
  else
  {
    if (scePowerIsLowBattery() == 1)
      *color_batt = COLOR_BATT_LOW;
    else
      *color_batt = COLOR_BATT_NORMAL;
  }
}


static void get_timestamp_string(char *buffer, u16 msg_id, pspTime *msg_time, int day_of_week)
{
  const char *week_str[] =
  {
    MSG[MSG_DAYW_0], MSG[MSG_DAYW_1], MSG[MSG_DAYW_2], MSG[MSG_DAYW_3], MSG[MSG_DAYW_4], MSG[MSG_DAYW_5], MSG[MSG_DAYW_6], ""
  };

  switch (date_format)
  {
    case 0: // DATE_FORMAT_YYYYMMDD
      sprintf(buffer, MSG[msg_id + 0], msg_time->year, msg_time->month, msg_time->day, week_str[day_of_week], msg_time->hour, msg_time->minutes, msg_time->seconds, (msg_time->microseconds / 1000));
      break;
    case 1: // DATE_FORMAT_MMDDYYYY
      sprintf(buffer, MSG[msg_id + 1], msg_time->month, msg_time->day, msg_time->year, week_str[day_of_week], msg_time->hour, msg_time->minutes, msg_time->seconds, (msg_time->microseconds / 1000));
      break;
    case 2: // DATE_FORMAT_DDMMYYYY
      sprintf(buffer, MSG[msg_id + 1], msg_time->day, msg_time->month, msg_time->year, week_str[day_of_week], msg_time->hour, msg_time->minutes, msg_time->seconds, (msg_time->microseconds / 1000));
      break;
  }
}


/*-----------------------------------------------------------------------------
  Save Config Files
-----------------------------------------------------------------------------*/

static s32 save_game_config_file(void)
{
  SceUID game_config_file;
  char game_config_path[MAX_PATH];
  char game_config_filename[MAX_FILE];
  s32 return_value = -1;

  if (gamepak_filename[0] == 0)
    return return_value;

  change_ext(gamepak_filename, game_config_filename, ".cfg");
  sprintf(game_config_path, "%s%s", dir_cfg, game_config_filename);

  scePowerLock(0);

  FILE_OPEN(game_config_file, game_config_path, WRITE);

  if (FILE_CHECK_VALID(game_config_file))
  {
    u32 i;
    u32 file_options[GPSP_GAME_CONFIG_NUM];

    file_options[0]  = option_screen_scale;
    file_options[1]  = option_screen_mag;
    file_options[2]  = option_screen_filter;
    file_options[3] = option_frameskip_type;
    file_options[4] = option_frameskip_value;
    file_options[5] = option_clock_speed;
    file_options[6]  = option_sound_volume;

    for (i = 0; i < 16; i++)
    {
      file_options[7 + i] = gamepad_config_map[i];
    }

    FILE_WRITE_ARRAY(game_config_file, file_options);
    FILE_CLOSE(game_config_file);

    return_value = 0;
  }

  scePowerUnlock(0);

  return return_value;
}

s32 save_config_file(void)
{
  SceUID config_file;
  char config_path[MAX_PATH];

  s32 ret_value = -1;

  save_game_config_file();

  sprintf(config_path, "%s%s", main_path, GPSP_CONFIG_FILENAME);

  scePowerLock(0);

  FILE_OPEN(config_file, config_path, WRITE);

  if (FILE_CHECK_VALID(config_file))
  {
    u32 i;
    u32 file_options[GPSP_CONFIG_NUM];

    file_options[0]  = option_screen_scale;
    file_options[1]  = option_screen_mag;
    file_options[2]  = option_screen_filter;
    file_options[3] = psp_fps_debug;
    file_options[4] = option_frameskip_type;
    file_options[5] = option_frameskip_value;
    file_options[6] = option_clock_speed;
    file_options[7]  = option_sound_volume;
    file_options[8]  = option_stack_optimize;
    file_options[9]  = option_boot_mode;
    file_options[10]  = option_update_backup;
    file_options[11]  = option_screen_capture_format;
    file_options[12]  = option_enable_analog;
    file_options[13]  = option_analog_sensitivity;
    file_options[14] = option_language;

    for (i = 0; i < 16; i++)
    {
      file_options[15 + i] = gamepad_config_map[i];
    }

    FILE_WRITE_ARRAY(config_file, file_options);
    FILE_CLOSE(config_file);

    ret_value = 0;
  }

  scePowerUnlock(0);

  return ret_value;
}


/*-----------------------------------------------------------------------------
  Load Config Files
-----------------------------------------------------------------------------*/

s32 load_game_config_file(void)
{
  SceUID game_config_file;
  char game_config_filename[MAX_FILE];
  char game_config_path[MAX_PATH];

  change_ext(gamepak_filename, game_config_filename, ".cfg");
  sprintf(game_config_path, "%s%s", dir_cfg, game_config_filename);

  FILE_OPEN(game_config_file, game_config_path, READ);

  if (FILE_CHECK_VALID(game_config_file))
  {
    u32 file_size = file_length(game_config_path);

    // Sanity check: File size must be the right size
    if (file_size == (GPSP_GAME_CONFIG_NUM * 4))
    {
      u32 i;
      u32 file_options[file_size / 4];
      s32 menu_button = -1;

      FILE_READ_ARRAY(game_config_file, file_options);

      option_screen_scale   = file_options[0] % 4;
      option_screen_mag     = file_options[1] % 201;
      option_screen_filter  = file_options[2] % 2;
      option_frameskip_type  = file_options[3] % 3;
      option_frameskip_value = file_options[4];
      option_clock_speed     = file_options[5] % 4;
      option_sound_volume   = file_options[6] % 11;

      for (i = 0; i < 16; i++)
      {
        gamepad_config_map[i] = file_options[7 + i] % (BUTTON_ID_NONE + 1);

        if (gamepad_config_map[i] == BUTTON_ID_MENU)
          menu_button = i;
      }

      if ((enable_home_menu == 0) && (menu_button == -1))
        gamepad_config_map[0] = BUTTON_ID_MENU;

      if (option_frameskip_value > 9)
        option_frameskip_value = 9;

	  u32 j;
      for(j = 0; j < MAX_CHEATS; j++)
      {
        cheats[j].cheat_active = 0;
        cheats[j].cheat_name[0] = '\0';
      }

      FILE_CLOSE(game_config_file);

      return 0;
    }
  }

  option_frameskip_type = FRAMESKIP_AUTO;
  option_frameskip_value = 9;
  option_clock_speed = PSP_CLOCK_333;

  return -1;
}

s32 load_config_file(void)
{
  SceUID config_file;
  char config_path[MAX_PATH];

  sprintf(config_path, "%s%s", main_path, GPSP_CONFIG_FILENAME);

  FILE_OPEN(config_file, config_path, READ);

  if (FILE_CHECK_VALID(config_file))
  {
    u32 file_size = file_length(config_path);

    // Sanity check: File size must be the right size
    if (file_size == (GPSP_CONFIG_NUM * 4))
    {
      u32 i;
      u32 file_options[file_size / 4];
      s32 menu_button = -1;

      FILE_READ_ARRAY(config_file, file_options);

      option_screen_scale   = file_options[0] % 4;
      option_screen_mag     = file_options[1] % 201;
      option_screen_filter  = file_options[2] % 2;
      psp_fps_debug       = file_options[3] % 2;
      option_frameskip_type  = file_options[4] % 3;
      option_frameskip_value = file_options[5];
      option_clock_speed     = file_options[6] % 4;
      option_sound_volume   = file_options[7] % 11;
      option_stack_optimize = file_options[8] % 2;
      option_boot_mode      = file_options[9] % 2;
      option_update_backup  = file_options[10] % 2;
      option_screen_capture_format = file_options[11] % 2;
      option_enable_analog  = file_options[12] % 2;
      option_analog_sensitivity = file_options[13] % 10;
      option_language       = file_options[14] % 4;

      for (i = 0; i < 16; i++)
      {
        gamepad_config_map[i] = file_options[15 + i] % (BUTTON_ID_NONE + 1);

        if (gamepad_config_map[i] == BUTTON_ID_MENU)
          menu_button = i;
      }

      if ((enable_home_menu == 0) && (menu_button == -1))
        gamepad_config_map[0] = BUTTON_ID_MENU;

      FILE_CLOSE(config_file);
    }

    return 0;
  }

  option_screen_scale = SCALED_X15_GU;
  option_screen_mag = 170;
  option_screen_filter = FILTER_BILINEAR;
  psp_fps_debug = 0;
  option_sound_volume = 10;
  option_stack_optimize = 1;
  option_boot_mode = 0;
  option_update_backup = 1;		//auto
  option_screen_capture_format = 0;
  option_enable_analog = 0;
  option_analog_sensitivity = 4;

  int id_language;
  sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &id_language);
  if (id_language == PSP_SYSTEMPARAM_LANGUAGE_JAPANESE)
    option_language = 0;
  else if (id_language == PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED)
    option_language = 2;
  else if (id_language == PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL)
    option_language = 3;
  else
    option_language = 1;

  return -1;
}


s32 load_dir_cfg(char *file_name)
{
  char current_line[256];
  char current_variable[256];
  char current_value[256];

  const char item_roms[]  = "rom_directory";
  const char item_save[]  = "save_directory";
  const char item_state[] = "save_state_directory";
  const char item_cfg[]   = "game_config_directory";
  const char item_snap[]  = "snapshot_directory";
  const char item_cheat[]  = "cheat_directory";

  FILE *dir_config;
  SceUID check_dir = -1;

  char str_buf[256];
  u32 str_line = 7;

  auto void add_launch_directory(void);
  auto void set_directory(char *dir_name, const char *item_name);
  auto void check_directory(char *dir_name, const char *item_name);

  void add_launch_directory(void)
  {
    if (strchr(current_value, ':') == NULL)
    {
      strcpy(str_buf, current_value);
      sprintf(current_value, "%s%s", main_path, str_buf);
    }

    if (current_value[strlen(current_value) - 1] != '/')
    {
      strcat(current_value, "/");
    }
  }

  void set_directory(char *dir_name, const char *item_name)
  {
    if (strcasecmp(current_variable, item_name) == 0)
    {
      if ((check_dir = sceIoDopen(current_value)) >= 0)
      {
        strcpy(dir_name, current_value);
        sceIoDclose(check_dir);
      }
      else
      {
        sprintf(str_buf, MSG[MSG_ERR_SET_DIR_0], current_variable);
	    if (option_language == 0)
        print_string(str_buf, 7, str_line, COLOR15_WHITE, COLOR15_BLACK);
		else
        print_string_gbk(str_buf, 7, str_line, COLOR15_WHITE, COLOR15_BLACK);
        str_line += FONTHEIGHT;

        strcpy(dir_name, main_path);
      }
    }
  }

  void check_directory(char *dir_name, const char *item_name)
  {
    if (dir_name[0] == 0)
    {
      sprintf(str_buf, MSG[MSG_ERR_SET_DIR_1], item_name);
	  if (option_language == 0)
      print_string(str_buf, 7, str_line, COLOR15_WHITE, COLOR15_BLACK);
	  else
      print_string_gbk(str_buf, 7, str_line, COLOR15_WHITE, COLOR15_BLACK);
      str_line += FONTHEIGHT;

      strcpy(dir_name, main_path);
    }
  }

  dir_roms[0]  = 0;
  dir_save[0]  = 0;
  dir_state[0] = 0;
  dir_cfg[0]   = 0;
  dir_snap[0]  = 0;
  dir_cheat[0]  = 0;

  dir_config = fopen(file_name, "r");

  if (dir_config != NULL)
  {
    while (fgets(current_line, 256, dir_config))
    {
      if (parse_config_line(current_line, current_variable, current_value) != -1)
      {
        add_launch_directory();

        set_directory(dir_roms,  item_roms);
        set_directory(dir_save,  item_save);
        set_directory(dir_state, item_state);
        set_directory(dir_cfg,   item_cfg);
        set_directory(dir_snap,  item_snap);
        set_directory(dir_cheat, item_cheat);
      }
    }

    fclose(dir_config);

    check_directory(dir_roms,  item_roms);
    check_directory(dir_save,  item_save);
    check_directory(dir_state, item_state);
    check_directory(dir_cfg,   item_cfg);
    check_directory(dir_snap,  item_snap);
    check_directory(dir_cheat, item_cheat);

    if (str_line > 7)
    {
      sprintf(str_buf, MSG[MSG_ERR_SET_DIR_2], main_path);
      sprintf(str_buf, "%s\n\n%s", str_buf, MSG[MSG_ERR_CONT]);

      str_line += FONTHEIGHT;
	  if (option_language == 0)
      print_string(str_buf, 7, str_line, COLOR15_WHITE, COLOR15_BLACK);
	  else
      print_string_gbk(str_buf, 7, str_line, COLOR15_WHITE, COLOR15_BLACK);
      error_msg("", CONFIRMATION_NONE);
    }

    return 0;
  }

  // set launch directory
  strcpy(dir_roms,  main_path);
  strcpy(dir_save,  main_path);
  strcpy(dir_state, main_path);
  strcpy(dir_cfg,   main_path);
  strcpy(dir_snap,  main_path);
  strcpy(dir_cheat, main_path);
  return -1;
}


/*-----------------------------------------------------------------------------
  Screen Capture
-----------------------------------------------------------------------------*/

static void get_snapshot_filename(char *name, const char *ext)
{
  char filename[MAX_FILE];
  char timestamp[80];

  pspTime current_time = { 0 };

  change_ext(gamepak_filename, filename, "_");

  sceRtcGetCurrentClockLocalTime(&current_time);
  get_timestamp_string(timestamp, MSG_SS_DATE_FMT_0, &current_time, 7);

  sprintf(name, "%s%s%s.%s", dir_snap, filename, timestamp, ext);
}

static void save_bmp(const char *path, u16 *screen_image)
{
  const u8 ALIGN_DATA header[] =
  {
     'B',  'M', 0x36, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
    0x00, 0x00,  240, 0x00, 0x00, 0x00,  160, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  SceUID ss;

  u32 x, y;
  u16 color;
  u16 *src_ptr;
  u8  *bmp_data, *dest_ptr;

  bmp_data = (u8 *)malloc(GBA_SCREEN_WIDTH * GBA_SCREEN_HEIGHT * 3);

  if (bmp_data == NULL)
  {
    clear_screen(COLOR32_BLACK);
    error_msg(MSG[MSG_ERR_MALLOC], CONFIRMATION_CONT);
    return;
  }

  dest_ptr = bmp_data;

  for (y = 0; y < GBA_SCREEN_HEIGHT; y++)
  {
    src_ptr = &screen_image[(GBA_SCREEN_HEIGHT - y - 1) * GBA_SCREEN_WIDTH];

    for (x = 0; x < GBA_SCREEN_WIDTH; x++)
    {
      color = src_ptr[x];

      *dest_ptr++ = (u8)COL15_GET_B8(color);
      *dest_ptr++ = (u8)COL15_GET_G8(color);
      *dest_ptr++ = (u8)COL15_GET_R8(color);
    }
  }

  FILE_OPEN(ss, path, WRITE);

  if (FILE_CHECK_VALID(ss))
  {
    FILE_WRITE_VARIABLE(ss, header);
    FILE_WRITE(ss, bmp_data, GBA_SCREEN_WIDTH * GBA_SCREEN_HEIGHT * 3);
    FILE_CLOSE(ss);
  }

  free(bmp_data);
}

