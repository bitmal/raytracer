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

typedef struct scene_directional_light
{
	v4 direction;
	real32 intensity;
} scene_directional_light;

struct raytracer_scene
{
	scene_camera camera;
	scene_directional_light *directionalLights;
	i32 directionalLightCount;
	scene_sphere *spheres;
	i32 sphereCount;
	real32 ambientIntensity;
	real32 pixelDensity;
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

	scene->directionalLights = NULL;
	scene->directionalLightCount = 0;
	scene->ambientIntensity = 0.17f;
	scene->pixelDensity = 1.f;

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
scene_set_pixel_density(raytracer_scene *scene, real32 dropoff)
{
	scene->pixelDensity = dropoff;
}

real32
scene_get_pixel_density(raytracer_scene *scene)
{
	return scene->pixelDensity;
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

i32
scene_create_directional_light(raytracer_scene *scene, const v4 *direction, 
		real32 intensity)
{
	i32 index = scene->directionalLightCount;

	if(scene->directionalLightCount > 0)
	{
		scene->directionalLights = realloc(scene->directionalLights, 
				sizeof(scene_directional_light)*(++scene->directionalLightCount));
	}
	else
	{
		scene->directionalLights = malloc(sizeof(scene_directional_light)*
				(++scene->directionalLightCount));
	}

	scene_directional_light *light = &scene->directionalLights[index];
	light->direction = *direction;
	light->intensity = intensity;

	return index;
}

void
scene_set_directional_light_intensity(raytracer_scene *scene, i32 lightId, 
		real32 intensity)
{
	scene->directionalLights[lightId].intensity = intensity;
}

static i32
_scene_get_ray_sphere_intersection(raytracer_scene *scene, i32 sphereId, 
		const v4 *viewportPosition, const v4 *origin, real32 *out0, real32 *out1)
{
	scene_sphere *sphere = &scene->spheres[sphereId];

	v4 CO;
	vec4_subtract3(origin, &sphere->position, &CO);

	real32 a = vec4_dot3(viewportPosition, viewportPosition);
	real32 b = 2.f*vec4_dot3(&CO, viewportPosition);
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
scene_trace_ray(raytracer_scene *scene, const v4 *viewportPosition, color32 *outColor)
{
	i32 closestSphereId = SPHERE_NULL;
	real32 closestDistance;

	for(i32 i = 0; i < scene->sphereCount; ++i)
	{
		real32 distances[2];
		i32 intersections = _scene_get_ray_sphere_intersection(scene, i, viewportPosition, 
				&scene->camera.position, &distances[0], &distances[1]);

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
		color32 c = scene->spheres[closestSphereId].color;

		real32 intensity = scene->ambientIntensity;

		for(i32 i = 0; i < scene->directionalLightCount; ++i)
		{
			scene_directional_light *light = &scene->directionalLights[i];

			v4 direction;
			vec4_direction(&scene->camera.position, viewportPosition, &direction);

			v4 intersectPoint;
			vec4_scalar(&direction, closestDistance, &intersectPoint);
			vec4_add3(&scene->camera.position, &intersectPoint, &intersectPoint);

			v4 normal;
			vec4_direction(&scene->spheres[closestSphereId].position, &intersectPoint, &normal);

			real32 dot = vec4_dot3(&light->direction, &normal);

			if(dot < 0.f)
			{
				intensity += -dot*light->intensity;
			}
		}

		if(intensity > 1.f)
		{
			intensity = 1.f;
		}

		*outColor = ((u32)(((c >> 16) & 0xFF)*intensity) << 16) |
			((u32)(((c >> 8) & 0xFF)*intensity) << 8) |
			((u32)(((c) & 0xFF)*intensity));

		return B32_TRUE;
	}

	return B32_FALSE;
}
