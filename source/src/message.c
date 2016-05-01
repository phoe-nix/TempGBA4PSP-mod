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


const char *message[4][MSG_END] =
{
  {
    //MSG_TURBO
	"--FF--",

    //MSG_CHARGE
	"[DC IN]",

    //MSG_BUFFER
	"ROM Buffer %2dMB",

    // MSG_BROWSER_HELP
    "仜:慖戰  亊:儊僯儏乕  仩:" FONT_UP_DIRECTORY,

    // MSG_MENU_DATE_FMT_0
    "%4d/%2d/%2d %-4s %2d:%02d",

    // MSG_MENU_DATE_FMT_1
    "%2d/%2d/%4d %-4s %2d:%02d",

    // MSG_MAIN_MENU_TITLE
	#include "text/main_menu_t.h"

    // MSG_MAIN_MENU_0
    "儘乕僪僗僥乕僩 : 僗儘僢僩 %d",

    // MSG_MAIN_MENU_1
    "僙乕僽僗僥乕僩 : 僗儘僢僩 %d",

    // MSG_MAIN_MENU_2
    "僗僥乕僩僙乕僽奼挘 " FONT_R_TRIGGER,

    // MSG_MAIN_MENU_3
    "僗僋儕乕儞 僔儑僢僩: %s",

    // MSG_MAIN_MENU_4
    "僄儈儏儗乕僞偺愝掕",

    // MSG_MAIN_MENU_5
    "僎乕儉僷僢僪偺愝掕",

    // MSG_MAIN_MENU_6
    "傾僫儘僌僗僥傿僢僋偺愝掕",

	//MSG_MAIN_MENU_CHEAT
    "僠乕僩儊僯儏乕",

    // MSG_MAIN_MENU_7
    "僎乕儉偺儘乕僪 " FONT_L_TRIGGER,

    // MSG_MAIN_MENU_8
    "儕僙僢僩",

    // MSG_MAIN_MENU_9
    "僎乕儉偵栠傞",

    // MSG_MAIN_MENU_10
    "僗儕乕僾",

    // MSG_MAIN_MENU_11
    "TempGBA偺廔椆",

    // MSG_MAIN_MENU_HELP_0
    "仜:儘乕僪  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":僗儘僢僩慖戰",

    // MSG_MAIN_MENU_HELP_1
    "仜:僙乕僽  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":僗儘僢僩慖戰",

    // MSG_MAIN_MENU_HELP_2
    "仜:僒僽儊僯儏乕  " FONT_R_TRIGGER ":僔儑乕僩僇僢僩",

    // MSG_MAIN_MENU_HELP_3
    "仜:僀儊乕僕曐懚  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":僼僅乕儅僢僩慖戰",

    // MSG_MAIN_MENU_HELP_4
    "仜:僒僽儊僯儏乕",

    // MSG_MAIN_MENU_HELP_5
    "仜:僒僽儊僯儏乕",

    // MSG_MAIN_MENU_HELP_6
    "仜:僒僽儊僯儏乕",

    // MSG_MAIN_MENU_HELP_CHEAT
    "仜:僠乕僩傪曄峏偟傑偡",

    // MSG_MAIN_MENU_HELP_7
    "仜:僼傽僀儖僽儔僂僓  " FONT_L_TRIGGER ":僔儑乕僩僇僢僩",

    // MSG_MAIN_MENU_HELP_8
    "仜:僎乕儉傪儕僙僢僩",

    // MSG_MAIN_MENU_HELP_9
    "仜:僎乕儉偵栠傞",

    // MSG_MAIN_MENU_HELP_10
    "仜:僗儕乕僾 儌乕僪",

    // MSG_MAIN_MENU_HELP_11
    "仜:廔椆",

    // MSG_OPTION_MENU_TITLE
	#include "text/option_menu_t.h"

    // MSG_OPTION_MENU_0
    "夋柺昞\帵丂丂丂丂丂: %s",

    // MSG_OPTION_MENU_1
    "夋柺偺奼戝棪丂丂丂: %d%%",

    // MSG_OPTION_MENU_2
    "夋柺偺僼傿儖僞丂丂: %s",

    // MSG_OPTION_MENU_SHOW_FPS
    "FPS昞\帵 丂丂丂丂丂: %s",

    // MSG_OPTION_MENU_3
    "僼儗乕儉僗僉僢僾丂: %s",

    // MSG_OPTION_MENU_4
    "僗僉僢僾偺抣丂丂丂: %d",

    // MSG_OPTION_MENU_5
    "摦嶌僋儘僢僋丂丂丂: %s",

    // MSG_OPTION_MENU_6
    "僒僂儞僪壒検丂丂丂: %s",

    // MSG_OPTION_MENU_7
    "僗僞僢僋偺嵟揔壔丂: %s",

    // MSG_OPTION_MENU_8
    "BIOS偐傜婲摦傪峴偆: %s",

    // MSG_OPTION_MENU_9
    "僶僢僋傾僢僾偺峏怴: %s",

    // MSG_OPTION_MENU_10
    "尵岅丂丂丂丂丂丂丂: %s",

    // MSG_OPTION_MENU_HELP_DEFAULT
    "愝掕偺弶婜壔",

    // MSG_OPTION_MENU_11
    "栠傞",

    // MSG_OPTION_MENU_HELP_0
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_1
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_2
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_SHOW_FPS
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   僄儈儏儗乕僞",

    // MSG_OPTION_MENU_HELP_3
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_4
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_5
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_6
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   嫟捠",

    // MSG_OPTION_MENU_HELP_7
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   僄儈儏儗乕僞",

    // MSG_OPTION_MENU_HELP_8
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   僄儈儏儗乕僞",

    // MSG_OPTION_MENU_HELP_9
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   僄儈儏儗乕僞",

    // MSG_OPTION_MENU_HELP_10
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰   僄儈儏儗乕僞",

    // MSG_OPTION_MENU_DEFAULT
    "仜:弶婜壔",

    // MSG_OPTION_MENU_HELP_11
    "仜:儊僀儞儊僯儏乕",

    // MSG_STATE_MENU_DATE_FMT_0
    "%4d/%2d/%2d %-4s %2d:%02d:%02d",

    // MSG_STATE_MENU_DATE_FMT_1
    "%2d/%2d/%4d %-4s %2d:%02d:%02d",

    // MSG_STATE_MENU_DATE_NONE_0
    "----/--/-- ---- --:--:--",

    // MSG_STATE_MENU_DATE_NONE_1
    "--/--/---- ---- --:--:--",

    // MSG_STATE_MENU_STATE_NONE
    "偙偺僗儘僢僩偵僨乕僞偼偁傝傑偣傫",

    // MSG_STATE_MENU_TITLE
	#include "text/state_menu_t.h"

    // MSG_STATE_MENU_0
    "",

    // MSG_STATE_MENU_1
    "僼傽僀儖偐傜僗僥乕僩傪儘乕僪",

    // MSG_STATE_MENU_2
    "栠傞",

    // MSG_STATE_MENU_HELP_0
    "仜:幚峴  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰",

    // MSG_STATE_MENU_HELP_1
    "仜:僼傽僀儖僽儔僂僓",

    // MSG_STATE_MENU_HELP_2
    "仜:儊僀儞儊僯儏乕",

    // MSG_PAD_MENU_TITLE
	#include "text/pad_menu_t.h"

    // MSG_PAD_MENU_0
    "仾丂丂 : %s",

    // MSG_PAD_MENU_1
    "伀丂丂 : %s",

    // MSG_PAD_MENU_2
    "仼丂丂 : %s",

    // MSG_PAD_MENU_3
    "仺丂丂 : %s",

    // MSG_PAD_MENU_4
    "仜丂丂 : %s",

    // MSG_PAD_MENU_5
    "亊丂丂 : %s",

    // MSG_PAD_MENU_6
    "仩丂丂 : %s",

    // MSG_PAD_MENU_7
    "仮丂丂 : %s",

    // MSG_PAD_MENU_8
    FONT_L_TRIGGER "丂丂 : %s",

    // MSG_PAD_MENU_9
    FONT_R_TRIGGER "丂丂 : %s",

    // MSG_PAD_MENU_10
    "START  : %s",

    // MSG_PAD_MENU_11
    "SELECT : %s",

    // MSG_PAD_MENU_12
    "栠傞",

    // MSG_PAD_MENU_HELP_0
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰",

    // MSG_PAD_MENU_HELP_1
    "仜:儊僀儞儊僯儏乕",

    //PAD偺愝掕 僐儞僼傿僌

    // MSG_PAD_MENU_CFG_0
    "仾",

    // MSG_PAD_MENU_CFG_1
    "伀",

    // MSG_PAD_MENU_CFG_2
    "仼",

    // MSG_PAD_MENU_CFG_3
    "仺",

    // MSG_PAD_MENU_CFG_4
    "A",

    // MSG_PAD_MENU_CFG_5
    "B",

    // MSG_PAD_MENU_CFG_6
    "L",

    // MSG_PAD_MENU_CFG_7
    "R",

    // MSG_PAD_MENU_CFG_8
    "僗僞乕僩",

    // MSG_PAD_MENU_CFG_9
    "僙儗僋僩",

    // MSG_PAD_MENU_CFG_10
    "儊僯儏乕",

    // MSG_PAD_MENU_CFG_11
    "僞乕儃",

    // MSG_PAD_MENU_CFG_12
    "僗僥乕僩儘乕僪",

    // MSG_PAD_MENU_CFG_13
    "僗僥乕僩僙乕僽",

    // MSG_PAD_MENU_CFG_14
    "A楢幩",

    // MSG_PAD_MENU_CFG_15
    "B楢幩",

    // MSG_PAD_MENU_CFG_16
    "L楢幩",

    // MSG_PAD_MENU_CFG_17
    "R楢幩",

    // MSG_PAD_MENU_CFG_18
    "僼儗乕儉儗乕僩昞\帵",

    // MSG_PAD_MENU_CFG_19
    "側偟",

    // MSG_A_PAD_MENU_TITLE
	#include "text/a_pad_menu_t.h"

    // MSG_A_PAD_MENU_0
    "傾僫儘僌 仾 : %s",

    // MSG_A_PAD_MENU_1
    "傾僫儘僌 伀 : %s",

    // MSG_A_PAD_MENU_2
    "傾僫儘僌 仼 : %s",

    // MSG_A_PAD_MENU_3
    "傾僫儘僌 仺 : %s",

    // MSG_A_PAD_MENU_4
    "傾僫儘僌擖椡傪桳岠: %s",

    // MSG_A_PAD_MENU_5
    "傾僫儘僌擖椡偺姶搙: %d",

    // MSG_A_PAD_MENU_6
    "栠傞",

    // MSG_A_PAD_MENU_HELP_0
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰",

    // MSG_A_PAD_MENU_HELP_1
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰",

    // MSG_A_PAD_MENU_HELP_2
    "仜:儊僀儞儊僯儏乕",

	//MSG_CHEAT_MENU_TITLE,
	#include "text/cheat_menu_t.h"

	//MSG_CHEAT_MENU_NON_LOAD,
	"(柍) %2d: ------------------------",

	//MSG_CHEAT_MENU_0,
	"%%s %2d: %s",

	//MSG_CHEAT_MENU_1,
	"僠乕僩僼傽僀儖偺儘乕僪  " FONT_L_TRIGGER,

	//MSG_CHEAT_MENU_2,
	"栠傞",

	//MSG_CHEAT_MENU_3,
	"僠乕僩儁乕僕: %d",

	//MSG_CHEAT_MENU_HELP_0,
	FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":慖戰偟偨僐乕僪偺桳岠乛柍岠偺愗懼",

	//MSG_CHEAT_MENU_HELP_1,
	"仜:僠乕僩僼傽僀儖傪儘乕僪偟傑偡  " FONT_L_TRIGGER ":僔儑乕僩僇僢僩",

	//MSG_CHEAT_MENU_HELP_2,
	"仜:儊僀儞儊僯儏乕偵栠傝傑偡",

	//MSG_CHEAT_MENU_HELP_3,
	FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":儁乕僕傪曄峏",

    // MSG_NON_LOAD_GAME
    "僎乕儉偼儘乕僪偝傟偰偄傑偣傫",

    // MSG_DAYW_0
    "(擔)",

    // MSG_DAYW_1
    "(寧)",

    // MSG_DAYW_2
    "(壩)",

    // MSG_DAYW_3
    "(悈)",

    // MSG_DAYW_4
    "(栘)",

    // MSG_DAYW_5
    "(嬥)",

    // MSG_DAYW_6
    "(搚)",

    // MSG_YES
    "偼偄",

    // MSG_NO
    "偄偄偊",

    // MSG_ON
    "僆儞",

    // MSG_OFF
    "僆僼",

    // MSG_ENABLED
    "桳岠",

    // MSG_DISABLED
    "柍岠",

    // MSG_AUTO
    "帺摦",

    // MSG_MANUAL
    "庤摦",

    // MSG_EXITONLY
    "廔椆帪偺傒",

    // MSG_LOAD
    "儘乕僪",

    // MSG_SAVE
    "僙乕僽",

    // MSG_SCN_SCALED_NONE
    "100% GU",

    // MSG_SCN_SCALED_X15_GU
    "150% GU",

    // MSG_SCN_SCALED_X15_SW
    "150% SW",

    // MSG_SCN_SCALED_USER
    "巜掕 GU",

    // MSG_LANG_JAPANESE
    "擔杮岅",

    // MSG_LANG_ENGLISH
    "塸岅",

    // MSG_LANG_CHS
    "拞崙岅娙閾",

    // MSG_LANG_CHT
    "拞崙岅斏閾",

    // MSG_SS_DATE_FMT_0
    "%04d_%02d_%02d_%s%02d_%02d_%02d_%03d",

    // MSG_SS_DATE_FMT_1
    "%02d_%02d_%04d_%s%02d_%02d_%02d_%03d",

    // MSG_ERR_SET_DIR_0
    "僄儔乕 [%s] 巜掕偝傟偨僨傿儗僋僩儕偼柍岠偱偡丅",

    // MSG_ERR_SET_DIR_1
    "僄儔乕 [%s] 巜掕偑偁傝傑偣傫丅",

    // MSG_ERR_SET_DIR_2
    "僄儔乕偺敪惗偟偨崁栚偼丄埲壓偺僨傿儗僋僩儕偵愝掕偟傑偡丅\n%s",

    // png.c
    // MSG_ERR_SS_PNG_0
    "儊儌儕偺妋曐偑弌棃傑偣傫偱偟偨丅",

    // MSG_ERR_SS_PNG_1
    "PNG僀儊乕僕偺嶌惉偑弌棃傑偣傫偱偟偨丅",

    // memory.c
    // MSG_LOADING_ROM
    "儘乕僪拞...",

    // MSG_SEARCHING_BACKUP_ID
    "BACKUP ID 傪専嶕拞",

    // main.c
    // MSG_GBA_SLEEP_MODE
    "僗儕乕僾 儌乕僪",

    // MSG_ERR_LOAD_DIR_INI
    "dir.ini偑儘乕僪弌棃傑偣傫丅偡傋偰傪埲壓偺僨傿儗僋僩儕偵愝掕偟傑偡丅\n%s",

    // MSG_ERR_BIOS_NONE
    "BIOS僼傽僀儖偑儘乕僪弌棃傑偣傫丅",

    // MSG_ERR_LOAD_GAMEPACK
    "僎乕儉僼傽僀儖偑儘乕僪弌棃傑偣傫丅",

    // MSG_ERR_OPEN_GAMEPACK
    "僎乕儉僼傽僀儖偑撉傔傑偣傫丅",

    // MSG_ERR_START_CALLBACK_THREAD
    "僐乕儖僶僢僋僗儗僢僪傪奐巒弌棃傑偣傫丅",

    // sound.c
    // MSG_ERR_RESERVE_AUDIO_CHANNEL
    "僆乕僨傿僆僠儍儞僱儖傪妋曐弌棃傑偣傫丅",

    // MSG_ERR_START_SOUND_THEREAD
    "僒僂儞僪僗儗僢僪傪奐巒弌棃傑偣傫丅",

    // MSG_ERR_MALLOC
    "儊儌儕偺妋曐偑弌棃傑偣傫丅",

    // MSG_ERR_CONT
    "壗偐儃僞儞傪墴偟偰偔偩偝偄丅",

    // MSG_ERR_QUIT
    "壗偐儃僞儞傪墴偡偲廔椆偟傑偡丅",

    // MSG_BLANK
    ""
  },
#include "text/message_ansi.h"
};

