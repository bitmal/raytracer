#ifndef __CONFIG_H
#define __CONFIG_H

// config: a data structure for containment, and
// manipulation of configuration files & data

#include "stdinc.h"

namespace raytracer
{
	struct config_context;

	struct config_context *
	get_config_context();
};

#endif
