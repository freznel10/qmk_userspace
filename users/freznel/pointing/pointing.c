// Copyright 2021 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// Copyright 2022 Freznel B. Sta. Ana  (@freznel10) <freznel@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pointing.h"
#include <stdlib.h>
#ifdef HAPTIC_ENABLE
#include "drivers/haptic/drv2605l.h"
#    define POINTING_HAPTIC_PULSE() drv2605l_pulse(DRV2605L_EFFECT_MEDIUM_CLICK_1_100)
#else
#    define POINTING_HAPTIC_PULSE() ((void)0)
#endif

bool            enable_acceleration = false;
static bool     APP_ALT, APP_WIN;

#ifdef TAPPING_TERM_PER_KEY
#    define TAP_CHECK get_tapping_term(KC_BTN1, NULL)
#else
#    ifndef TAPPING_TERM
#        define TAPPING_TERM 200
#    endif
#    define TAP_CHECK TAPPING_TERM
#endif

__attribute__((weak)) void pointing_device_init_keymap(void) {}

void pointing_device_init_user(void) {
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    set_auto_mouse_enable(true);
#endif
    pointing_device_init_keymap();
}

__attribute__((weak)) report_mouse_t pointing_device_task_keymap(report_mouse_t mouse_report) {
    return mouse_report;
}


// Divisor callback (new API: mouse_xy_report_t return, PMD_* direction constants)
mouse_xy_report_t pointing_modes_get_divisor_user(uint8_t mode_id, uint8_t direction) {
    switch (mode_id) {
        case PM_BROW:
            return (direction & PMD_VERT) ? 128 : 64;
        case PM_RGB_MODE_VAL:
            return (direction & PMD_VERT) ? 64 : 128;
        case PM_RGB_HUE_SAT:
            switch (direction) {
                case PMD_DOWN:  return 32;
                case PMD_UP:    return 64;
                case PMD_LEFT:  return 16;
                case PMD_RIGHT: return 128;
            }
            break;
        case PM_RGB_SPEED:      return 64;
        case PM_WINDOW:         return 128;
        case PM_SWITCHER:       return 64;
        case PM_CUR_ACCEL:      return 8;
        case PM_APP_2:          return 64;
        case PM_BROWSER_CONTROL: return 64;
        case PM_WIN_POS:        return 128;
        case PM_PRECISION:      return 8;
    }
    return 0; // let default processing continue
}

#define CONSTRAIN_XY(value) ((value) > XY_REPORT_MAX ? XY_REPORT_MAX : (value) < XY_REPORT_MIN ? XY_REPORT_MIN : (value))

// Custom task callback for non-map modes (new API signature)
bool pointing_modes_task_user(report_mouse_t* mouse_report, pointing_modes_residuals_t* residuals) {
    switch (pointing_modes_get_mode()) {
        case PM_PRECISION: {
            mouse_xy_report_t divisor = pointing_modes_get_divisor();
            if (divisor) {
                mouse_report->x += (mouse_xy_report_t)(residuals->x / divisor);
                residuals->x     = residuals->x % divisor;
                mouse_report->y += (mouse_xy_report_t)(residuals->y / divisor);
                residuals->y     = residuals->y % divisor;
            }
            pointing_modes_set_residuals(*residuals);
            return false;
        }
        case PM_CUR_ACCEL: {
            *mouse_report = pointing_device_get_report();
            mouse_xy_report_t divisor = pointing_modes_get_divisor();
            if (divisor) {
                mouse_xy_report_t dx = residuals->x / divisor;
                mouse_xy_report_t dy = residuals->y / divisor;
#ifdef POINTING_DEVICE_INVERT_H
                mouse_report->x = CONSTRAIN_XY(mouse_report->x - dx);
#else
                mouse_report->x = CONSTRAIN_XY(mouse_report->x + dx);
#endif
#ifdef POINTING_DEVICE_INVERT_V
                mouse_report->y = CONSTRAIN_XY(mouse_report->y - dy);
#else
                mouse_report->y = CONSTRAIN_XY(mouse_report->y + dy);
#endif
                residuals->x -= dx * divisor;
                residuals->y -= dy * divisor;
            }
            pointing_modes_set_residuals(*residuals);
            return false;
        }
        case PM_APP_2: {
            mouse_xy_report_t divisor = pointing_modes_get_divisor();
            if (abs(residuals->x) >= divisor && !APP_ALT) {
                register_code(KC_LALT);
                APP_ALT = true;
            }
            mouse_xy_report_t moved = pointing_modes_apply_divisor(PM_H_AXIS);
            if (moved > 0) tap_code16(KC_TAB);
            else if (moved < 0) tap_code16(S(KC_TAB));
            return false;
        }
        case PM_WIN_POS: {
            mouse_xy_report_t divisor = pointing_modes_get_divisor();
            if (abs(residuals->x) >= divisor && !APP_WIN) {
                register_code(KC_LGUI);
                APP_WIN = true;
            }
            // Original pointing_tap_codes(KC_LEFT, KC_DOWN, KC_UP, KC_RIGHT)
            // = UP→KC_LEFT, DOWN→KC_DOWN, LEFT→KC_UP, RIGHT→KC_RIGHT
            uint8_t dir = pointing_modes_get_direction();
            mouse_xy_report_t moved_h = pointing_modes_apply_divisor(PM_H_AXIS);
            mouse_xy_report_t moved_v = pointing_modes_apply_divisor(PM_V_AXIS);
            if (moved_h > 0) tap_code16(KC_RIGHT);
            else if (moved_h < 0) tap_code16(KC_UP);
            if (moved_v > 0) tap_code16(KC_DOWN);
            else if (moved_v < 0) tap_code16(KC_LEFT);
            (void)dir;
            return false;
        }
        default:
            break;
    }
    return true; // let map / default processing continue
}

bool process_record_pointing(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case KC_ACCEL:
            enable_acceleration = record->event.pressed;
            break;
        case TD_DRGS:
            POINTING_HAPTIC_PULSE();
            break;
        case KC_BTN1:
            POINTING_HAPTIC_PULSE();
            break;
        case KB_MO_APP:
            if (!record->event.pressed && APP_ALT) {
                unregister_code(KC_LALT);
                APP_ALT = false;
            }
            pointing_modes_key_momentary(PM_APP_2, record->event.pressed);
            break;
        case KB_TG_ACCEL:
            pointing_modes_key_toggle(PM_CUR_ACCEL, record->event.pressed);
            break;
        case KB_MO_WINDOW:
            if (!record->event.pressed && APP_WIN) {
                unregister_code(KC_LGUI);
                APP_WIN = false;
            }
            pointing_modes_key_momentary(PM_WIN_POS, record->event.pressed);
            break;
#if defined(SPLIT_POINTING_ENABLE) && defined(POINTING_DEVICE_COMBINED)
        case PM_SWITCH:
            if (record->event.pressed) {
                uint8_t next = (pointing_modes_get_active_device() == PM_LEFT_DEVICE) ? PM_RIGHT_DEVICE : PM_LEFT_DEVICE;
                pointing_modes_set_active_device(next);
            }
            break;
#endif
        case PMR_DRAG:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                pointing_modes_set_active_device(PM_RIGHT_DEVICE);
                pointing_modes_toggle_mode(PM_DRAG);
            }
            break;
        case PML_DRAG:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                pointing_modes_set_active_device(PM_LEFT_DEVICE);
                pointing_modes_toggle_mode(PM_DRAG);
            }
            break;
        case PMR_VOL:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                pointing_modes_set_active_device(PM_RIGHT_DEVICE);
                pointing_modes_toggle_mode(PM_VOL);
            }
            break;
        case PML_VOL:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                pointing_modes_set_active_device(PM_LEFT_DEVICE);
                pointing_modes_toggle_mode(PM_VOL);
            }
            break;
        case ROUTE:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_X);
            }
            break;
        case ROTATE:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_R);
            }
            break;
        case DRAG_TRACKS:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_D);
            }
            break;
        case PLACE_VIA:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_V);
            }
            break;
        case TRACK_WIDTH:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_W);
            }
            break;
        case VIA_WIDTH:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_BACKSLASH);
            }
            break;
        case TRACK_POSTURE:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code(KC_SLASH);
            }
            break;
        case TRACK_CORNER_MODE:
            if (record->event.pressed) {
                POINTING_HAPTIC_PULSE();
                tap_code16(C(KC_SLASH));
            }
            break;
        default:
            break;
    }
    return true;
}

#if defined(POINTING_DEVICE_AUTO_MOUSE_ENABLE)
bool is_mouse_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case KC_ACCEL:
        case TD_DRGS:
        case TD_DRGR:
        case BK_TAB:
        case NX_TAB:
        case TD_PMD1:
        case ST_MACRO_6:
        case KB_MO_WINDOW:
        case KB_MO_APP:
        case ROUTE:
        case ROTATE:
        case DRAG_TRACKS:
        case PLACE_VIA:
        case TRACK_WIDTH:
        case VIA_WIDTH:
        case TRACK_POSTURE:
        case TRACK_CORNER_MODE:
            return true;
    }
    return false;
}
#endif
