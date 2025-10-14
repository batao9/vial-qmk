/*
Copyright 2022 aki27

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include <stdio.h>
#include "quantum.h"


// Defines names for use in layer keycodes and the keymap
enum layer_number {
    _BASE = 0,
    _FN1 = 1,
    _FN2 = 2
};


#define LT_LNG1 LT(_FN1, KC_LNG1)
#define LT_LNG2 LT(_FN2, KC_LNG2)
#define LT_SLSH LT(_FN2, KC_SLSH)

#define KC_FN1 MO(_FN1)
#define KC_FN2 MO(_FN2)


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
      KC_ESC,    KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,              KC_7,    KC_8,    KC_9,    KC_0, KC_MINS,  KC_EQL, KC_BSPC,
      KC_TAB,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                       KC_Y,    KC_U,    KC_I,    KC_O,    KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
     KC_LALT,    KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                       KC_H,    KC_J,    KC_K,    KC_L,          KC_SCLN, KC_QUOT,  KC_ENT,
     KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_V, KC_MINS,  KC_BTN1,          KC_BTN2,    KC_B,    KC_N,    KC_M, KC_COMM,  KC_DOT, LT_SLSH,   KC_UP,
     KC_LCTL, KC_LGUI,  KC_FN2,           KC_SPC, LT_LNG2,           KC_BTN3,          LT_LNG1,  KC_SPC, KC_RALT,          KC_LEFT, KC_DOWN, KC_RGHT,
                                                            KC_WH_D,          KC_WH_U
    ),

    [_FN1] = LAYOUT(
      KC_GRV,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,             KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,  KC_F12,  KC_DEL,
      KC_TAB,    KC_Q,   KC_UP,    KC_E,    KC_R,    KC_T,                      KC_P7,   KC_P8,   KC_P9,    KC_O,    KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
     KC_CAPS, KC_LEFT, KC_DOWN, KC_RGHT,    KC_F,    KC_G,                      KC_P4,   KC_P5,   KC_P6,    KC_L,          KC_SCLN, KC_QUOT,  KC_ENT,
     KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_V, KC_MINS,  KC_BTN4,          KC_BTN5,   KC_P1,   KC_P2,   KC_P3, KC_COMM,  KC_DOT, KC_SLSH, KC_PGUP,
     KC_LCTL, KC_LGUI,  KC_FN2,           KC_SPC, KC_BSPC,           KC_BTN3,            KC_NO,   KC_P0,  KC_NUM,          KC_HOME, KC_PGDN,  KC_END,
                                                            KC_WH_R,          KC_WH_L
    ),

    [_FN2] = LAYOUT(
      KC_GRV,   KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,             KC_F7,   KC_F8,   KC_F9,  KC_F10,  KC_F11,  KC_F12,  KC_DEL,
      KC_TAB,    KC_Q,   KC_UP,    KC_E,    KC_R,    KC_T,                       KC_Y,    KC_U,   KC_UP,    KC_O,    KC_P, KC_LBRC, KC_RBRC, KC_BSLS,
     KC_CAPS, KC_LEFT, KC_DOWN, KC_RGHT,    KC_F,    KC_G,                       KC_H, KC_LEFT, KC_DOWN, KC_RGHT,          KC_SCLN, KC_QUOT,  KC_ENT,
     KC_LSFT,    KC_Z,    KC_X,    KC_C,    KC_V, KC_MINS,  KC_BTN4,          KC_BTN5,    KC_B,    KC_N,    KC_M, KC_COMM,  KC_DOT, KC_SLSH, KC_PGUP,
     KC_LCTL, KC_LGUI,  KC_FN2,           KC_SPC,   KC_NO,           KC_BTN3,           KC_ENT,  KC_SPC, KC_RALT,          KC_HOME, KC_PGDN,  KC_END,
                                                            KC_WH_R,          KC_WH_L
    ),
};

// 現在アクティブなレイヤから [row,col] のキーコードを取得し、タップする
// Vial/VIA の動的キーマップを反映するため dynamic_keymap_get_keycode を使用
static uint16_t keycode_at(uint8_t row, uint8_t col) {
    keypos_t pos = (keypos_t){ .row = row, .col = col };
    layer_state_t st = layer_state;
    // 上位レイヤから透過でないキーを探索
    while (st) {
        uint8_t top = get_highest_layer(st);
        uint16_t kc = dynamic_keymap_get_keycode(top, pos.row, pos.col);
        if (kc != KC_TRNS) return kc;
        st &= ~((layer_state_t)1u << top);
    }
    // 全て透過ならベースレイヤ
    return dynamic_keymap_get_keycode(0, pos.row, pos.col);
}

static void tap_key_at(uint8_t row, uint8_t col) {
    uint16_t kc = keycode_at(row, col);
    if (kc == KC_NO) return;
    tap_code16(kc);
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index != 0) return true;
    tap_key_at(5, clockwise ? 1 : 0);
    return false;
}


layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case _BASE:
        // rgblight_sethsv_range(HSV_BLUE, 0, 2);
        // cocot_set_scroll_mode(false);
        break;
    case _FN1:
        // rgblight_sethsv_range(HSV_RED, 0, 2);
        // cocot_set_scroll_mode(true);
        break;
    case _FN2:
        // rgblight_sethsv_range(HSV_GREEN, 0, 2);
        // cocot_set_scroll_mode(true);
        break;
    default:
        // rgblight_sethsv_range( 0, 0, 0, 0, 2);
        // cocot_set_scroll_mode(false);
        break;
    }
    // rgblight_set_effect_range( 2, 10);
      return state;
};



