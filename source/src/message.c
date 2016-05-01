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
    "○:選択  ×:メニュー  □:" FONT_UP_DIRECTORY,

    // MSG_MENU_DATE_FMT_0
    "%4d/%2d/%2d %-4s %2d:%02d",

    // MSG_MENU_DATE_FMT_1
    "%2d/%2d/%4d %-4s %2d:%02d",

    // MSG_MAIN_MENU_TITLE
	#include "text/main_menu_t.h"

    // MSG_MAIN_MENU_0
    "ロードステート : スロット %d",

    // MSG_MAIN_MENU_1
    "セーブステート : スロット %d",

    // MSG_MAIN_MENU_2
    "ステートセーブ拡張 " FONT_R_TRIGGER,

    // MSG_MAIN_MENU_3
    "スクリーン ショット: %s",

    // MSG_MAIN_MENU_4
    "エミュレータの設定",

    // MSG_MAIN_MENU_5
    "ゲームパッドの設定",

    // MSG_MAIN_MENU_6
    "アナログスティックの設定",

	//MSG_MAIN_MENU_CHEAT
    "チートメニュー",

    // MSG_MAIN_MENU_7
    "ゲームのロード " FONT_L_TRIGGER,

    // MSG_MAIN_MENU_8
    "リセット",

    // MSG_MAIN_MENU_9
    "ゲームに戻る",

    // MSG_MAIN_MENU_10
    "スリープ",

    // MSG_MAIN_MENU_11
    "TempGBAの終了",

    // MSG_MAIN_MENU_HELP_0
    "○:ロード  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":スロット選択",

    // MSG_MAIN_MENU_HELP_1
    "○:セーブ  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":スロット選択",

    // MSG_MAIN_MENU_HELP_2
    "○:サブメニュー  " FONT_R_TRIGGER ":ショートカット",

    // MSG_MAIN_MENU_HELP_3
    "○:イメージ保存  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":フォーマット選択",

    // MSG_MAIN_MENU_HELP_4
    "○:サブメニュー",

    // MSG_MAIN_MENU_HELP_5
    "○:サブメニュー",

    // MSG_MAIN_MENU_HELP_6
    "○:サブメニュー",

    // MSG_MAIN_MENU_HELP_CHEAT
    "○:チートを変更します",

    // MSG_MAIN_MENU_HELP_7
    "○:ファイルブラウザ  " FONT_L_TRIGGER ":ショートカット",

    // MSG_MAIN_MENU_HELP_8
    "○:ゲームをリセット",

    // MSG_MAIN_MENU_HELP_9
    "○:ゲームに戻る",

    // MSG_MAIN_MENU_HELP_10
    "○:スリープ モード",

    // MSG_MAIN_MENU_HELP_11
    "○:終了",

    // MSG_OPTION_MENU_TITLE
	#include "text/option_menu_t.h"

    // MSG_OPTION_MENU_0
    "画面表\示　　　　　: %s",

    // MSG_OPTION_MENU_1
    "画面の拡大率　　　: %d%%",

    // MSG_OPTION_MENU_2
    "画面のフィルタ　　: %s",

    // MSG_OPTION_MENU_SHOW_FPS
    "FPS表\示 　　　　　: %s",

    // MSG_OPTION_MENU_3
    "フレームスキップ　: %s",

    // MSG_OPTION_MENU_4
    "スキップの値　　　: %d",

    // MSG_OPTION_MENU_5
    "動作クロック　　　: %s",

    // MSG_OPTION_MENU_6
    "サウンド音量　　　: %s",

    // MSG_OPTION_MENU_7
    "スタックの最適化　: %s",

    // MSG_OPTION_MENU_8
    "BIOSから起動を行う: %s",

    // MSG_OPTION_MENU_9
    "バックアップの更新: %s",

    // MSG_OPTION_MENU_10
    "言語　　　　　　　: %s",

    // MSG_OPTION_MENU_HELP_DEFAULT
    "設定の初期化",

    // MSG_OPTION_MENU_11
    "戻る",

    // MSG_OPTION_MENU_HELP_0
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_1
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_2
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_SHOW_FPS
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   エミュレータ",

    // MSG_OPTION_MENU_HELP_3
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_4
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_5
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_6
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   共通",

    // MSG_OPTION_MENU_HELP_7
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   エミュレータ",

    // MSG_OPTION_MENU_HELP_8
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   エミュレータ",

    // MSG_OPTION_MENU_HELP_9
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   エミュレータ",

    // MSG_OPTION_MENU_HELP_10
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択   エミュレータ",

    // MSG_OPTION_MENU_DEFAULT
    "○:初期化",

    // MSG_OPTION_MENU_HELP_11
    "○:メインメニュー",

    // MSG_STATE_MENU_DATE_FMT_0
    "%4d/%2d/%2d %-4s %2d:%02d:%02d",

    // MSG_STATE_MENU_DATE_FMT_1
    "%2d/%2d/%4d %-4s %2d:%02d:%02d",

    // MSG_STATE_MENU_DATE_NONE_0
    "----/--/-- ---- --:--:--",

    // MSG_STATE_MENU_DATE_NONE_1
    "--/--/---- ---- --:--:--",

    // MSG_STATE_MENU_STATE_NONE
    "このスロットにデータはありません",

    // MSG_STATE_MENU_TITLE
	#include "text/state_menu_t.h"

    // MSG_STATE_MENU_0
    "",

    // MSG_STATE_MENU_1
    "ファイルからステートをロード",

    // MSG_STATE_MENU_2
    "戻る",

    // MSG_STATE_MENU_HELP_0
    "○:実行  " FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択",

    // MSG_STATE_MENU_HELP_1
    "○:ファイルブラウザ",

    // MSG_STATE_MENU_HELP_2
    "○:メインメニュー",

    // MSG_PAD_MENU_TITLE
	#include "text/pad_menu_t.h"

    // MSG_PAD_MENU_0
    "↑　　 : %s",

    // MSG_PAD_MENU_1
    "↓　　 : %s",

    // MSG_PAD_MENU_2
    "←　　 : %s",

    // MSG_PAD_MENU_3
    "→　　 : %s",

    // MSG_PAD_MENU_4
    "○　　 : %s",

    // MSG_PAD_MENU_5
    "×　　 : %s",

    // MSG_PAD_MENU_6
    "□　　 : %s",

    // MSG_PAD_MENU_7
    "△　　 : %s",

    // MSG_PAD_MENU_8
    FONT_L_TRIGGER "　　 : %s",

    // MSG_PAD_MENU_9
    FONT_R_TRIGGER "　　 : %s",

    // MSG_PAD_MENU_10
    "START  : %s",

    // MSG_PAD_MENU_11
    "SELECT : %s",

    // MSG_PAD_MENU_12
    "戻る",

    // MSG_PAD_MENU_HELP_0
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択",

    // MSG_PAD_MENU_HELP_1
    "○:メインメニュー",

    //PADの設定 コンフィグ

    // MSG_PAD_MENU_CFG_0
    "↑",

    // MSG_PAD_MENU_CFG_1
    "↓",

    // MSG_PAD_MENU_CFG_2
    "←",

    // MSG_PAD_MENU_CFG_3
    "→",

    // MSG_PAD_MENU_CFG_4
    "A",

    // MSG_PAD_MENU_CFG_5
    "B",

    // MSG_PAD_MENU_CFG_6
    "L",

    // MSG_PAD_MENU_CFG_7
    "R",

    // MSG_PAD_MENU_CFG_8
    "スタート",

    // MSG_PAD_MENU_CFG_9
    "セレクト",

    // MSG_PAD_MENU_CFG_10
    "メニュー",

    // MSG_PAD_MENU_CFG_11
    "ターボ",

    // MSG_PAD_MENU_CFG_12
    "ステートロード",

    // MSG_PAD_MENU_CFG_13
    "ステートセーブ",

    // MSG_PAD_MENU_CFG_14
    "A連射",

    // MSG_PAD_MENU_CFG_15
    "B連射",

    // MSG_PAD_MENU_CFG_16
    "L連射",

    // MSG_PAD_MENU_CFG_17
    "R連射",

    // MSG_PAD_MENU_CFG_18
    "フレームレート表\示",

    // MSG_PAD_MENU_CFG_19
    "なし",

    // MSG_A_PAD_MENU_TITLE
	#include "text/a_pad_menu_t.h"

    // MSG_A_PAD_MENU_0
    "アナログ ↑ : %s",

    // MSG_A_PAD_MENU_1
    "アナログ ↓ : %s",

    // MSG_A_PAD_MENU_2
    "アナログ ← : %s",

    // MSG_A_PAD_MENU_3
    "アナログ → : %s",

    // MSG_A_PAD_MENU_4
    "アナログ入力を有効: %s",

    // MSG_A_PAD_MENU_5
    "アナログ入力の感度: %d",

    // MSG_A_PAD_MENU_6
    "戻る",

    // MSG_A_PAD_MENU_HELP_0
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択",

    // MSG_A_PAD_MENU_HELP_1
    FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択",

    // MSG_A_PAD_MENU_HELP_2
    "○:メインメニュー",

	//MSG_CHEAT_MENU_TITLE,
	#include "text/cheat_menu_t.h"

	//MSG_CHEAT_MENU_NON_LOAD,
	"(無) %2d: ------------------------",

	//MSG_CHEAT_MENU_0,
	"%%s %2d: %s",

	//MSG_CHEAT_MENU_1,
	"チートファイルのロード  " FONT_L_TRIGGER,

	//MSG_CHEAT_MENU_2,
	"戻る",

	//MSG_CHEAT_MENU_3,
	"チートページ: %d",

	//MSG_CHEAT_MENU_HELP_0,
	FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":選択したコードの有効／無効の切替",

	//MSG_CHEAT_MENU_HELP_1,
	"○:チートファイルをロードします  " FONT_L_TRIGGER ":ショートカット",

	//MSG_CHEAT_MENU_HELP_2,
	"○:メインメニューに戻ります",

	//MSG_CHEAT_MENU_HELP_3,
	FONT_CURSOR_LEFT FONT_CURSOR_RIGHT ":ページを変更",

    // MSG_NON_LOAD_GAME
    "ゲームはロードされていません",

    // MSG_DAYW_0
    "(日)",

    // MSG_DAYW_1
    "(月)",

    // MSG_DAYW_2
    "(火)",

    // MSG_DAYW_3
    "(水)",

    // MSG_DAYW_4
    "(木)",

    // MSG_DAYW_5
    "(金)",

    // MSG_DAYW_6
    "(土)",

    // MSG_YES
    "はい",

    // MSG_NO
    "いいえ",

    // MSG_ON
    "オン",

    // MSG_OFF
    "オフ",

    // MSG_ENABLED
    "有効",

    // MSG_DISABLED
    "無効",

    // MSG_AUTO
    "自動",

    // MSG_MANUAL
    "手動",

    // MSG_EXITONLY
    "終了時のみ",

    // MSG_LOAD
    "ロード",

    // MSG_SAVE
    "セーブ",

    // MSG_SCN_SCALED_NONE
    "100% GU",

    // MSG_SCN_SCALED_X15_GU
    "150% GU",

    // MSG_SCN_SCALED_X15_SW
    "150% SW",

    // MSG_SCN_SCALED_USER
    "指定 GU",

    // MSG_LANG_JAPANESE
    "日本語",

    // MSG_LANG_ENGLISH
    "英語",

    // MSG_LANG_CHS
    "中国語簡體",

    // MSG_LANG_CHT
    "中国語繁體",

    // MSG_SS_DATE_FMT_0
    "%04d_%02d_%02d_%s%02d_%02d_%02d_%03d",

    // MSG_SS_DATE_FMT_1
    "%02d_%02d_%04d_%s%02d_%02d_%02d_%03d",

    // MSG_ERR_SET_DIR_0
    "エラー [%s] 指定されたディレクトリは無効です。",

    // MSG_ERR_SET_DIR_1
    "エラー [%s] 指定がありません。",

    // MSG_ERR_SET_DIR_2
    "エラーの発生した項目は、以下のディレクトリに設定します。\n%s",

    // png.c
    // MSG_ERR_SS_PNG_0
    "メモリの確保が出来ませんでした。",

    // MSG_ERR_SS_PNG_1
    "PNGイメージの作成が出来ませんでした。",

    // memory.c
    // MSG_LOADING_ROM
    "ロード中...",

    // MSG_SEARCHING_BACKUP_ID
    "BACKUP ID を検索中",

    // main.c
    // MSG_GBA_SLEEP_MODE
    "スリープ モード",

    // MSG_ERR_LOAD_DIR_INI
    "dir.iniがロード出来ません。すべてを以下のディレクトリに設定します。\n%s",

    // MSG_ERR_BIOS_NONE
    "BIOSファイルがロード出来ません。",

    // MSG_ERR_LOAD_GAMEPACK
    "ゲームファイルがロード出来ません。",

    // MSG_ERR_OPEN_GAMEPACK
    "ゲームファイルが読めません。",

    // MSG_ERR_START_CALLBACK_THREAD
    "コールバックスレッドを開始出来ません。",

    // sound.c
    // MSG_ERR_RESERVE_AUDIO_CHANNEL
    "オーディオチャンネルを確保出来ません。",

    // MSG_ERR_START_SOUND_THEREAD
    "サウンドスレッドを開始出来ません。",

    // MSG_ERR_MALLOC
    "メモリの確保が出来ません。",

    // MSG_ERR_CONT
    "何かボタンを押してください。",

    // MSG_ERR_QUIT
    "何かボタンを押すと終了します。",

    // MSG_BLANK
    ""
  },
#include "text/message_ansi.h"
};

