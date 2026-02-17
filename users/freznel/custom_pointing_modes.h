// Copyright 2026 Freznel B. Sta. Ana (@freznel10)
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Custom pointing mode IDs extending the official PM_SAFE_RANGE.
// Map-based modes must come first (IDs starting at POINTING_MODES_MAP_START).
// Non-map (custom task) modes are placed after the last map-based mode.
enum custom_pointing_mode_ids {
    // --- Map-based modes (pointing_modes_maps[] entry per mode, index = id - POINTING_MODES_MAP_START) ---
    PM_BROW             = POINTING_MODES_MAP_START, // Browser tab navigation       [map index 0]
    PM_RGB_MODE_VAL,                                // RGB mode / brightness         [map index 1]
    PM_RGB_HUE_SAT,                                 // RGB hue / saturation          [map index 2]
    PM_RGB_SPEED,                                   // RGB speed                     [map index 3]
    PM_WINDOW,                                      // Window management             [map index 4]
    PM_SWITCHER,                                    // App switcher (Win)            [map index 5]
    PM_BROWSER_CONTROL,                             // Browser back / forward        [map index 6]
    PM_CARET,                                       // Arrow keys / cursor move      [map index 7]
    PM_HISTORY,                                     // Undo / Redo                   [map index 8]
    PM_VOL,                                         // Volume                        [map index 9]
    // --- Custom task modes (handled in pointing_modes_task_user, return false) ---
    PM_APP_2,                                       // App browsing with held ALT
    PM_WIN_POS,                                     // Window positioning with GUI
    PM_CUR_ACCEL,                                   // Cursor speed boost
    PM_PRECISION,                                   // Precision / slow cursor
};

// Map index enum matching the order above (for clarity when editing pointing_modes_maps[]).
enum custom_pointing_modes_map_index {
    _PM_BROW = 0,
    _PM_RGB_MODE_VAL,
    _PM_RGB_HUE_SAT,
    _PM_RGB_SPEED,
    _PM_WINDOW,
    _PM_SWITCHER,
    _PM_BROWSER_CONTROL,
    _PM_CARET,
    _PM_HISTORY,
    _PM_VOL,
};
