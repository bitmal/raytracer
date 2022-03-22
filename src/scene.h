#ifndef __SCENE_H
#define __SCENE_H

#include "stdinc.h"
#include "rt_math.h"

typedef struct raytracer_scene raytracer_scene;
struct raytracer_canvas;

typedef enum scene_light_type
{
	LIGHT_AMBIENT,
	LIGHT_DIRECTIONAL,
	LIGHT_POINT
} scene_light_t;

typedef enum scene_object_type
{
	SCENE_OBJECT_SPHERE,
	SCENE_OBJECT_BOX
} scene_object_t;

#define SCENE_OBJECT_NULL (-1)

#define LIGHT_VALUE_TYPE (1 << 0)
#define LIGHT_VALUE_POSITION (1 << 1)
#define LIGHT_VALUE_DIRECTION (1 << 2)
#define LIGHT_VALUE_COLOR (1 << 3)
#define LIGHT_VALUE_INTENSITY (1 << 4)
#define LIGHT_VALUE_RANGE (1 << 5)

#define SCENE_OBJECT_VALUE_TYPE (1 << 0)
#define SCENE_OBJECT_VALUE_POSITION (1 << 1)
#define SCENE_OBJECT_VALUE_COLOR (1 << 2)
#define SCENE_OBJECT_VALUE_ALBEDO (1 << 3)
#define SCENE_OBJECT_VALUE_SPHERE_RADIUS (1 << 4)
#define SCENE_OBJECT_VALUE_BOX_WIDTH (1 << 5)
#define SCENE_OBJECT_VALUE_BOX_HEIGHT (1 << 6)
#define SCENE_OBJECT_VALUE_BOX_DEPTH (1 << 7)

extern raytracer_scene *
scene_init();

extern void
scene_set_camera_viewport(raytracer_scene *scene, real32 left, real32 right, real32 top,
		real32 bottom, real32 front, real32 distance, real32 fov);

extern void
scene_set_camera_position(raytracer_scene *scene, const v4 *position);

extern void
scene_set_pixel_size(raytracer_scene *scene, real32 density);

extern real32
scene_get_pixel_size(raytracer_scene *scene);

extern void
scene_get_camera_position(raytracer_scene *scene, v4 *out);

extern void
scene_canvas_to_world_coordinates(raytracer_scene *scene, raytracer_canvas *canvas, 
		i32 x, i32 y, v4 *out);

extern i32
scene_world_to_canvas_x(raytracer_scene *scene, raytracer_canvas *canvas,
		const v4 *worldCoords);

extern i32
scene_world_to_canvas_y(raytracer_scene *scene, raytracer_canvas *canvas,
		const v4 *worldCoords);

extern i32
scene_create_object(raytracer_scene *scene, scene_object_t type);

extern void
scene_object_set_values(raytracer_scene *scene, i32 objectId, u32 valueFlags, 
		const void **values);

extern void
scene_object_set_value(raytracer_scene *scene, i32 objectId, u32 valueFlag, 
		const void *value);

extern void
scene_object_get_value(raytracer_scene *scene, i32 objectId, u32 valueFlag, void *outValue);

extern i32
scene_create_light(raytracer_scene *scene, scene_light_t type);

extern void
light_set_values(raytracer_scene *scene, i32 lightId, u32 valueFlags, const void **values);

extern void
light_set_value(raytracer_scene *scene, i32 lightId, u32 valueFlag, const void *value);

extern void
light_get_value(raytracer_scene *scene, i32 lightId, u32 valueFlag, void *outValue);

extern b32
scene_trace_ray(raytracer_scene *scene, const v4 *viewportPosition, color32 *outColor);

extern void
scene_save(raytracer_scene *scene, const char *name);

extern void
scene_load(raytracer_scene *scene, const char *name);

#endif
