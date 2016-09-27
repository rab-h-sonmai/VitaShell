/*
	VitaShell
	Copyright (C) 2015-2016, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SETTINGS_DIALOG_H__
#define __SETTINGS_DIALOG_H__


#define SETTINGS_MARGIN_X 200
#define SETTINGS_MARGIN_Y 50

#define CENTER(a, b) (((a) - (b)) / 2)
#define LIST_MAX_ENTRY 5
#define LIST_VISIBLE_ENTRY 5

#define UP_ARROW "\xe2\x96\xb2"
#define DOWN_ARROW "\xe2\x96\xbc"

#include "config.h"

typedef struct
{
	int animation_mode;
	int status;
	float x;
	float y;
	float width;
	float height;
	float target_y;
	float start_y;
    float anim_value;
} SettingsDialog;

enum SettingsDialogState
{
	SETTINGS_DIALOG_OPEN,
	SETTINGS_DIALOG_CLOSED
};

enum SettingsCategory {
    SETTINGS_CATEGORY_GENERAL,
    SETTINGS_CATEGORY_FTP,

    SETTINGS_CATEGORY_COUNT,
};

enum SettingsEntry {
    SETTINGS_ENTRY_LANGUAGE,
    SETTINGS_ENTRY_THEME,
    SETTINGS_ENTRY_FTP_ENABLED,

    SETTINGS_ENTRY_COUNT,
};

typedef struct
{
    ConfigEntry configEntry;
    char **display_name;
    enum SettingsCategory category;
    enum SettingsEntry entry;
} SettingsEntry;

enum SettingsAnimationMode {
	SETTINGS_DIALOG_ANIMATION_CLOSED,
	SETTINGS_DIALOG_ANIMATION_CLOSING,
	SETTINGS_DIALOG_ANIMATION_OPENED,
	SETTINGS_DIALOG_ANIMATION_OPENING,
};

int getSettingsDialogMode();
void drawSettingsDialog();
void settingsDialogCtrl();
void showSettingsDialog();
void loadSettings();
void saveSettings();

#endif