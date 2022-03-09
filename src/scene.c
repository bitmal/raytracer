#include "scene.h"
#include "canvas.h"
#include "rt_math.h"

#include <stdlib.h>

typedef struct camera_viewport
{
	real32 left;
	real32 right;
	real32 top;
	real32 bottom;
	real32 front;
	real32 back;
	real32 fov;
} camera_viewport;

typedef struct scene_camera
{
	camera_viewport viewport;
	v4 position;
} scene_camera;

typedef struct scene_sphere
{
	v4 position;
	real32 radius;
	color32 color;
} scene_sphere;

struct raytracer_scene
{
	scene_camera camera;
	scene_sphere *spheres;
	i32 sphereCount;
};

raytracer_scene *
scene_init()
{
	raytracer_scene *scene = malloc(sizeof(raytracer_scene));
	scene->camera.viewport.left = -1.f;
	scene->camera.viewport.right = 1.f;
	scene->camera.viewport.top = 1.f;
	scene->camera.viewport.bottom = -1.f;
	scene->camera.viewport.front = 1.f;
	scene->camera.viewport.back = scene->camera.viewport.front + 100.f;
	scene->camera.viewport.fov = 90.f;
	scene->camera.position = vec4_init(0.f, 0.f, 0.f, 0.f);

	return scene;
}

void
scene_set_camera_viewport(raytracer_scene *scene, real32 left, real32 right, real32 top,
		real32 bottom, real32 front, real32 distance, real32 fov)
{
	scene->camera.viewport.left = left;
	scene->camera.viewport.right = right;
	scene->camera.viewport.top = top;
	scene->camera.viewport.bottom = bottom;
	scene->camera.viewport.front = front;
	scene->camera.viewport.back = scene->camera.viewport.front + distance;
	scene->camera.viewport.fov = fov;
}

void
scene_get_camera_position(raytracer_scene *scene, v4 *out)
{
	*out = scene->camera.position;
}

void
scene_canvas_to_world_coordinates(raytracer_scene *scene, raytracer_canvas *canvas, 
		i32 x, i32 y, v4 *out)
{
	i32 width = canvas_get_width(canvas);
	i32 height = canvas_get_height(canvas);

	out->x = (real32)x*((scene->camera.viewport.right - scene->camera.viewport.left)/
			(real32)width) + (scene->camera.viewport.left) + scene->camera.position.x;
	out->y = (real32)-y*((scene->camera.viewport.top - scene->camera.viewport.bottom)/
			(real32)height) - (scene->camera.viewport.bottom) + scene->camera.position.y;
	out->z = scene->camera.viewport.front + scene->camera.position.z;
}

i32
scene_world_to_canvas_x(raytracer_scene *scene, raytracer_canvas *canvas,
		const v4 *worldCoords)
{
	i32 width = canvas_get_width(canvas);

	return (i32)((worldCoords->x - scene->camera.viewport.left)*((real32)width/
				(scene->camera.viewport.right - scene->camera.viewport.left)));
}

i32
scene_world_to_canvas_y(raytracer_scene *scene, raytracer_canvas *canvas,
		const v4 *worldCoords)
{
	i32 height = canvas_get_height(canvas);

	return (i32)(-(worldCoords->y + scene->camera.viewport.bottom)*((real32)height/
				(scene->camera.viewport.top - scene->camera.viewport.bottom)));
}

i32
scene_create_sphere(raytracer_scene *scene, const v4 *position, real32 radius, color32 c)
{
	i32 index = scene->sphereCount;

	if(scene->sphereCount > 0)
	{
		scene->spheres = realloc(scene->spheres, sizeof(scene_sphere)*(++scene->sphereCount));
	}
	else
	{
		scene->spheres = malloc(sizeof(scene_sphere)*(++scene->sphereCount));
	}

	scene_sphere *sphere = &scene->spheres[index];
	sphere->position = *position;
	sphere->radius = radius;
	sphere->color = c;

	return index;
}

static i32
_scene_get_ray_sphere_intersection(raytracer_scene *scene, i32 sphereId, 
		const v4 *rayDirection, real32 rayDistance, real32 *out0, real32 *out1)
{
	scene_sphere *sphere = &scene->spheres[sphereId];

	v4 CO;
	vec4_subtract3(&scene->camera.position, &sphere->position, &CO);

	v4 D;
	vec4_scalar3(rayDirection, rayDistance, &D);

	real32 a = vec4_dot3(&D, &D);
	real32 b = 2.f*vec4_dot3(&CO, &D);
	real32 c = vec4_dot3(&CO, &CO) - sphere->radius*sphere->radius;

	real32 discriminant = b*b - 4*a*c;

	if(discriminant < 0)
	{
		return 0;
	}

	real32 d = sqrtf(discriminant);
	*out0 = (-b + d) / (2.f*a);
	*out1 = (-b - d) / (2.f*a);

	return 2;
}

b32
scene_trace_ray(raytracer_scene *scene, const v4 *direction, real32 distance, 
		color32 *outColor)
{
	i32 closestSphereId = SPHERE_NULL;
	real32 closestDistance;

	for(i32 i = 0; i < scene->sphereCount; ++i)
	{
		real32 distances[2];
		i32 intersections = _scene_get_ray_sphere_intersection(scene, i, direction, 
				distance, &distances[0], &distances[1]);

		for(i32 j = 0; j < intersections; ++j)
		{
			if(closestSphereId != SPHERE_NULL)
			{
				if(distances[j] < closestDistance)
				{
					closestSphereId = i;
					closestDistance = distances[j];
				}
			}
			else
			{
				closestSphereId = i;
				closestDistance = distances[j];
			}
		}
	}

	if(closestSphereId != SPHERE_NULL)
	{
		*outColor = scene->spheres[closestSphereId].color;

		return B32_TRUE;
	}

	return B32_FALSE;
}
