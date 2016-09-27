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

#include "main.h"
#include "init.h"
#include "theme.h"
#include "language.h"
#include "utils.h"
#include "settings_dialog.h"
#include "config.h"



static int dialog_mode = SETTINGS_DIALOG_CLOSED;
static SettingsDialog settings_dialog;
static int max_entry_show = 0;
static int base_pos = 0;
static int rel_pos = 0;

static int default_loaded = 0;
static int current_category = SETTINGS_CATEGORY_GENERAL;

static SettingsEntry settings[SETTINGS_ENTRY_COUNT];
static SettingsEntry *current_settings[SETTINGS_ENTRY_COUNT];
static int n_current_settings = 0;

static vita2d_texture **category_icons[SETTINGS_CATEGORY_COUNT];
static char **category_names[SETTINGS_CATEGORY_COUNT];

void setCategory(enum SettingsCategory category) {
    category = MOD(category, SETTINGS_CATEGORY_COUNT);

    current_category = category;
    n_current_settings = 0;

    int i;
    for (i = 0; i < SETTINGS_ENTRY_COUNT; i++)
        if (settings[i].category == category)
            current_settings[n_current_settings++] = &settings[i];

}

void loadDefaults() {
    default_loaded = 1;

    #define SETUP_SETTING_INT(name, category, value) { static uint32_t val = value; SettingsEntry entry = {{#name, CONFIG_TYPE_INT, &val}, &language_container[name##_SETTING], category, SETTINGS_ENTRY_##name}; settings[SETTINGS_ENTRY_##name] = entry; }
    #define SETUP_SETTING_BOOL(name, category, value) { static uint32_t val = value; SettingsEntry entry = {{#name, CONFIG_TYPE_BOOLEAN, &val}, &language_container[name##_SETTING], category, SETTINGS_ENTRY_##name}; settings[SETTINGS_ENTRY_##name] = entry; }
    #define SETUP_SETTING_HEX(name, category, value) { static uint32_t val = value; SettingsEntry entry = {{#name, CONFIG_TYPE_HEXDECIMAL, &val}, &language_container[name##_SETTING], category, SETTINGS_ENTRY_##name}; settings[SETTINGS_ENTRY_##name] = entry; }
    #define SETUP_SETTING_STRING(name, category, value) { static char val[MAX_CONFIG_STRING_LENGTH]; strcpy(val, value); static char *p = val; SettingsEntry entry = {{#name, CONFIG_TYPE_STRING, &p}, &language_container[name##_SETTING], category, SETTINGS_ENTRY_##name}; settings[SETTINGS_ENTRY_##name] = entry; }

    SETUP_SETTING_BOOL(FTP_ENABLED, SETTINGS_CATEGORY_GENERAL, 0);
    SETUP_SETTING_STRING(LANGUAGE, SETTINGS_CATEGORY_GENERAL, getLang(language));
    SETUP_SETTING_STRING(THEME, SETTINGS_CATEGORY_GENERAL, "Default");

    #define SETUP_CATEGORY(name, icon) { category_icons[name] = &icon; category_names[name] = &language_container[name##_TITLE] ;}

    SETUP_CATEGORY(SETTINGS_CATEGORY_GENERAL, settings_general);
    SETUP_CATEGORY(SETTINGS_CATEGORY_FTP, settings_general);

    setCategory(SETTINGS_CATEGORY_GENERAL);
}

int getSettingsDialogMode()
{
	return dialog_mode;
}

void showSettingsDialog() {
    if (!default_loaded)
        loadDefaults();

	base_pos = 0;
	rel_pos = 0;

	settings_dialog.height = SCREEN_HEIGHT - 2 * SETTINGS_MARGIN_Y;
	settings_dialog.width = SCREEN_WIDTH - 2 * SETTINGS_MARGIN_X;

	settings_dialog.x = SETTINGS_MARGIN_X;
	settings_dialog.y = -settings_dialog.height;
    settings_dialog.start_y = -settings_dialog.height;
    settings_dialog.target_y = 0;
    settings_dialog.anim_value = 0.0f;
	settings_dialog.animation_mode = SETTINGS_DIALOG_ANIMATION_OPENING;
	settings_dialog.status = SCE_COMMON_DIALOG_STATUS_RUNNING;

	dialog_mode = SETTINGS_DIALOG_OPEN;
}

static float easeInOut(float a, float start, float end) {
    float f;
    if (a < 0.5) {
        f = 2 * a * a;
    } else {
        f = (-2 * a * a) + (4 * a) - 1;
    }
    return ( 1.0f - f ) * start + f * end;
}

void drawSettingsDialog() {
	if (dialog_mode == SETTINGS_DIALOG_CLOSED)
		return;

    if (!default_loaded)
        loadDefaults();

	// Dialog background
	vita2d_draw_texture_scale_rotate_hotspot(dialog_image, settings_dialog.x + settings_dialog.width / 2.0f,
														settings_dialog.y + settings_dialog.height / 2.0f,
														(settings_dialog.width / vita2d_texture_get_width(dialog_image)),
                                                        (settings_dialog.height / vita2d_texture_get_height(dialog_image)),
														0.0f, vita2d_texture_get_width(dialog_image) / 2.0f, vita2d_texture_get_height(dialog_image) / 2.0f);

	// Easing out
	if (settings_dialog.animation_mode == SETTINGS_DIALOG_ANIMATION_CLOSING) {
		if (settings_dialog.anim_value > 0.0) {
            settings_dialog.anim_value -= 0.05f;
            settings_dialog.y = easeInOut(settings_dialog.anim_value, settings_dialog.start_y, settings_dialog.target_y);
		} else {
            settings_dialog.anim_value = 0.0f;
            settings_dialog.y = settings_dialog.start_y;
			settings_dialog.animation_mode = SETTINGS_DIALOG_ANIMATION_CLOSED;
			settings_dialog.status = SCE_COMMON_DIALOG_STATUS_FINISHED;
		}
	}

    // Easing in
	if (settings_dialog.animation_mode == SETTINGS_DIALOG_ANIMATION_OPENING) {
		if (settings_dialog.anim_value < 1.0) {
            settings_dialog.anim_value += 0.05f;
            settings_dialog.y = easeInOut(settings_dialog.anim_value, settings_dialog.start_y, settings_dialog.target_y);
		} else {
            settings_dialog.anim_value = 1.0f;
            settings_dialog.y = settings_dialog.target_y;
			settings_dialog.animation_mode = SETTINGS_DIALOG_ANIMATION_OPENED;
		}
	}

    if (settings_dialog.animation_mode == SETTINGS_DIALOG_ANIMATION_CLOSED) {
        dialog_mode = SETTINGS_DIALOG_CLOSED;
    }

    float base_x = settings_dialog.x;
    float base_y = settings_dialog.y + SHELL_MARGIN_Y;

    // Draw the title
    {
        int width = vita2d_pgf_text_width(font, FONT_SIZE * 1.3, language_container[SETTINGS_TITLE]);
        int height = vita2d_pgf_text_height(font, FONT_SIZE * 1.3, language_container[SETTINGS_TITLE]);

        // Title background
        vita2d_draw_rectangle(base_x, settings_dialog.y, settings_dialog.width, height + 1.5 * SHELL_MARGIN_Y, 0xFF404040);

        pgf_draw_text(base_x + (settings_dialog.width-width)/2, base_y, TEXT_COLOR, FONT_SIZE * 1.3, language_container[SETTINGS_TITLE]);

        base_y += height + SHELL_MARGIN_Y;
    }

    // Draw the tabs
    {
        int icon_size = vita2d_texture_get_width(settings_general);
        int text_height = 0;
        int tabs_width = SETTINGS_CATEGORY_COUNT * icon_size + (SETTINGS_CATEGORY_COUNT) * SHELL_MARGIN_X * 1.5;

        // Tab background
        vita2d_draw_rectangle(base_x, base_y - SHELL_MARGIN_Y/2, settings_dialog.width, icon_size + SHELL_MARGIN_Y, 0xFF404040);

        // L/R-Trigger
        vita2d_draw_texture(ltrigger, settings_dialog.x + SHELL_MARGIN_X/2, base_y- SHELL_MARGIN_Y/2);
        vita2d_draw_texture(rtrigger, settings_dialog.x + settings_dialog.width - vita2d_texture_get_width(rtrigger) - SHELL_MARGIN_X/2, base_y- SHELL_MARGIN_Y/2);

        int i;
        for (i = 0; i < SETTINGS_CATEGORY_COUNT; i++) {
            tabs_width += vita2d_pgf_text_width(font, FONT_SIZE, *category_names[i]);
            int h = vita2d_pgf_text_height(font, FONT_SIZE, *category_names[i]);
            if (h > text_height)
                text_height = h;
        }
        int x = base_x + (settings_dialog.width-tabs_width)/2;
        int y = base_y;

        for (i = 0; i < SETTINGS_CATEGORY_COUNT; i++) {
            int width = 0;
            vita2d_draw_texture(*category_icons[i], x + SHELL_MARGIN_X/2, base_y);
            width += icon_size + SHELL_MARGIN_X;
            width += pgf_draw_text(x + width, y, TEXT_COLOR, FONT_SIZE, *category_names[i]) + SHELL_MARGIN_X/2;

            if (current_category == i) {
                vita2d_draw_rectangle(x, base_y - SHELL_MARGIN_Y/2, width, icon_size + SHELL_MARGIN_Y, 0x4F7F7F7F);
            }

            x+= width;
        }

        base_y += icon_size;
    }

    // Draw the entries
    int i;
    for (i = 0; i < LIST_VISIBLE_ENTRY; i++) {
        if (base_pos + i >= n_current_settings) break;

        SettingsEntry *entry = current_settings[base_pos + i];

        float x = base_x;
        float y = base_y + SHELL_MARGIN_Y + (i * FONT_Y_SPACE);

        if (i == rel_pos) {
            vita2d_draw_rectangle(x, y, settings_dialog.width - 2, FONT_Y_SPACE, MARKED_COLOR);
        }

        pgf_draw_text(x + SHELL_MARGIN_X, y, (rel_pos == i) ? TEXT_FOCUS_COLOR : TEXT_COLOR, FONT_SIZE, *entry->display_name);

        char value[MAX_CONFIG_STRING_LENGTH];
        switch (entry->configEntry.type) {
            case CONFIG_TYPE_STRING:
                strcpy(value, *(char**)entry->configEntry.value);
                break;

            case CONFIG_TYPE_HEXDECIMAL:
                itoa(*(int*)entry->configEntry.value, value, 16);
                break;

            case CONFIG_TYPE_DECIMAL:
                itoa(*(int*)entry->configEntry.value, value, 10);
                break;

            case CONFIG_TYPE_BOOLEAN:
                strcpy(value, *(int*)entry->configEntry.value != 0 ? language_container[YES]:language_container[NO]);
                break;
        }

        float width = vita2d_pgf_text_width(font, FONT_SIZE, value);
        pgf_draw_text(settings_dialog.x + settings_dialog.width - SHELL_MARGIN_X - width, y, (rel_pos == i) ? TEXT_FOCUS_COLOR : TEXT_COLOR, FONT_SIZE, value);
    }
}

void applySetting(SettingsEntry *entry) {
    switch (entry->entry) {
        case SETTINGS_ENTRY_FTP_ENABLED:
            setFtpEnabled(*(uint32_t*)entry->configEntry.value);
            break;

        case SETTINGS_ENTRY_LANGUAGE:
            loadLanguage(language);
            break;

        case SETTINGS_ENTRY_THEME:
            loadTheme(*(char **)entry->configEntry.value);
            break;

        default:  break;
    }
}

void loadSettings() {
    if (!default_loaded) {
        loadDefaults();
    }

    ConfigEntry config_entries[SETTINGS_ENTRY_COUNT];

    int i;
    for (i = 0; i < SETTINGS_ENTRY_COUNT; i++) {
        memcpy(&config_entries[i], &settings[i].configEntry, sizeof(ConfigEntry));
    }

    readConfig("ux0:/VitaShell/settings.txt", config_entries, SETTINGS_ENTRY_COUNT);

    for (i = 0; i < SETTINGS_ENTRY_COUNT; i++) {
        memcpy(&settings[i].configEntry, &config_entries[i], sizeof(ConfigEntry));
        applySetting(&settings[i]);
    }
}

void saveSettings() {
    if (!default_loaded)
        loadDefaults();

    ConfigEntry config_entries[SETTINGS_ENTRY_COUNT];

    int i;
    for (i = 0; i < SETTINGS_ENTRY_COUNT; i++) {
        config_entries[i] = settings[i].configEntry;
    }

    writeConfig("ux0:/VitaShell/settings.txt", config_entries, SETTINGS_ENTRY_COUNT);
}

void handleCtrlEntryDefault() {
    SettingsEntry *entry = current_settings[base_pos+rel_pos];
    switch(entry->configEntry.type) {
        case CONFIG_TYPE_BOOLEAN:
            if (pressed_buttons & SCE_CTRL_LEFT || pressed_buttons & SCE_CTRL_RIGHT || pressed_buttons & SCE_CTRL_ENTER) {
                *(int*)entry->configEntry.value = !*(int*)entry->configEntry.value;
                applySetting(entry);
            }
            break;

        case CONFIG_TYPE_DECIMAL:
        case CONFIG_TYPE_HEXDECIMAL:
            if (pressed_buttons & SCE_CTRL_LEFT) {
                (*(int*)entry->configEntry.value)--;
                applySetting(entry);
            }
            if (pressed_buttons & SCE_CTRL_RIGHT) {
                (*(int*)entry->configEntry.value)++;
                applySetting(entry);
            }
            break;

        case CONFIG_TYPE_STRING:
            break;
    }
}

static int compare_dirent(const void *a, const void *b) {
    return strcmp(((SceIoDirent*)a)->d_name, ((SceIoDirent*)b)->d_name);
}

void handleCtrlEntry() {
    SettingsEntry *entry = current_settings[base_pos+rel_pos];
    switch (entry->entry) {
        int i;
        SceUID theme_dir_fd;
        struct SceIoDirent theme_dirs[512];
        int n_theme_dirs;
        int cur_theme_index;

        case SETTINGS_ENTRY_LANGUAGE:
            if (pressed_buttons & SCE_CTRL_LEFT) {
                language = MOD(language - 1, getLangCount());
                strcpy(*(char**)entry->configEntry.value, getLang(language));
                applySetting(entry);
            } else if (pressed_buttons & SCE_CTRL_RIGHT) {
                language = MOD(language + 1, getLangCount());
                strcpy(*(char **) entry->configEntry.value, getLang(language));
                applySetting(entry);
            }
            break;

        case SETTINGS_ENTRY_THEME:
            if (pressed_buttons & SCE_CTRL_LEFT || pressed_buttons & SCE_CTRL_RIGHT) {
                theme_dir_fd = sceIoDopen("ux0:VitaShell/theme/");
                if (theme_dir_fd) {
                    n_theme_dirs = 0;
                    while(sceIoDread(theme_dir_fd, &theme_dirs[n_theme_dirs])) {

                        // Filter only dirs
                        if (SCE_S_ISDIR(theme_dirs[n_theme_dirs].d_stat.st_mode)) {
                            n_theme_dirs++;
                        }
                    }

                    // sort results
                    qsort(theme_dirs, n_theme_dirs, sizeof(SceIoDirent), &compare_dirent);

                    cur_theme_index = 0;
                    // find current theme
                    for (i = 0; i < n_theme_dirs; i++) {
                        if (strcmp(theme_dirs[i].d_name, getCurrentTheme()) == 0) {
                            cur_theme_index = i;
                        }
                    }

                    if (pressed_buttons & SCE_CTRL_LEFT) {
                        cur_theme_index--;
                    }
                    else if (pressed_buttons & SCE_CTRL_RIGHT) {
                        cur_theme_index++;
                    }

                    cur_theme_index = n_theme_dirs > 0 ? MOD(cur_theme_index, n_theme_dirs) : 0;

                    strcpy(*(char**)entry->configEntry.value, theme_dirs[cur_theme_index].d_name);

                    sceIoDclose(theme_dir_fd);

                    applySetting(entry);
                }
            }
            break;

        default:
            handleCtrlEntryDefault();
            break;
    }
}

void settingsDialogCtrl() {
	if (dialog_mode == SETTINGS_DIALOG_CLOSED)
		return;

	if ((pressed_buttons & (SCE_CTRL_CANCEL | SCE_CTRL_SELECT))) {
        settings_dialog.animation_mode = SETTINGS_DIALOG_ANIMATION_CLOSING;
        saveSettings();
    } else if (hold_buttons & SCE_CTRL_UP || hold2_buttons & SCE_CTRL_LEFT_ANALOG_UP) {
        if (rel_pos > 0) {
            rel_pos--;
        } else {
            if (base_pos > 0) {
                base_pos--;
            }
        }
    } else if (hold_buttons & SCE_CTRL_DOWN || hold2_buttons & SCE_CTRL_LEFT_ANALOG_DOWN) {
        if ((rel_pos + 1) < SETTINGS_ENTRY_COUNT) {
            if ((rel_pos + 1) < LIST_MAX_ENTRY) {
                rel_pos++;
            } else {
                if ((base_pos + rel_pos + 1) < SETTINGS_ENTRY_COUNT) {
                    base_pos++;
                }
            }
        }
    } else if (pressed_buttons & SCE_CTRL_LTRIGGER) {
        setCategory((enum SettingsCategory) (current_category - 1));
    } else if (pressed_buttons & SCE_CTRL_RTRIGGER) {
        setCategory((enum SettingsCategory) (current_category + 1));
    } else {
        handleCtrlEntry();
    }
}
