#define FONTWIDTH  (6)
#define FONTHEIGHT (12)

#define BG_NO_FILL (-1)


#define FONT_BATTERY0_GBK          "\x13\x16\x17" // empty
#define FONT_BATTERY1_GBK          "\x13\x15\x12"
#define FONT_BATTERY2_GBK          "\x13\x14\x12"
#define FONT_BATTERY3_GBK          "\x10\x11\x12" // full
#define FONT_GBA_ICON_GBK          "\x18\xA1\x4C"
#define FONT_PSP_ICON_GBK          "\x19\xA1\x4D"
#define FONT_MSC_ICON_GBK          "\x1A\xA1\x4E"
#define FONT_OPT_ICON_GBK          "\x1B\xA1\x4F"
#define FONT_PAD_ICON_GBK          "\x1C\xA1\x50"

#define FONT_KEY_ICON_GBK          "\xA1\x40"
#define FONT_R_TRIGGER_GBK         "\xA1\x41"
#define FONT_L_TRIGGER_GBK         "\xA1\x42"
#define FONT_CURSOR_RIGHT_GBK      "\xA1\x43"
#define FONT_CURSOR_LEFT_GBK       "\xA1\x44"
#define FONT_CURSOR_UP_GBK         "\xA1\x45"
#define FONT_CURSOR_DOWN_GBK       "\xA1\x46"
#define FONT_CURSOR_RIGHT_FILL_GBK "\xA1\x47"
#define FONT_CURSOR_LEFT_FILL_GBK  "\xA1\x48"
#define FONT_CURSOR_UP_FILL_GBK    "\xA1\x49"
#define FONT_CURSOR_DOWN_FILL_GBK  "\xA1\x4A"
#define FONT_UP_DIRECTORY_GBK      "\xA1\x4B"


void ch_print(const char *str, u16 x, u16 y, u16 col, s16 bg_col, u16 *base_vram, u16 bufferwidth);

