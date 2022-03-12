#include "user_interface.h"

user_interface *
user_interface_init(u32 flags, void *values)
{
	user_interface *interface = malloc(sizeof(user_interface));
}

extern void
user_interface_change_settings(user_interface *interface, u32 flags, void *values);

extern void
user_interface_get_setting(user_interface *interface, void *value);
