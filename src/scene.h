#ifndef __SCENE_H
#define __SCENE_H

#include "stdinc.h"
#include "rt_math.h"

typedef struct raytracer_scene raytracer_scene;
struct raytracer_canvas;

#define SPHERE_NULL (-1)

extern raytracer_scene *
scene_init();

extern void
scene_set_camera_viewport(raytracer_scene *scene, real32 left, real32 right, real32 top,
		real32 bottom, real32 front, real32 distance, real32 fov);

extern void
scene_set_pixel_density(raytracer_scene *scene, real32 density);

extern real32
scene_get_pixel_density(raytracer_scene *scene);

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
scene_create_sphere(raytracer_scene *scene, const v4 *position, real32 radius, color32 c);

extern i32
scene_create_directional_light(raytracer_scene *scene, const v4 *direction, 
		real32 intensity);

extern void
scene_set_directional_light_intensity(raytracer_scene *scene, i32 lightId, 
		real32 intensity);

extern b32
scene_trace_ray(raytracer_scene *scene, const v4 *viewportPosition, color32 *outColor);

#endif
