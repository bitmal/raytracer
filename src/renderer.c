#include "renderer.h"
#include "canvas.h"
#include "scene.h"

#include <stdlib.h>

struct raytracer_renderer
{
	raytracer_canvas *canvas;
	i32 *sphereIds;
	i32 sphereIdCount;
	color32 backgroundColor;
};

raytracer_renderer *
renderer_init(raytracer_canvas *canvas)
{
	raytracer_renderer *r = malloc(sizeof(raytracer_renderer));
	r->canvas = canvas;
	r->sphereIds = NULL;
	r->sphereIdCount = 0;

	return r;
}

void
renderer_push_sphere(raytracer_renderer *renderer, i32 sphereId)
{
	i32 index = renderer->sphereIdCount;

	if(renderer->sphereIdCount > 0)
	{
		renderer->sphereIds = realloc(renderer->sphereIds, 
				sizeof(i32)*(++renderer->sphereIdCount));
	}
	else
	{
		renderer->sphereIds = malloc(sizeof(i32)*(++renderer->sphereIdCount));
	}

	renderer->sphereIds[index] = sphereId;
}

void
renderer_draw(raytracer_renderer *renderer, raytracer_scene *scene)
{
	real32 pixelDensity = scene_get_pixel_density(scene);
	i32 width = canvas_get_width(renderer->canvas);
	i32 height = canvas_get_height(renderer->canvas);

	// interpolate color to background

#if 1
	for(i32 _y = 0; _y < height; ++_y)
	{
		i32 y = _y;

		for(i32 _x = 0; _x < width; ++_x)
		{
			i32 x = _x;

			v4 cameraPosition;
			scene_get_camera_position(scene, &cameraPosition);

			v4 viewportPoint;
			scene_canvas_to_world_coordinates(scene, renderer->canvas, x, y, &viewportPoint);

			color32 result;

			if(scene_trace_ray(scene, &viewportPoint, &result))
			{
				canvas_put_pixel(renderer->canvas, x, y, result);
			}
			else
			{
				canvas_put_pixel(renderer->canvas, x, y, 0x0);
			}
		}
	}
#endif
}
