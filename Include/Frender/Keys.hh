// This was taken from GLFW and modified for Frender

/*************************************************************************
 * Copyright (c) 2002-2006 Marcus Geelnard
 * Copyright (c) 2006-2019 Camilla Löwy <elmindreda@glfw.org>
 * Copyright (c) 2021      Finn Bainbridge
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

/* The unknown key */
#define FRENDER_KEY_UNKNOWN            -1

/* Printable keys */
#define FRENDER_KEY_SPACE              32
#define FRENDER_KEY_APOSTROPHE         39  /* ' */
#define FRENDER_KEY_COMMA              44  /* , */
#define FRENDER_KEY_MINUS              45  /* - */
#define FRENDER_KEY_PERIOD             46  /* . */
#define FRENDER_KEY_SLASH              47  /* / */
#define FRENDER_KEY_0                  48
#define FRENDER_KEY_1                  49
#define FRENDER_KEY_2                  50
#define FRENDER_KEY_3                  51
#define FRENDER_KEY_4                  52
#define FRENDER_KEY_5                  53
#define FRENDER_KEY_6                  54
#define FRENDER_KEY_7                  55
#define FRENDER_KEY_8                  56
#define FRENDER_KEY_9                  57
#define FRENDER_KEY_SEMICOLON          59  /* ; */
#define FRENDER_KEY_EQUAL              61  /* = */
#define FRENDER_KEY_A                  65
#define FRENDER_KEY_B                  66
#define FRENDER_KEY_C                  67
#define FRENDER_KEY_D                  68
#define FRENDER_KEY_E                  69
#define FRENDER_KEY_F                  70
#define FRENDER_KEY_G                  71
#define FRENDER_KEY_H                  72
#define FRENDER_KEY_I                  73
#define FRENDER_KEY_J                  74
#define FRENDER_KEY_K                  75
#define FRENDER_KEY_L                  76
#define FRENDER_KEY_M                  77
#define FRENDER_KEY_N                  78
#define FRENDER_KEY_O                  79
#define FRENDER_KEY_P                  80
#define FRENDER_KEY_Q                  81
#define FRENDER_KEY_R                  82
#define FRENDER_KEY_S                  83
#define FRENDER_KEY_T                  84
#define FRENDER_KEY_U                  85
#define FRENDER_KEY_V                  86
#define FRENDER_KEY_W                  87
#define FRENDER_KEY_X                  88
#define FRENDER_KEY_Y                  89
#define FRENDER_KEY_Z                  90
#define FRENDER_KEY_LEFT_BRACKET       91  /* [ */
#define FRENDER_KEY_BACKSLASH          92  /* \ */
#define FRENDER_KEY_RIGHT_BRACKET      93  /* ] */
#define FRENDER_KEY_GRAVE_ACCENT       96  /* ` */
#define FRENDER_KEY_WORLD_1            161 /* non-US #1 */
#define FRENDER_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define FRENDER_KEY_ESCAPE             256
#define FRENDER_KEY_ENTER              257
#define FRENDER_KEY_TAB                258
#define FRENDER_KEY_BACKSPACE          259
#define FRENDER_KEY_INSERT             260
#define FRENDER_KEY_DELETE             261
#define FRENDER_KEY_RIGHT              262
#define FRENDER_KEY_LEFT               263
#define FRENDER_KEY_DOWN               264
#define FRENDER_KEY_UP                 265
#define FRENDER_KEY_PAGE_UP            266
#define FRENDER_KEY_PAGE_DOWN          267
#define FRENDER_KEY_HOME               268
#define FRENDER_KEY_END                269
#define FRENDER_KEY_CAPS_LOCK          280
#define FRENDER_KEY_SCROLL_LOCK        281
#define FRENDER_KEY_NUM_LOCK           282
#define FRENDER_KEY_PRINT_SCREEN       283
#define FRENDER_KEY_PAUSE              284
#define FRENDER_KEY_F1                 290
#define FRENDER_KEY_F2                 291
#define FRENDER_KEY_F3                 292
#define FRENDER_KEY_F4                 293
#define FRENDER_KEY_F5                 294
#define FRENDER_KEY_F6                 295
#define FRENDER_KEY_F7                 296
#define FRENDER_KEY_F8                 297
#define FRENDER_KEY_F9                 298
#define FRENDER_KEY_F10                299
#define FRENDER_KEY_F11                300
#define FRENDER_KEY_F12                301
#define FRENDER_KEY_F13                302
#define FRENDER_KEY_F14                303
#define FRENDER_KEY_F15                304
#define FRENDER_KEY_F16                305
#define FRENDER_KEY_F17                306
#define FRENDER_KEY_F18                307
#define FRENDER_KEY_F19                308
#define FRENDER_KEY_F20                309
#define FRENDER_KEY_F21                310
#define FRENDER_KEY_F22                311
#define FRENDER_KEY_F23                312
#define FRENDER_KEY_F24                313
#define FRENDER_KEY_F25                314
#define FRENDER_KEY_KP_0               320
#define FRENDER_KEY_KP_1               321
#define FRENDER_KEY_KP_2               322
#define FRENDER_KEY_KP_3               323
#define FRENDER_KEY_KP_4               324
#define FRENDER_KEY_KP_5               325
#define FRENDER_KEY_KP_6               326
#define FRENDER_KEY_KP_7               327
#define FRENDER_KEY_KP_8               328
#define FRENDER_KEY_KP_9               329
#define FRENDER_KEY_KP_DECIMAL         330
#define FRENDER_KEY_KP_DIVIDE          331
#define FRENDER_KEY_KP_MULTIPLY        332
#define FRENDER_KEY_KP_SUBTRACT        333
#define FRENDER_KEY_KP_ADD             334
#define FRENDER_KEY_KP_ENTER           335
#define FRENDER_KEY_KP_EQUAL           336
#define FRENDER_KEY_LEFT_SHIFT         340
#define FRENDER_KEY_LEFT_CONTROL       341
#define FRENDER_KEY_LEFT_ALT           342
#define FRENDER_KEY_LEFT_SUPER         343
#define FRENDER_KEY_RIGHT_SHIFT        344
#define FRENDER_KEY_RIGHT_CONTROL      345
#define FRENDER_KEY_RIGHT_ALT          346
#define FRENDER_KEY_RIGHT_SUPER        347
#define FRENDER_KEY_MENU               348

#define FRENDER_KEY_LAST               FRENDER_KEY_MENU

/*! @} */

/*! @defgroup mods Modifier key flags
 *  @brief Modifier key flags.
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  @ingroup input
 *  @{ */

/*! @brief If this bit is set one or more Shift keys were held down.
 *
 *  If this bit is set one or more Shift keys were held down.
 */
#define FRENDER_MOD_SHIFT           0x0001
/*! @brief If this bit is set one or more Control keys were held down.
 *
 *  If this bit is set one or more Control keys were held down.
 */
#define FRENDER_MOD_CONTROL         0x0002
/*! @brief If this bit is set one or more Alt keys were held down.
 *
 *  If this bit is set one or more Alt keys were held down.
 */
#define FRENDER_MOD_ALT             0x0004
/*! @brief If this bit is set one or more Super keys were held down.
 *
 *  If this bit is set one or more Super keys were held down.
 */
#define FRENDER_MOD_SUPER           0x0008
/*! @brief If this bit is set the Caps Lock key is enabled.
 *
 *  If this bit is set the Caps Lock key is enabled and the @ref
 *  FRENDER_LOCK_KEY_MODS input mode is set.
 */
#define FRENDER_MOD_CAPS_LOCK       0x0010
/*! @brief If this bit is set the Num Lock key is enabled.
 *
 *  If this bit is set the Num Lock key is enabled and the @ref
 *  FRENDER_LOCK_KEY_MODS input mode is set.
 */
#define FRENDER_MOD_NUM_LOCK        0x0020

/*! @} */

/*! @defgroup buttons Mouse buttons
 *  @brief Mouse button IDs.
 *
 *  See [mouse button input](@ref input_mouse_button) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define FRENDER_MOUSE_BUTTON_1         0
#define FRENDER_MOUSE_BUTTON_2         1
#define FRENDER_MOUSE_BUTTON_3         2
#define FRENDER_MOUSE_BUTTON_4         3
#define FRENDER_MOUSE_BUTTON_5         4
#define FRENDER_MOUSE_BUTTON_6         5
#define FRENDER_MOUSE_BUTTON_7         6
#define FRENDER_MOUSE_BUTTON_8         7
#define FRENDER_MOUSE_BUTTON_LAST      FRENDER_MOUSE_BUTTON_8
#define FRENDER_MOUSE_BUTTON_LEFT      FRENDER_MOUSE_BUTTON_1
#define FRENDER_MOUSE_BUTTON_RIGHT     FRENDER_MOUSE_BUTTON_2
#define FRENDER_MOUSE_BUTTON_MIDDLE    FRENDER_MOUSE_BUTTON_3
/*! @} */

/*! @defgroup joysticks Joysticks
 *  @brief Joystick IDs.
 *
 *  See [joystick input](@ref joystick) for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define FRENDER_JOYSTICK_1             0
#define FRENDER_JOYSTICK_2             1
#define FRENDER_JOYSTICK_3             2
#define FRENDER_JOYSTICK_4             3
#define FRENDER_JOYSTICK_5             4
#define FRENDER_JOYSTICK_6             5
#define FRENDER_JOYSTICK_7             6
#define FRENDER_JOYSTICK_8             7
#define FRENDER_JOYSTICK_9             8
#define FRENDER_JOYSTICK_10            9
#define FRENDER_JOYSTICK_11            10
#define FRENDER_JOYSTICK_12            11
#define FRENDER_JOYSTICK_13            12
#define FRENDER_JOYSTICK_14            13
#define FRENDER_JOYSTICK_15            14
#define FRENDER_JOYSTICK_16            15
#define FRENDER_JOYSTICK_LAST          FRENDER_JOYSTICK_16
/*! @} */

/*! @defgroup gamepad_buttons Gamepad buttons
 *  @brief Gamepad buttons.
 *
 *  See @ref gamepad for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define FRENDER_GAMEPAD_BUTTON_A               0
#define FRENDER_GAMEPAD_BUTTON_B               1
#define FRENDER_GAMEPAD_BUTTON_X               2
#define FRENDER_GAMEPAD_BUTTON_Y               3
#define FRENDER_GAMEPAD_BUTTON_LEFT_BUMPER     4
#define FRENDER_GAMEPAD_BUTTON_RIGHT_BUMPER    5
#define FRENDER_GAMEPAD_BUTTON_BACK            6
#define FRENDER_GAMEPAD_BUTTON_START           7
#define FRENDER_GAMEPAD_BUTTON_GUIDE           8
#define FRENDER_GAMEPAD_BUTTON_LEFT_THUMB      9
#define FRENDER_GAMEPAD_BUTTON_RIGHT_THUMB     10
#define FRENDER_GAMEPAD_BUTTON_DPAD_UP         11
#define FRENDER_GAMEPAD_BUTTON_DPAD_RIGHT      12
#define FRENDER_GAMEPAD_BUTTON_DPAD_DOWN       13
#define FRENDER_GAMEPAD_BUTTON_DPAD_LEFT       14
#define FRENDER_GAMEPAD_BUTTON_LAST            FRENDER_GAMEPAD_BUTTON_DPAD_LEFT

#define FRENDER_GAMEPAD_BUTTON_CROSS       FRENDER_GAMEPAD_BUTTON_A
#define FRENDER_GAMEPAD_BUTTON_CIRCLE      FRENDER_GAMEPAD_BUTTON_B
#define FRENDER_GAMEPAD_BUTTON_SQUARE      FRENDER_GAMEPAD_BUTTON_X
#define FRENDER_GAMEPAD_BUTTON_TRIANGLE    FRENDER_GAMEPAD_BUTTON_Y
/*! @} */

/*! @defgroup gamepad_axes Gamepad axes
 *  @brief Gamepad axes.
 *
 *  See @ref gamepad for how these are used.
 *
 *  @ingroup input
 *  @{ */
#define FRENDER_GAMEPAD_AXIS_LEFT_X        0
#define FRENDER_GAMEPAD_AXIS_LEFT_Y        1
#define FRENDER_GAMEPAD_AXIS_RIGHT_X       2
#define FRENDER_GAMEPAD_AXIS_RIGHT_Y       3
#define FRENDER_GAMEPAD_AXIS_LEFT_TRIGGER  4
#define FRENDER_GAMEPAD_AXIS_RIGHT_TRIGGER 5
#define FRENDER_GAMEPAD_AXIS_LAST          FRENDER_GAMEPAD_AXIS_RIGHT_TRIGGER
/*! @} */