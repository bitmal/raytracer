#ifndef __USER_INTERFACE_H
#define __USER_INTERFACE_H

#include "stdinc.h"

typedef struct user_interface user_interface;

typedef struct user_interface_settings_constructor
{
	b32 isAmbientLighting;
	b32 isDirectionalLighting;
	real32 directionalLightingIntensity;
	b32 isShowFPS;
	real32 pixelDensity;
} user_interface_settings_constructor;

#define UI_SETTING_null ((u32)(0x0))
#define UI_SETTING_is_fps ((u32)(0x1))
#define UI_SETTING_is_directional_lighting ((u32)(0x2))
#define UI_SETTING_directional_lighting_intensity ((u32)(0x4))
#define UI_SETTING_is_show_fps ((u32)(0x8))
#define UI_SETTING_pixel_density ((u32)(0x10))

#define UI_SETTING_VALUE_null ((u32)(0x0))
#define UI_SETTING_VALUE_false ((u32)(0x0))
#define UI_SETTING_VALUE_true ((u32)(0x1))

extern user_interface *
user_interface_init(u32 flags, void *values);

extern void
user_interface_change_settings(user_interface *interface, u32 flags, void *values);

extern const void *
user_interface_get_setting(user_interface *interface);

#endif
