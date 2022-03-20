#include "work.h"

#include <stdlib.h>

typedef struct work_object
{
	i32 _placeholder;
} work_object;

struct work_dispatcher
{
	work_object *objects;
	i32 objectCount;
};

work_dispatcher *
work_init_dispatcher()
{
	work_dispatcher *dispatcher = calloc(1, sizeof(work_dispatcher));

	return dispatcher;
}
