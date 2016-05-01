#define FONTWIDTH  (6)
#define FONTHEIGHT (12)

#define BG_NO_FILL (-1)


#define FONT_BATTERY0          "\xF3\xF6\xF7" // empty
#define FONT_BATTERY1          "\xF3\xF5\xF2"
#define FONT_BATTERY2          "\xF3\xF4\xF2"
#define FONT_BATTERY3          "\xF0\xF1\xF2" // full
#define FONT_GBA_ICON          "\xF8\x87\xAC"
#define FONT_PSP_ICON          "\xF9\x87\xAD"
#define FONT_MSC_ICON          "\xFA\x87\xAE"

#define FONT_KEY_ICON          "\x87\xA0"
#define FONT_R_TRIGGER         "\x87\xA1"
#define FONT_L_TRIGGER         "\x87\xA2"
#define FONT_CURSOR_RIGHT      "\x87\xA3"
#define FONT_CURSOR_LEFT       "\x87\xA4"
#define FONT_CURSOR_UP         "\x87\xA5"
#define FONT_CURSOR_DOWN       "\x87\xA6"
#define FONT_CURSOR_RIGHT_FILL "\x87\xA7"
#define FONT_CURSOR_LEFT_FILL  "\x87\xA8"
#define FONT_CURSOR_UP_FILL    "\x87\xA9"
#define FONT_CURSOR_DOWN_FILL  "\x87\xAA"
#define FONT_UP_DIRECTORY      "\x87\xAB"


void mh_print(const char *str, u16 x, u16 y, u16 col, s16 bg_col, u16 *base_vram, u16 bufferwidth);

